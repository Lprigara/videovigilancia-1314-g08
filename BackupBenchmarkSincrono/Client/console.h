#ifndef CONSOLE_H
#define CONSOLE_H


#include <QSettings>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCoreApplication>
#include <QFileDialog>
#include <QMessageBox>

#include <QByteArray>
#include <QMovie>
#include <QBuffer>
#include <QImageWriter>
#include <QTimer>

#include <QString>

#include "capturebuffer.h"

#include <QSslSocket>

#include <QDateTime>

#include <QObject>
#include <QSocketNotifier>
#include <sys/socket.h>
#include <signal.h>

#include <QThread>
#include <unistd.h>

#include "motiondetector.h"

int setupUnixSignalHandlers();

class Console : public QObject
{
    Q_OBJECT

public:
    explicit Console(QString host, int port, QString nameClient, int device);
    ~Console();

    // Manejadores de señal POSIX
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);
    static void intSignalHandler(int unused);



signals:
    void readyForMotionDetection(QImage img);
    void resetMotionDetection();

public slots:

    // Slots Qt donde atender las señales POSIX
    void handleSigHup();
    void handleSigTerm();
    void handleSigInt();

private slots:

    void on_actionCapturar_triggered();

    void image1(QImage image); //Slot para modificar los frames

    void connected();

    void disconnect();

    void socketError();

    void reconnect();

    void motionDetected(std::vector<QRect > boundingRects, QImage image);

private:
    QMovie *movie_;
    QSettings *setting_;
    QCamera *camera_;
    QCameraViewfinder *viewfinder_;
    QByteArray dispdefault_;
    QByteArray dispchoise_;
    captureBuffer *captureB_;
    QSslSocket *sslSocket_;
    QList<QByteArray> devices_;
    bool connectedServer_;
    QString host_;
    int port_;
    bool exit_;
    QString clientName_;
    MotionDetector* mDetect_;
    QThread *mThread_;

    // Pares de sockets. Un par por señal a manejar
    static int sigHupSd[2];
    static int sigTermSd[2];
    static int sigIntSd[2];

    // Objetos para monitorizar los pares de sockets
    QSocketNotifier *sigHupNotifier;
    QSocketNotifier *sigTermNotifier;
    QSocketNotifier *sigIntNotifier;

};

#endif // CONSOLE_H
