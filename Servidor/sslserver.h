#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>
#include <QFile>
#include <QSettings>

#include "client.h"


class Server: public QTcpServer
{
    Q_OBJECT

public:
    Server(QObject * parent = 0);
    void incomingConnection(qintptr socketDescriptor);
    ~Server();
    QPixmap* getPixmap();
signals:
    void showNewImage();


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
    QPixmap* last_pixmap_;
    QList<Client*> clientConnections_;
};


#endif // SSLSERVER_H
