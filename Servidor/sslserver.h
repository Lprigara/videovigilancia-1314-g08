#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>
#include <QFile>
#include <QSslConfiguration>

class Server: public QTcpServer
{
    Q_OBJECT

public:
    Server(QObject * parent = 0);
    void incomingConnection(qintptr socketDescriptor);
    ~Server();
signals:
    void signal();

public slots:
    void stepToMain();
    void disconnect();
    void connectionFailure();

private:
    QSslSocket* socket;
};


#endif // SSLSERVER_H
