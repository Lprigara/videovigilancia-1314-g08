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
    Server(QObject * parent, QByteArray key, QByteArray cert, QString outputDestination);
    void incomingConnection(qintptr socketDescriptor);
    ~Server();
    QImage* getImage();

signals:


public slots:

    void readClientData();

    void clientDisconnected();

    void clientCompletePackage();

private:
    QString outputDestination_;
    int frameCounter;
    QByteArray key_;
    QByteArray certificate_;
    QSettings* setting_;
    QImage* last_image_;
    QList<Client*> clientConnections_;
};


#endif // SSLSERVER_H
