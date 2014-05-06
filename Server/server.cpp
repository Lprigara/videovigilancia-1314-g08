#include "server.h"
#include "console.h"
#include "client.h"


Server::Server(QObject *parent, QByteArray key, QByteArray cert, QString outputDestination): QTcpServer(parent)
{    
    last_image_ = NULL;
    key_=key;
    certificate_=cert;
    frameCounter = 0;
    outputDestination_ = outputDestination;
    if (!outputDestination_.endsWith("/")) outputDestination_.append("/");
}

Server::~Server()
{
    if (last_image_ != NULL) delete last_image_;
    qDeleteAll(clientConnections_);
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    QSslSocket* new_connection = new QSslSocket();

    if(new_connection->setSocketDescriptor(socketDescriptor))
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
        QObject::connect(new_client, SIGNAL(receivedCompletePackage()), this, SLOT(clientCompletePackage()));

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
        if (last_image_ != NULL)
        {
            delete last_image_;
        }
        last_image_ = new QImage(*client->getImage());

        //Save client image in folder (%outputdestination%/CLIENTNAME/YYYY-MM-DD/CLIENTNAME_DATE_XXXX.png)


        QString clientName = client->getName();
        if (clientName == "") clientName = "UNNAMED";
        QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");

        QDir dir(outputDestination_ + clientName + "/" + date);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        QString filename = clientName + "_" + date + "_" + QString("%1").arg(frameCounter, 4, 16, QChar('0')).toUpper() + ".png";
        QString fullpath = dir.path() + "/" + filename;

        if (last_image_->save(fullpath))
        {
            qDebug() << "Saved image to" << fullpath;
        }
        else
            qDebug() << "Error saving image";

        frameCounter++;
    }
    else
    {
        qDebug() << "Not showing data of client" << client->getName();
    }
}

QImage* Server::getImage()
{
    return last_image_;
}
