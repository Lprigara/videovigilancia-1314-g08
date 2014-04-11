#include "sslserver.h"
#include "mainwindow.h"

Server::Server(QObject *parent): QTcpServer(parent)
{
    setting = new QSettings("Leonor", "viewer"); //configura QSetting
    setting->setValue("viewer/key", "SSL/server.key");
    setting->setValue("viewer/certificate", "SSL/server.crt");
    last_pixmap = NULL;
}

Server::~Server()
{
    if (last_pixmap != NULL) delete last_pixmap;
    qDeleteAll(clientConnections);
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    QSslSocket* new_connection = new QSslSocket();

    if(new_connection->setSocketDescriptor(socketDescriptor))
    {
        //Initialize SSL connection (key + certificate + protocol)
        key = setting->value("viewer/key", "").toByteArray();
        certificate = setting->value("viewer/certificate", "").toByteArray();

        QFile file_key(key);

        if(file_key.open(QIODevice::ReadOnly))
        {
            key = file_key.readAll();
            file_key.close();
            //qDebug()<<key;
        }
        else
        {
            qDebug() <<"Error key: "<< file_key.errorString();
        }

        QFile file_cert(certificate);
        if(file_cert.open(QIODevice::ReadOnly))
        {
             certificate = file_cert.readAll();
             file_cert.close();
            // qDebug()<<cert;
        }
        else
        {
            qDebug() <<"Error cert: "<< file_cert.errorString();
        }

        QSslKey ssl_key(key,QSsl::Rsa);
        QSslCertificate ssl_cert(certificate);

        new_connection->setPrivateKey(ssl_key);
        new_connection->setLocalCertificate(ssl_cert);
        new_connection->setPeerVerifyMode(QSslSocket::VerifyNone);
        new_connection->setProtocol(QSsl::SslV3);

        qDebug()<<"Starting server encryption...";
        new_connection->startServerEncryption();

        QList<QSslError> errors;
        errors.append(QSslError::SelfSignedCertificate);
        errors.append(QSslError::CertificateUntrusted);

        new_connection->ignoreSslErrors(errors);


        //Create new client with the established connection
        Client* new_client = new Client(new_connection);

        connect(new_client, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
        connect(new_client, SIGNAL(readyRead()), this, SLOT(readClientData()));
        connect(new_client, SIGNAL(receivedCompletePackage()), this, SLOT(clientCompletePackage()));

        clientConnections.append(new_client);
        qDebug() << "Opened connection with a new client";
    }
    else
    {
        delete new_connection;
    }
}

void Server::readClientData()
{
    Client* client = qobject_cast<Client *>(sender());

    if (!client)
        return;

    client->readByProtocol();
}

void Server::clientDisconnected(){
    Client* client = qobject_cast<Client *>(sender());

    if (!client)
        return;

    clientConnections.removeAll(client);
    client->deleteLater();
    qDebug() << "Closed connection with client " << client->getName();
}

void Server::clientCompletePackage()
{
    Client* client = qobject_cast<Client *>(sender());

    if (!client)
        return;

    if (clientConnections.indexOf(client) == 0) //only show images of the first client
    {
        if (last_pixmap != NULL)
        {
            delete last_pixmap;
        }
        last_pixmap = new QPixmap(*client->getPixmap());
        emit showNewImage();
    }
    else
    {
        qDebug() << "Not showing data of client" << client->getName();
    }
}

QPixmap* Server::getPixmap()
{
    return last_pixmap;
}
