#ifndef CLIENT_H
#define CLIENT_H

#include <QSslSocket>
#include <QObject>
#include <QBuffer>
#include <QImage>

class Client : public QObject
{
    Q_OBJECT

public:
    Client(QSslSocket* sslSocket);
    ~Client();

    QSslSocket* getSocket();
    QString getName();
    QImage* getImage();
    void readByProtocol();

private:
    QSslSocket* sslSocket_;
    int protocol_state_;
    int next_image_size_;
    QString name_;
    QString last_timestamp_;
    QString protocolName_;
    QImage* last_image_;

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
