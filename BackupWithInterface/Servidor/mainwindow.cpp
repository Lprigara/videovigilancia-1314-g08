#include "mainwindow.h"
#include "ui_mainwindow.h"

bool Paused = true;  //variable para parar/pausar

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    server_= NULL;
    setting_ = new QSettings("Leonor", "viewer"); //configura QSetting
}

MainWindow::~MainWindow()
{
    delete ui_;
    delete setting_;
    delete server_;
}

void MainWindow::on_exit_clicked()
{
    qApp->quit(); //qApp = QApplication del main
}
void MainWindow::on_actionSalir_triggered()
{
    qApp->quit();
}

void MainWindow::on_actionAcerca_de_triggered()
{
    Acerca acercaDe(this);
    acercaDe.exec();
}

void MainWindow::on_actionCapturar_de_red_triggered()
{
    //Delete the server if it already exists
    if (server_ != NULL)
    {
        qDebug() << "Restarting server..";
        delete server_;
    }

    //Start server with the port specified in settings
    int port = setting_->value("viewer/port", "9600").toInt();
    server_ = new Server(this);
    server_->listen(QHostAddress::Any,port);

    //connect a signal to show the newest received image of the first client in the servers list
    connect(server_, SIGNAL(showNewImage()), this, SLOT(on_showNewImage()));
}

void MainWindow::on_showNewImage()
{
    ui_->label->setPixmap(*server_->getPixmap());
}

