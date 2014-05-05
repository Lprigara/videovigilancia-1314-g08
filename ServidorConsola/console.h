#ifndef CONSOLE_H
#define CONSOLE_H

#include <QCamera>
#include <QCameraViewfinder>
#include <QCoreApplication>
#include <QByteArray>
#include <QString>

#include "capturebuffer.h"
#include "server.h"

#include <QSslSocket>
#include <QTcpServer>

#include <QObject>
#include <QSocketNotifier>
#include <sys/socket.h>
#include <signal.h>

int setupUnixSignalHandlers();



class Console : public QObject
{
    Q_OBJECT

public:
    explicit Console(int port, QByteArray key, QByteArray cert);
    ~Console();

    // Manejadores de señal POSIX
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);
    static void intSignalHandler(int unused);

private slots:

    void on_actionCapturar_de_red_triggered();

public slots:

    // Slots Qt donde atender las señales POSIX
    void handleSigHup();
    void handleSigTerm();
    void handleSigInt();


private:
    Server *server_;
    int port_;
    QByteArray key_;
    QByteArray cert_;

    // Pares de sockets. Un par por señal a manejar
    static int sigHupSd[2];
    static int sigTermSd[2];
    static int sigIntSd[2];

    // Objetos para monitorizar los pares de sockets
    QSocketNotifier *sigHupNotifier;
    QSocketNotifier *sigTermNotifier;
    QSocketNotifier *sigIntNotifier;
};

#endif // MAINWINDOW_H
