#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>

#include "clientthread.h"


class Server: public QTcpServer
{
    Q_OBJECT

public:
    Server(QObject * parent, QByteArray key, QByteArray cert, QString outputDestination);
    void incomingConnection(qintptr socketDescriptor);
    ~Server();

signals:

public slots:

private:
    QString outputDestination_;
    QByteArray key_, cert_;

};


#endif // SSLSERVER_H
