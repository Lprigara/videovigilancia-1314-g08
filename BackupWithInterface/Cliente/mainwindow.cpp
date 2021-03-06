#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "acerca.h"
#include "preferencias.h"
#include "conexion.h"

bool Paused = true;  //variable para parar/pausar

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
    movie_=NULL;
    camera_=NULL;
    viewfinder_=NULL;
    captureB_=NULL;
    sslSocket_=NULL;

    //Motion detection
    mThread_ = new QThread();
    mDetect_ = new MotionDetector();
    mDetect_->moveToThread(mThread_);
    connect(this, SIGNAL(resetMotionDetection()), mDetect_, SLOT(reset()));
    connect(this, SIGNAL(readyForMotionDetection(QImage)), mDetect_, SLOT(detectMotion(QImage)));
    connect(mDetect_, SIGNAL(signalResult(std::vector<QRect >, QImage)), this, SLOT(motionDetected(std::vector<QRect >, QImage)));
    mThread_->start();


    setting_ = new QSettings("Leonor", "viewer"); //configura QSetting
    ui_->autoinicio->setChecked(setting_->value("viewer/autoinicio",true).toBool()); //setChecked necesita un bool como arg.

    devices_=QCamera::availableDevices();
    dispdefault_ = setting_->value("viewer/deviceDefault",devices_[0]).toByteArray();
    dispchoise_ = setting_->value("viewer/deviceChoise",dispdefault_).toByteArray();

    //QtNetwork
    connectedServer_=0;

    //bool para no reconectar al desconectarse
    exit_=false;


}

MainWindow::~MainWindow()
{
    mThread_->quit();
    mThread_->wait();
    delete mDetect_;
    delete mThread_;



    delete ui_;
    delete movie_;
    if (camera_ != NULL) delete camera_;
    delete viewfinder_;
    delete setting_;
    if (captureB_ != NULL) delete captureB_;
    if (sslSocket_ != NULL) delete sslSocket_;

}


//Funcion para abrir archivo de video
void MainWindow::on_actionAbrir_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Abrir archivo", QString() ,"Video (*.mjpeg)"); //te devuelve el nombre del archivo
    if(!fileName.isEmpty()) {
        QPixmap foto(fileName);
        if(foto.isNull()) {
            QMessageBox::information(this, "Abrir archivo", "El archivo no pudo ser abierto. ");
        }
        else {
            movie_ = new QMovie(fileName);
            ui_->label->setMovie(movie_);
            if(ui_->autoinicio->isChecked())
                movie_->start();

        }//endelse
    }//endif
}

void MainWindow::on_actionCapturar_triggered()
{ 
    //Set name client from ui->lineEdit
    //setting_->setValue("viewer/client", ui_->clientName->text());
    clientName_ = ui_->clientName->text();

    //(Re)create camera
    if (camera_ != NULL)
    {
        camera_->stop();
        delete camera_;
    }

     if(operator!= (dispdefault_,dispchoise_)){  
       // camera_->stop();
       // delete camera_;
        camera_ = new QCamera(dispchoise_);
     }
     else{
        camera_ = new QCamera(dispdefault_);
     }

     //Reset motion detector
     emit resetMotionDetection();

     //(Re)create capturebuffer
     if (captureB_ != NULL) delete captureB_;
     captureB_ = new captureBuffer;

     camera_->setViewfinder(captureB_);
     camera_->setCaptureMode(QCamera::CaptureViewfinder);

     connect(captureB_, SIGNAL(signalImage(QImage)), this, SLOT(image1(QImage)));

     //Connect to the server

     host_ = setting_->value("viewer/host", "127.0.0.1").toString();
     port_ = setting_->value("viewer/port", 9600).toInt();

     qDebug() << host_ << port_;

     if (sslSocket_ != NULL)
     {
         delete sslSocket_;
         qDebug() << "Restarting socket..";
     }

     sslSocket_ = new QSslSocket(this);
     connect(sslSocket_, SIGNAL(disconnected()), this, SLOT(disconnect()));
     connect(sslSocket_, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(socketError()));
     sslSocket_->ignoreSslErrors();

     connect(sslSocket_, SIGNAL(encrypted()), this, SLOT(connected()));
     sslSocket_->connectToHostEncrypted(host_, port_);
     sslSocket_->ignoreSslErrors();
}

void MainWindow::image1(QImage image)
{
    //Motion detection
    emit readyForMotionDetection(image.copy()); //sends image to motiondetection thread for processing and waits for result in other slot
}

