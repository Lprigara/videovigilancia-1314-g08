#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>
#include <QFile>
#include <QSettings>
#include <QDir>

#include "client.h"


class Server: public QTcpServer
{
    Q_OBJECT

public:
    Server(QObject * parent, QByteArray key, QByteArray cert);
    void incomingConnection(qintptr socketDescriptor);
    ~Server();
    QImage* getImage();

signals:


public slots:

    void readClientData();

    void clientDisconnected();

    void clientCompletePackage();

private:
    int frameCounter;
    int frameSkipCounter;
    QByteArray key_;
    QByteArray certificate_;
    QSettings* setting_;
    QImage* last_image_;
    QList<Client*> clientConnections_;
};


#endif // SSLSERVER_H
