#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

//#define BENCHMARK

#include <QThread>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>
#include <QFile>
#include <QDir>
#include <QBuffer>
#include <QImage>

class ClientThread : public QThread
{
    Q_OBJECT
public:
    explicit ClientThread(qintptr ID, QByteArray key, QByteArray cert, QString outputDestination, QObject *parent);
    ~ClientThread();

    void run();

signals:

public slots:
    void readByProtocol();
    void disconnected();
    void connectionFailure();

private:
    #ifdef BENCHMARK
        QTime* timer_;
        bool timer_running_;
    #endif
    QSslSocket* sslSocket_;
    QString outputDestination_;
    QByteArray key_, cert_;
    qintptr ID_;
    int frameCounter_;
    int protocol_state_;
    int next_image_size_;
    int next_bb_count_;
    int bb_counter_;
    QVector<QRect> last_boundingboxes_;
    QString name_;
    QString last_timestamp_;
    QString protocolName_;
    QImage* last_image_;

};

#endif // CLIENTTHREAD_H