void MainWindow::motionDetected(std::vector<QRect > boundingRects, QImage image)
{
    //Get current time as string
    QTime time;
    QTime currenTime= time.currentTime();
    QString stringTime=currenTime.toString();

    //Convert image to pixmap and use painter to draw on it
    QPixmap pixmap(QPixmap::fromImage(image));
    QPainter painter(&pixmap);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 15));
    painter.drawText(0, 0,pixmap.width(), pixmap.height(), Qt::AlignBottom, stringTime,0);

    //Update pixmap in label
    ui_->label->setPixmap(pixmap);

    if(connectedServer_)
    {

        int bbCount = boundingRects.size();
        if (bbCount <= 0) return; //Only send images with changes

        //Codificar la imagen para enviarla por la red
        QBuffer buffer;
        QImageWriter writer(&buffer, "jpeg"); //Para controlar el nivel de compresión, el de gamma o algunos otros parámetros específicos del formato, tendremos que emplear un objeto QImageWriter.

        QImage imageSend; //creación de la imagen a enviar
        imageSend=pixmap.toImage(); //conversión del pixmap (con la hora pintada) en un QImage
        writer.setCompression(70);//codifica la imagen
        writer.write(imageSend); //aplicar lo anterior a la imagen

        QByteArray bytes = buffer.buffer();
        int sizeImg=bytes.size(); //tamaño de la imagen

        //clientName_ = setting_->value("viewer/client").toString();
        QByteArray name;
        name.append(clientName_); //nombre del cliente convertido a String para poder enviarlo

        qint64 timestamp = QDateTime::currentMSecsSinceEpoch(); //tiempo en milisegundos desde EPOC hasta el instante de la imagen

        QByteArray protocol;
        protocol.append("GLP/1.0");

        //Envio de los distintos campos separados por \n
        sslSocket_->write(protocol);
        sslSocket_->write("\n");

        sslSocket_->write(name);
        sslSocket_->write("\n");

        sslSocket_->write(QByteArray::number(timestamp));
        sslSocket_->write("\n");

        sslSocket_->write(QByteArray::number(sizeImg));
        sslSocket_->write("\n");

        sslSocket_->write(bytes);

        sslSocket_->write(QByteArray::number(bbCount));
        sslSocket_->write("\n");

        for (std::vector<QRect >::iterator it = boundingRects.begin() ; it != boundingRects.end(); ++it)
        {
            QRect r = *it;
            QByteArray rBA;
            rBA.append(QByteArray::number(r.x()));
            rBA.append("_");
            rBA.append(QByteArray::number(r.y()));
            rBA.append("_");
            rBA.append(QByteArray::number(r.width()));
            rBA.append("_");
            rBA.append(QByteArray::number(r.height()));

            sslSocket_->write(rBA);
            sslSocket_->write("\n");
        }
    }
}

void MainWindow::connected()
{
    connectedServer_ = 1;
    qDebug() << "Connected to server";
    camera_->start();
}

void MainWindow::disconnect()
{
    connectedServer_ = 0;
    qDebug() << "Disconnected from server";
    if(!exit_){
        reconnect();
    }
}

void MainWindow::socketError()
{
  qDebug() << "Socket error: " << sslSocket_->errorString();
  sslSocket_->close();
}

void MainWindow::on_start_clicked()
{
    if(!Paused){
        movie_->setPaused(1);
        Paused = true;
    }
    else{
        movie_->start();
        Paused = false;
    }
}

void MainWindow::on_stop_clicked()
{
    movie_->stop();
    Paused = true;
}

void MainWindow::on_exit_clicked()
{
    exit_=true;
    qApp->quit(); //qApp = QApplication del main
}

void MainWindow::on_actionSalir_triggered()
{
    exit_=true;
    qApp->quit();
}

void MainWindow::on_autoinicio_stateChanged(int)
{
    setting_->setValue("viewer/autoinicio", ui_->autoinicio->isChecked());
}

void MainWindow::on_actionAcerca_de_triggered()
{
    Acerca acercaDe(this);
    acercaDe.exec();
}

void MainWindow::on_actionPreferencias_triggered()
{
   Preferencias prefe(this);
   prefe.exec();
   dispchoise_ = setting_->value("viewer/deviceChoise",dispdefault_).toByteArray();
}

void MainWindow::on_actionConexion_triggered()
{
    Conexion conexion(this);
    conexion.exec();
}

void MainWindow::reconnect()
{
    QTimer *timer = new QTimer(this);
    // QTimer::singleShot(7000, this, SLOT(reconnect()));
    timer->start(7000);

    if(sslSocket_->state() == QAbstractSocket::UnconnectedState)
    {
        sslSocket_->connectToHostEncrypted(host_, port_);
        sslSocket_->ignoreSslErrors();
        sslSocket_->waitForEncrypted(8000);

        qDebug()<<"Reconecting client: "<< clientName_ ;
    }
    if(sslSocket_->state() == QAbstractSocket::ConnectedState)
    {
        timer->stop();
        qDebug()<<"Reconnected client succesful";
    }
    connect(sslSocket_, SIGNAL(encrypted()), this, SLOT(connected()));
    connect(timer,SIGNAL(timeout()),this,SLOT(reconnect()));
}

