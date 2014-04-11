#include "sslserver.h"
#include "mainwindow.h"

Server::Server(QObject *parent): QTcpServer(parent)
{
    socket = new QSslSocket;//mas q uno..
    setting = new QSettings("Leonor", "viewer"); //configura QSetting
    setting->setValue("viewer/key", "SSL/server.key");
    setting->setValue("viewer/certificate", "SSL/server.crt");


    //pruebas, varios cliente a la vez, desconectar y reconectar
    //prueba cifrado: conectar con netcat, ver q intenta conectarse con handshake
    //prueba protocolo: ..nada, wiki
    //prueba almacenamiento: ver archivos
}

Server::~Server()
{
    //delete socket;
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionFailure()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnect()));

    if(socket->setSocketDescriptor(socketDescriptor))
    {
        addPendingConnection(socket);
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

void Server::connectionFailure(){
    qDebug() << "Fallo en la conexion" << socket->errorString();
    socket->disconnect();
    //socket->deleteLater();
    socket->ignoreSslErrors();
}

void Server::disconnect(){
    socket->disconnect();
    //socket->deleteLater();
    qDebug() << "Conexion cerrada";
}
