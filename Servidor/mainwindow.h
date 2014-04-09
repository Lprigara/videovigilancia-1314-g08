#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QCamera>
#include <QCameraViewfinder>
#include <QtWidgets>

#include <QByteArray>
#include <QPainter>
#include <QTime>

#include <QString>

#include "preferencias.h"
#include "capturebuffer.h"
#include "sslserver.h"

#include <QSslSocket>
#include <QTcpServer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_exit_clicked();

    void on_actionSalir_triggered();

    void on_actionAcerca_de_triggered();

    void on_actionPreferencias_triggered();

    void on_actionCapturar_de_red_triggered();

    void acceptConnection();

    void startRead();

    void disconnected();

private:
    Ui::MainWindow *ui_;
    QSettings *setting_;
    QByteArray dispdefault_;
    QByteArray dispchoise_;
    QList<QByteArray> devices_;
    QSslSocket *clientConnection_;
    Server *server_;
    QString host_;
    QString key;
    QString certificate;
    int port_;
    int stateClient_;
    int sizeImage_;
};

#endif // MAINWINDOW_H
