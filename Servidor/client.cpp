#include "client.h"

Client::Client(QString name, QSslSocket* sslSocket)
{
    this->name = name;
    this->sslSocket = sslSocket;
    protocol_state = 0;

    connect(sslSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(sslSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

Client::~Client()
{
    delete sslSocket;
}

QSslSocket* Client::getSocket()
{
    return sslSocket;
}

QString Client::getName()
{
    return name;
}

void Client::readByProtocol()
{
    QByteArray clientName;
    QByteArray timestamp;
    QByteArray sizeIm;
    QByteArray aux;

    switch (protocol_state)
    {
    case 0: //Client name
        if (sslSocket->canReadLine())
        {
            aux.clear();
            clientName.clear();
            //guardamos la linea en un objeto QByteArray, que luego recorreremos para imprimir su contenido sin el caracter \n
            aux= sslSocket->readLine(); //lee hasta que encuentre un caracter \n
            int i=0;
            while(aux[i] != '\n')
            {
                clientName[i]=aux[i];
                i++;
            }
            qDebug()<<"Nombre Cliente: "<<clientName;
            protocol_state = 1;
        }

        break;
    case 1: //Timestamp
        if (sslSocket->canReadLine())
        {
            aux.clear();
            timestamp.clear();
            //guardamos la linea en un objeto QByteArray, que luego recorreremos para imprimir su contenido sin el caracter \n
            aux= sslSocket->readLine();
            int i=0;
            while(aux[i] != '\n')
            {
                timestamp[i]=aux[i];
                i++;
            }
            qDebug()<<"Fecha en ms: "<< timestamp;
            protocol_state = 2;
        }
        break;
    case 2: //Image size
        if (sslSocket->canReadLine())
        {
            sizeIm.clear();
            aux.clear();
            //guardamos la linea en un objeto QByteArray, que luego recorreremos para imprimir su contenido sin el caracter \n
            sizeIm=sslSocket->readLine(); //size=6
            int i=0;
            while(sizeIm[i] != '\n')
            {
                aux[i]=sizeIm[i];
                i++;
            }

            next_image_size = aux.toInt(); //convertimos el QByteArray del tamaño de la imagen en un objeto tipo int para indicarle al read siguiente cuantos caracteres hay que leer.
            protocol_state = 3;
        }
        break;
    case 3: //image
        if(sslSocket->bytesAvailable() >= next_image_size)
        {
            QBuffer imgBuffer;
            imgBuffer.setData(sslSocket->read(next_image_size)); //lee el numero de caracteres que ocupa la imagen y el contenido lo metemos en un buffer
            QImage image;
            image.load(&imgBuffer, "jpeg");

            QString stringName = "Client Name: " + clientName;
            QString stringTimestamp = "Date in ms: "+ timestamp;

            //dibujamos el nombre del cliente y timestamp en el pixmap
            QPixmap pixmap(QPixmap::fromImage(image));
            QPainter painter(&pixmap);
            painter.setPen(Qt::white);
            painter.setFont(QFont("Times", 15));
            painter.drawText(0, 0,pixmap.width(), pixmap.height(), Qt::AlignTop | Qt::AlignLeft, stringName,0);
            painter.drawText(0, 0,pixmap.width(), pixmap.height(), Qt::AlignTop | Qt::AlignRight, stringTimestamp,0);
            //ui_->label->setPixmap(pixmap);
            protocol_state = 0;
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
