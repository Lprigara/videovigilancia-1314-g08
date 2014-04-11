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
    Client(QSslSocket* sslSocket);
    ~Client();

    QSslSocket* getSocket();
    QString getName();
    QPixmap* getPixmap();
    void readByProtocol();

private:
    QSslSocket* sslSocket;
    int protocol_state;
    int next_image_size;
    QString name;
    QString last_timestamp;
    QPixmap* last_pixmap;

private slots:
    void connectionFailure();
    void onDisconnected();
    void onReadyRead();

signals:
    void disconnected();
    void readyRead();
    void receivedCompletePackage();

};

#endif // CLIENT_H
