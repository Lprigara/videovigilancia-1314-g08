#include "sslserver.h"
#include "mainwindow.h"
#include <QSslCertificate>
#include <QSslKey>
#include <QFile>
#include <QSslConfiguration>

Server::Server(QObject *parent): QTcpServer(parent)
{
    socket = new QSslSocket;
}

Server::~Server()
{
    delete socket;
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    connect(socket, SIGNAL(encrypted()), this, SLOT(stepToMain()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionFailure()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnect()));

    if(socket->setSocketDescriptor(socketDescriptor))
    {
        addPendingConnection(socket);
            QByteArray key;
            QByteArray cert;

            QFile file_key("SSL/server.key");

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

            QFile file_cert("SSL/server.crt");
            if(file_cert.open(QIODevice::ReadOnly))
            {
                 cert = file_cert.readAll();
                 file_cert.close();
                // qDebug()<<cert;
            }
            else
            {
                qDebug() <<"Error cert: "<< file_cert.errorString();
            }

            QSslKey ssl_key(key,QSsl::Rsa);
            QSslCertificate ssl_cert(cert);
            qDebug()<<ssl_key ;
            qDebug()<<ssl_cert;
            socket->setPrivateKey(ssl_key);
            socket->setLocalCertificate(ssl_cert);
            socket->setPeerVerifyMode(QSslSocket::VerifyNone);
            socket->setProtocol(QSsl::SslV3);

            qDebug()<<"Starting server encryption...";
            socket->startServerEncryption();

            QList<QSslError> errors;
            errors.append(QSslError::SelfSignedCertificate);
            errors.append(QSslError::CertificateUntrusted);

            socket->ignoreSslErrors(errors);

    }
    else
    {
        delete socket;
    }
}

void Server::stepToMain(){
    qDebug()<<"hola";
    emit signal();
}
void Server::connectionFailure(){
    qDebug() << "Fallo en la conexion" << socket->errorString();
    socket->disconnect();
    socket->deleteLater();
    socket->ignoreSslErrors();
}

void Server::disconnect(){
    socket->disconnect();
    socket->deleteLater();
    qDebug() << "Conexion cerrada";
}
