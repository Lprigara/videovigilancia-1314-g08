#include "mainwindow.h"
#include "ui_mainwindow.h"



bool Paused = true;  //variable para parar/pausar

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    clientCounter = 0;

    server_=NULL;
    setting_ = new QSettings("Leonor", "viewer"); //configura QSetting
    key = setting_->value("key", "").toString();
    certificate = setting_->value("certificate", "").toString();
}

MainWindow::~MainWindow()
{
    qDeleteAll(clientConnections);
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

void MainWindow::on_actionPreferencias_triggered()
{
   Preferencias prefe(this);
   prefe.exec();
}

void MainWindow::on_actionCapturar_de_red_triggered()
{
    //Start server with the port specified in settings
    port_ = setting_->value("viewer/port", "9600").toInt();
    server_ = new Server(this);
    server_->listen(QHostAddress::Any,port_);

    //Connect signal for incoming connections
    connect(server_, SIGNAL(newConnection()), this, SLOT(handleNewConnections()));
}

void MainWindow::handleNewConnections()
{
    while(server_->hasPendingConnections())
    {
        QSslSocket* next_connection = dynamic_cast<QSslSocket *>(server_->nextPendingConnection());
        QString next_clientName = "Client" + clientCounter;
        Client* next_client = new Client(next_clientName, next_connection);
        clientCounter++;

        connect(next_client, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
        connect(next_client, SIGNAL(readyRead()), this, SLOT(readClientData()));

        qDebug() << "Opened connection with client " + next_client->getName();
    }
}

void MainWindow::readClientData()
{
    Client* client = qobject_cast<Client *>(sender());

    if (!client)
        return;

    client->readByProtocol();
}

void MainWindow::clientDisconnected(){
    Client* client = qobject_cast<Client *>(sender());

    if (!client)
        return;

    clientConnections.removeAll(client);
    client->deleteLater();
    qDebug() << "Closed connection with client " + client->getName();
}

