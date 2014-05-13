#include "clientthread.h"
#include <QMessageBox>
#include <QtSql/QSqlDatabase>
#include <QSqlRecord>

ClientThread::ClientThread(qintptr ID, QByteArray key, QByteArray cert, QString outputDestination, QObject *parent) :
    QThread(parent)
{

    ID_ = ID;
    outputDestination_ = outputDestination;
    key_ = key;
    cert_ = cert;
    frameCounter_ = 0;
    last_image_ = NULL;

#ifdef BENCHMARK
    timer_running_ = false;
    timer_ = new QTime();
    benchmarkCounter_ = 0;
#endif
}

ClientThread::~ClientThread()
{
#ifdef BENCHMARK
    delete timer_;
#endif
}

void ClientThread::run()
{

    //Create SSL socket with socketdescriptor ID
    sslSocket_ = new QSslSocket();

    if(sslSocket_->setSocketDescriptor(ID_))
    {
        QFile file_key(key_);

        if(file_key.open(QIODevice::ReadOnly))
        {
            key_ = file_key.readAll();
            file_key.close();
        }
        else
        {
            qDebug() <<"Error key: "<< file_key.errorString();
        }

        QFile file_cert(cert_);
        if(file_cert.open(QIODevice::ReadOnly))
        {
             cert_ = file_cert.readAll();
             file_cert.close();
        }
        else
        {
            qDebug() <<"Error cert: "<< file_cert.errorString();
        }

        QSslKey ssl_key(key_,QSsl::Rsa);
        QSslCertificate ssl_cert(cert_);

        sslSocket_->setPrivateKey(ssl_key);
        sslSocket_->setLocalCertificate(ssl_cert);

        qDebug()<<"Starting server encryption...";
        sslSocket_->startServerEncryption();

        QList<QSslError> errors;
        errors.append(QSslError::SelfSignedCertificate);
        errors.append(QSslError::CertificateUntrusted);

        sslSocket_->ignoreSslErrors(errors);

        qDebug() << "Opened connection with a new client (ID = " << ID_;
    }


    //Init protocol state
    protocol_state_ = 0;

    //Connect signals
    connect(sslSocket_, SIGNAL(readyRead()), this, SLOT(readByProtocol()));
    connect(sslSocket_, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(sslSocket_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionFailure()));

    //Make this thread a loop waiting for exit()
    exec();
}

