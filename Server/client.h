#ifndef CLIENT_H
#define CLIENT_H

//#define BENCHMARK

#include <QSslSocket>
#include <QObject>
#include <QBuffer>
#include <QImage>
#include <QVector>

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
    int next_bb_count_;
    int bb_counter_;
    QVector<QRect> last_boundingboxes_;
    QString name_;
    QString last_timestamp_;
    QString protocolName_;
    QImage* last_image_;

#ifdef BENCHMARK
    QTime* timer_;
    bool timer_running_;
#endif

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
