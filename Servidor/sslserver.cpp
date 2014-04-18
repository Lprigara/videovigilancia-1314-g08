#include "sslserver.h"
#include "mainwindow.h"

Server::Server(QObject *parent): QTcpServer(parent)
{
    setting_ = new QSettings("Leonor", "viewer"); //configura QSetting
    setting_->setValue("viewer/key", "SSL/server.key");
    setting_->setValue("viewer/certificate", "SSL/server.crt");
    last_pixmap_ = NULL;
}

Server::~Server()
{
    if (last_pixmap_ != NULL) delete last_pixmap_;
    qDeleteAll(clientConnections_);
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    QSslSocket* new_connection = new QSslSocket();

    if(new_connection->setSocketDescriptor(socketDescriptor))
    {
        //Initialize SSL connection (key + certificate + protocol)
        key_ = setting_->value("viewer/key", "").toByteArray();
        certificate_ = setting_->value("viewer/certificate", "").toByteArray();

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

        QFile file_cert(certificate_);
        if(file_cert.open(QIODevice::ReadOnly))
        {
             certificate_ = file_cert.readAll();
             file_cert.close();
        }
        else
        {
            qDebug() <<"Error cert: "<< file_cert.errorString();
        }

        QSslKey ssl_key(key_,QSsl::Rsa);
        QSslCertificate ssl_cert(certificate_);

        new_connection->setPrivateKey(ssl_key);
        new_connection->setLocalCertificate(ssl_cert);

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

        clientConnections_.append(new_client);
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

    clientConnections_.removeAll(client);
    client->deleteLater();
    qDebug() << "Closed connection with client " << client->getName();
}

void Server::clientCompletePackage()
{
    Client* client = qobject_cast<Client *>(sender());

    if (!client)
        return;

    if (clientConnections_.indexOf(client) == 0) //only show images of the first client
    {
        if (last_pixmap_ != NULL)
        {
            delete last_pixmap_;
        }
        last_pixmap_ = new QPixmap(*client->getPixmap());
        emit showNewImage();
    }
    else
    {
        qDebug() << "Not showing data of client" << client->getName();
    }
}

QPixmap* Server::getPixmap()
{
    return last_pixmap_;
}
