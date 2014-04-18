#include "client.h"

Client::Client(QSslSocket* sslSocket)
{
    this->sslSocket_ = sslSocket;
    protocol_state_ = 0;
    last_pixmap_ = NULL;

    connect(sslSocket_, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(sslSocket_, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(sslSocket_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionFailure()));
}

Client::~Client()
{
    if (last_pixmap_ != NULL)
    {
        delete last_pixmap_;
    }
    delete sslSocket_;
}

QSslSocket* Client::getSocket()
{
    return sslSocket_;
}

QString Client::getName()
{
    return name_;
}
QPixmap* Client::getPixmap()
{
    return last_pixmap_;
}

void Client::readByProtocol()
{
    QByteArray clientName;
    QByteArray timestamp;
    QByteArray sizeIm;
    QByteArray aux;
    QByteArray protocolName;

    switch (protocol_state_)
    {
    case 0: //Version and name of protocol
        if(sslSocket_->canReadLine())
        {
            aux.clear();
            protocolName.clear();
            aux = sslSocket_->readLine(); //lee hasta que encuentre un caracter \n
            int i=0;
            while(aux[i] != '\n')
            {
                protocolName[i]=aux[i];
                i++;
            }
            qFromLittleEndian(protocolName);
            if(protocolName != "GLP/1.0")
            {
                qDebug()<<"Intrusion. Invalid Protocol";
                onDisconnected();

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
            clientName.clear();
            //guardamos la linea en un objeto QByteArray, que luego recorreremos para imprimir su contenido sin el caracter \n
            aux = sslSocket_->readLine(); //lee hasta que encuentre un caracter \n
            int i=0;
            while(aux[i] != '\n')
            {
                clientName[i]=aux[i];
                i++;
            }
            qFromLittleEndian(clientName);
            name_ = QString(clientName);
            protocol_state_ = 2;
        }
        break;
     case 2: //Timestamp
        if (sslSocket_->canReadLine())
        {
            aux.clear();
            timestamp.clear();
            aux = sslSocket_->readLine();
            int i=0;
            while(aux[i] != '\n')
            {
                timestamp[i]=aux[i];
                i++;
            }
            qFromLittleEndian(timestamp);
            last_timestamp_ = QString(timestamp);
            protocol_state_ = 3;
        }
        break;
     case 3: //Image size
        if (sslSocket_->canReadLine())
        {
            sizeIm.clear();
            aux.clear();
            aux = sslSocket_->readLine(); //size=6
            int i=0;
            while(aux[i] != '\n')
            {
                sizeIm[i]=aux[i];
                i++;
            }
            qFromLittleEndian(sizeIm);
            next_image_size_ = sizeIm.toInt(); //convertimos el QByteArray del tamaño de la imagen en un objeto tipo int para indicarle al read siguiente cuantos caracteres hay que leer.
            protocol_state_ = 4;
        }
        break;
     case 4: //image
        if(sslSocket_->bytesAvailable() >= next_image_size_)
        {
            QBuffer imgBuffer;
            imgBuffer.setData(sslSocket_->read(next_image_size_)); //lee el numero de caracteres que ocupa la imagen y el contenido lo metemos en un buffer
            qFromLittleEndian(&imgBuffer);

            QImage image;
            image.load(&imgBuffer, "jpeg");

            if (last_pixmap_ != NULL)
            {
                delete last_pixmap_;
            }
            //dibujamos el nombre del cliente y timestamp en el pixmap
            last_pixmap_ = new QPixmap(QPixmap::fromImage(image));
            QPainter painter(last_pixmap_);
            painter.setPen(Qt::white);
            painter.setFont(QFont("Times", 15));
            painter.drawText(0, 0,last_pixmap_->width(), last_pixmap_->height(), Qt::AlignTop | Qt::AlignLeft, name_, 0);
            painter.drawText(0, 0,last_pixmap_->width(), last_pixmap_->height(), Qt::AlignBottom | Qt::AlignRight, last_timestamp_, 0);
            painter.drawText(0, 0,last_pixmap_->width(), last_pixmap_->height(), Qt::AlignTop | Qt::AlignRight, protocolName_, 0);

            protocol_state_ = 0;
            emit receivedCompletePackage();
        }
        break;
    }
}

void Client::onDisconnected()
{
    emit disconnected();
}

void Client::onReadyRead()
{
    emit readyRead();
}

void Client::connectionFailure()
{
    qDebug() << "Failure of connection with client" << name_ << ": " << sslSocket_->errorString();
}