void ClientThread::readByProtocol()
{
#ifdef BENCHMARK
    if (!timer_running_)
    {
        timer_->start();
        timer_running_ = true;
    }
#endif

    QByteArray aux;

    switch (protocol_state_)
    {
    case 0: //Version and name of protocol
        if(sslSocket_->canReadLine())
        {
            aux.clear();
            QByteArray protocolName;
            protocolName.clear();
            aux = sslSocket_->readLine(); //lee hasta que encuentre un caracter \n
            int i=0;
            while(aux[i] != '\n')
            {
                protocolName[i]=aux[i];
                i++;
            }
            if(protocolName != "GLP/1.0")
            {
                qDebug()<<"Intrusion. Invalid Protocol";
                disconnect();

                return;
            }
            else{
                protocolName_ = QString(protocolName);
                protocol_state_ = 1;
            }
        }
        break;

    case 1: //Client name
        if (sslSocket_->canReadLine())
        {
            aux.clear();
            QByteArray clientName;
            clientName.clear();
            //guardamos la linea en un objeto QByteArray, que luego recorreremos para imprimir su contenido sin el caracter \n
            aux = sslSocket_->readLine(); //lee hasta que encuentre un caracter \n
            int i=0;
            while(aux[i] != '\n')
            {
                clientName[i]=aux[i];
                i++;
            }
            name_ = QString(clientName);
            protocol_state_ = 2;
        }
        break;

     case 2: //Timestamp
        if (sslSocket_->canReadLine())
        {
            aux.clear();
            QByteArray timestamp;
            timestamp.clear();
            aux = sslSocket_->readLine();
            int i=0;
            while(aux[i] != '\n')
            {
                timestamp[i]=aux[i];
                i++;
            }
            last_timestamp_ = QString(timestamp);
            protocol_state_ = 3;
        }
        break;

     case 3: //Image size
        if (sslSocket_->canReadLine())
        {
            aux.clear();
            QByteArray sizeIm;
            sizeIm.clear();
            aux = sslSocket_->readLine(); //size=6
            int i=0;
            while(aux[i] != '\n')
            {
                sizeIm[i]=aux[i];
                i++;
            }
            next_image_size_ = sizeIm.toInt(); //convertimos el QByteArray del tamaño de la imagen en un objeto tipo int para indicarle al read siguiente cuantos caracteres hay que leer.
            protocol_state_ = 4;
        }
        break;

    case 4: //image
       if(sslSocket_->bytesAvailable() >= next_image_size_)
       {
           QBuffer imgBuffer;
           imgBuffer.setData(sslSocket_->read(next_image_size_)); //lee el numero de caracteres que ocupa la imagen y el contenido lo metemos en un buffer

           QImage image;
           image.load(&imgBuffer, "jpeg");

           if (last_image_ != NULL)
           {
               delete last_image_;
           }

           last_image_=new QImage(image);

           protocol_state_ = 5;
       }
       break;

   case 5: //ROI bounding rect count
       if (sslSocket_->canReadLine())
       {
            QByteArray bbCount;
            bbCount.clear();
            aux.clear();

            aux = sslSocket_->readLine();
            int i=0;
            while(aux[i] != '\n')
            {
               bbCount[i]=aux[i];
               i++;
            }

            next_bb_count_ = bbCount.toInt();
            bb_counter_ = 0;
            protocol_state_ = 6;

            if (next_bb_count_ == 0)
            {
                //Faulty package? 0 ROI but image was sent
                protocol_state_ = 0;
                qDebug() << "FAULTY PACKAGE" << bbCount;
            }
       }
       break;

   case 6: //ROI bounding rects
       if (sslSocket_->canReadLine())
       {
           //Get data
           QByteArray data;
           aux.clear();
           aux = sslSocket_->readLine();
           int i=0;
           while(aux[i] != '\n')
           {
               data[i]=aux[i];
               i++;
           }

           //reset boundingbox vector
           if (bb_counter_ == 0) {
               last_boundingboxes_.clear();
           }

           QStringList rectData = QString(data).split("_");
           if (rectData.size() != 4)
           {
               qDebug() << "RECTDATA ERROR:" << QString(data);
               qDebug() << "BB_COUNTER_" << bb_counter_ << "NEXT_BB_COUNT_" << next_bb_count_ << "VECTSIZE"<<last_boundingboxes_.size();
               protocol_state_ = 0; return;
           }
           QRect rect(rectData[0].toInt(), rectData[1].toInt(), rectData[2].toInt(), rectData[3].toInt());
           last_boundingboxes_.push_back(rect);

           //last bounding box received
           if(bb_counter_ == next_bb_count_ - 1)
           {
               protocol_state_ = 0;

               //Received all data..

#ifdef BENCHMARK
               int runtime = timer_->elapsed();
               qDebug() << "Received whole package in" << runtime << "ms" << "with" << next_bb_count_ << "ROI. (" << benchmarkCounter_+1<<"/ 50 )";
               timer_running_ = false;
               ms_list_.push_back(runtime);
               roi_list_.push_back(last_boundingboxes_.size());
               benchmarkCounter_++;
               if (benchmarkCounter_ == 49)
               {
                   int ms_max, ms_min;
                   float ms_avg = 0;
                   int roi_max, roi_min;
                   float roi_avg = 0;

                   ms_max = ms_list_[0]; roi_max = roi_list_[0];
                   ms_min = ms_list_[0]; roi_min = roi_list_[0];
                   //ms_median = ms_list_[25]; roi_median = roi_list_[25]; //needs to be a sorted vector

                   for (int i = 0; i < ms_list_.size(); i++)
                   {
                       if (ms_max < ms_list_[i]) ms_max = ms_list_[i];
                       if (roi_max < roi_list_[i]) roi_max = roi_list_[i];
                       if (ms_min > ms_list_[i]) ms_min = ms_list_[i];
                       if (roi_min > roi_list_[i]) roi_min = roi_list_[i];
                       ms_avg += ms_list_[i];
                       roi_avg += roi_list_[i];
                   }
                   ms_avg /= ms_list_.size();
                   roi_avg /= ms_list_.size();

                   qDebug() << "Benchmark result for 50 frames";
                   qDebug() << "MS: min"<<ms_min<<"max"<<ms_max<<"avg"<<ms_avg;
                   qDebug() << "ROI: min"<<roi_min<<"max"<<roi_max<<"avg"<<roi_avg;

                   benchmarkCounter_ = 0;
                   ms_list_.clear();
                   roi_list_.clear();
               }
#endif

               //Save client image in folder (%outputdestination%/CLIENTNAME/YYYY-MM-DD/CLIENTNAME_DATE_XXXX.png)

               QString clientName = name_;
               if (clientName == "") clientName = "UNNAMED";
               QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");

               QDir dir(outputDestination_ + clientName + "/" + date);
               if (!dir.exists()) {
                   dir.mkpath(".");
               }

               QString filename = clientName + "_" + date + "_" + QString("%1").arg(frameCounter_, 4, 16, QChar('0')).toUpper() + ".png";
               fullpath_ = dir.path() + "/" + filename;

               if (last_image_->save(fullpath_))
               {
                   qDebug() << "Saved image to" << fullpath_;
               }
               else
                   qDebug() << "Error saving image";

               frameCounter_++;


               ///////////////////////////////////////////////////////////////////////////////////

               QSqlQuery query;
               QSqlQuery("PRAGMA journal_mode = OFF");
               QSqlQuery("PRAGMA locking_mode = EXCLUSIVE");
               QSqlQuery("PRAGMA synchronous = OFF");

               query.prepare("INSERT INTO Datos (client, timestamp, image) "
                             "VALUES (:client, :timestamp, :image)");

               qDebug()<<"guardando nameclient";
               query.bindValue(":client", name_);
               qDebug()<<"guardando timestamp";
               query.bindValue(":timestamp", last_timestamp_);
               qDebug()<<"guardando imageroute";
               query.bindValue(":image", fullpath_);

               query.exec();


               QSqlQuery q("select id from Datos");

               QSqlRecord rec = q.record();

               int nameCol = rec.indexOf("id");

               q.last(); //Recupera la última fila de la lista de resultados y posiciona allí el objeto q.

               QString ultimoId = q.value(nameCol).toString();

               foreach(QRect rect, last_boundingboxes_)
               {

                   QSqlQuery query2;
                   query2.prepare("INSERT INTO ROI (x, y, h, w, link) "
                                  "VALUES (:x, :y, :h, :w, :link)");

                   qDebug()<<"Guardando link";
                   query2.bindValue(":link", ultimoId.toInt());
                   qDebug()<<"Guardando x";
                   query2.bindValue(":x", rect.x());
                   qDebug()<<"Guardando y";
                   query2.bindValue(":y", rect.y());
                   qDebug()<<"Guardando heigth";
                   query2.bindValue(":h", rect.height());
                   qDebug()<<"Guardando width";
                   query2.bindValue(":w", rect.width());

                   query2.exec();

               }
           }

           bb_counter_++;
       }
       break;
   }


}

void ClientThread::disconnected()
{
    qDebug() << ID_ << " Disconnected";
    sslSocket_->deleteLater();
    exit(0);
}

void ClientThread::connectionFailure()
{
    qDebug() << "Failure of connection with client" << name_ << "(ID =)"<< ID_<<": " << sslSocket_->errorString();
}

