#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>

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
