#ifndef CLIENT_H
#define CLIENT_H

#include <QSslSocket>
#include <QObject>
#include <QBuffer>
#include <QPainter>
#include <QPixmap>

class Client : public QObject
{
    Q_OBJECT

public:
    Client(QString name, QSslSocket* sslSocket);
    ~Client();

    QSslSocket* getSocket();
    QString getName();
    void readByProtocol();

private:
    QSslSocket* sslSocket;
    int protocol_state;
    int next_image_size;
    QString name;

private slots:
    void onDisconnected();
    void onReadyRead();

signals:
    void disconnected();
    void readyRead();

};

#endif // CLIENT_H