#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>
#include <QFile>
#include <QSettings>


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
    void disconnect();
    void connectionFailure();

private:
    QSslSocket* socket;
    QByteArray key;
    QByteArray certificate;
    QSettings* setting;
};


#endif // SSLSERVER_H
