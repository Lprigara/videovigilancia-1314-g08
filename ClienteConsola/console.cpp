#include "console.h"
#include "capturebuffer.h"


//Error: undefined reference to `Console::sigTermSd'...
//defined here
int Console::sigHupSd[2];
int Console::sigTermSd[2];
int Console::sigIntSd[2];

Console::Console(QString host, int port, QString nameClient, int device)
{
//    movie_=NULL;
    camera_=NULL;
    viewfinder_=NULL;
    captureB_=NULL;
    sslSocket_=NULL;

    clientName_=nameClient;
    port_=port;
    host_=host;

    devices_=QCamera::availableDevices();
    dispdefault_ = devices_[0];

    if (device == 0)
        dispchoise_=devices_[0];
    else if(device == 1)
        dispchoise_=devices_[1];
    else
        qDebug()<<"Dispositivo no encontrado";

    //QtNetwork
    connectedServer_=0;

    //bool para no reconectar al desconectarse
    exit_=false;

    // Crear las parejas de sockets UNIX
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigHupSd))
        qFatal("Couldn't create HUP socketpair");
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigTermSd))
        qFatal("Couldn't create TERM socketpair");
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigIntSd))
        qFatal("Couldn't create TERM socketpair");

    // Crear los objetos para monitorizar uno de los socket
    // de cada pareja.
    sigHupNotifier = new QSocketNotifier(sigHupSd[1],
        QSocketNotifier::Read, this);
    sigTermNotifier = new QSocketNotifier(sigTermSd[1],
        QSocketNotifier::Read, this);
    sigIntNotifier = new QSocketNotifier(sigIntSd[1],
        QSocketNotifier::Read, this);

    // Conectar la señal activated() de cada objeto
    // QSocketNotifier con el slot correspondiente. Esta señal
    // será emitida cuando hayan datos para ser leidos en el
    // socket monitorizado.
    connect(sigHupNotifier, SIGNAL(activated(int)), this,
        SLOT(handleSigHup()));
    connect(sigTermNotifier, SIGNAL(activated(int)), this,
        SLOT(handleSigTerm()));
    connect(sigIntNotifier, SIGNAL(activated(int)), this,
        SLOT(handleSigInt()));


    on_actionCapturar_triggered();    
}

Console::~Console()
{
//    delete movie_;
    if (camera_ != NULL) delete camera_;
    delete viewfinder_;
    delete setting_;
    delete captureB_;
    if (sslSocket_ != NULL) delete sslSocket_;

    delete sigHupNotifier;
    delete sigTermNotifier;
    delete sigIntNotifier;
}


/*//Funcion para abrir archivo de video
void Console::on_actionAbrir_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Abrir archivo", QString() ,"Video (*.mjpeg)"); //te devuelve el nombre del archivo
    if(!fileName.isEmpty()) {
        QPixmap foto(fileName);
        if(foto.isNull()) {
            QMessageBox::information(this, "Abrir archivo", "El archivo no pudo ser abierto. ");
        }
        else {
            movie_ = new QMovie(fileName);
            //ui_->label->setMovie(movie_);
            //if(ui_->autoinicio->isChecked())
                movie_->start();

        }//endelse
    }//endif
}
*/

void Console::on_actionCapturar_triggered()
{ 
     if(camera_ != NULL)
     {
        camera_->stop();
        delete camera_;
     }
     if(operator!= (dispdefault_,dispchoise_)){  
        camera_ = new QCamera(dispchoise_);
     }
     else{
        camera_ = new QCamera(dispdefault_);
     }

     captureB_ = new captureBuffer;
     camera_->setViewfinder(captureB_);
     camera_->setCaptureMode(QCamera::CaptureViewfinder);

     //Connect to the server
     qDebug() << host_ << port_;

     if (sslSocket_ != NULL)
     {
         delete sslSocket_;
         qDebug() << "Restarting socket..";
     }

     sslSocket_ = new QSslSocket(this);

     connect(sslSocket_, SIGNAL(disconnected()), this, SLOT(disconnect()));
     connect(sslSocket_, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(socketError()));
     sslSocket_->ignoreSslErrors();
     connect(captureB_, SIGNAL(signalImage(QImage)), this, SLOT(image1(QImage)));
     connect(sslSocket_, SIGNAL(encrypted()), this, SLOT(connected()));
     sslSocket_->connectToHostEncrypted(host_, port_);
     sslSocket_->ignoreSslErrors();
}

void Console::image1(QImage image)
{
    if(connectedServer_)
    {
        //Codificar la imagen para enviarla por la red
        QBuffer buffer;
        QImageWriter writer(&buffer, "jpeg"); //Para controlar el nivel de compresión, el de gamma o algunos otros parámetros específicos del formato, tendremos que emplear un objeto QImageWriter.
        QImage imageSend; //creación de la imagen a enviar

        imageSend=image;
        writer.setCompression(70);//codifica la imagen
        writer.write(imageSend); //aplicar lo anterior a la imagen

        QByteArray bytes = buffer.buffer();
        int sizeImg=bytes.size(); //tamaño de la imagen

        QByteArray name;
        name.append(clientName_); //nombre del cliente convertido a String para poder enviarlo

        qint64 timestamp = QDateTime::currentMSecsSinceEpoch(); //tiempo en milisegundos desde EPOC hasta el instante de la imagen

        QByteArray protocol;
        protocol.append("GLP/1.0");
        qDebug()<<"Enviando";
        //Envio de los distintos campos separados por \n
        sslSocket_->write(protocol);
        sslSocket_->write("\n");
        sslSocket_->write(name);
        sslSocket_->write("\n");
        sslSocket_->write(QByteArray::number(timestamp));
        sslSocket_->write("\n");
        sslSocket_->write(QByteArray::number(sizeImg));
        sslSocket_->write("\n");
        sslSocket_->write(bytes);
    }
    else
    {
        return;
    }
}

void Console::connected()
{
    connectedServer_ = 1;
    qDebug() << "Connected to server";
    camera_->start();
}

void Console::disconnect()
{
    connectedServer_ = 0;
    qDebug() << "Disconnected from server";
    if(!exit_){
        reconnect();
    }
}

void Console::socketError()
{
  qDebug() << "Socket error: " << sslSocket_->errorString();
  sslSocket_->close();
}

void Console::reconnect()
{
    QTimer *timer = new QTimer(this);
    timer->start(7000);

    if(sslSocket_->state() == QAbstractSocket::UnconnectedState)
    {
        sslSocket_->connectToHostEncrypted(host_, port_);
        sslSocket_->ignoreSslErrors();
        sslSocket_->waitForEncrypted(8000);

        qDebug()<<"Reconecting client: "<< clientName_ ;
    }
    if(sslSocket_->state() == QAbstractSocket::ConnectedState)
    {
        timer->stop();
        qDebug()<<"Reconnected client succesful";
    }
    connect(sslSocket_, SIGNAL(encrypted()), this, SLOT(connected()));
    connect(timer,SIGNAL(timeout()),this,SLOT(reconnect()));
}

//
// Manejador de la señal SIGHUP
//
void Console::hupSignalHandler(int)
{
    char a = 1;
    ::write(sigHupSd[0], &a, sizeof(a));
}

//
// Manejador de la señal SIGTERM
//
void Console::termSignalHandler(int)
{
    char a = 1;
    ::write(sigTermSd[0], &a, sizeof(a));
}

void Console::intSignalHandler(int)
{
    char a = 1;
    ::write(sigIntSd[0], &a, sizeof(a));
}

//
// Configurar los manejadores de señal
//
int setupUnixSignalHandlers()
{
    struct ::sigaction hup, term, ints;

    hup.sa_handler = &Console::hupSignalHandler;
    ::sigemptyset(&hup.sa_mask);
    hup.sa_flags = SA_RESTART;

    // Establecer manejador de la señal SIGHUP
    if (::sigaction(SIGHUP, &hup, 0) > 0)
    return 1;

    term.sa_handler = &Console::termSignalHandler;
    ::sigemptyset(&term.sa_mask);
    term.sa_flags = SA_RESTART;

    // Establecer manejador de la señal SIGTERM
    if (::sigaction(SIGTERM, &term, 0) > 0)
    return 2;

    ints.sa_handler = &Console::intSignalHandler;
    ::sigemptyset(&ints.sa_mask);
    ints.sa_flags = SA_RESTART;

    // Establecer manejador de la señal SIGINT
    if (::sigaction(SIGINT, &ints, 0) > 0)
    return 3;


    return 0;
}

//Cerrar conexiones y reiniciar servicio
void Console::handleSigHup()
{
    qDebug("Sighup");
    sigHupNotifier->setEnabled(false);
    char tmp;
    ::read(sigHupSd[1], &tmp, sizeof(tmp));

    sslSocket_->disconnect();
    sslSocket_->deleteLater();
    sslSocket_=new QSslSocket;
    reconnect();

    sigHupNotifier->setEnabled(true);
}

//Terminar proceso
void Console::handleSigTerm()
{
    qDebug("Sigterm");
    sigTermNotifier->setEnabled(false);
    char tmp;
    ::read(sigTermSd[1], &tmp, sizeof(tmp));

    sslSocket_->disconnect();
    sslSocket_->deleteLater();
    qDebug("Disconnected");
    QCoreApplication::quit();

    sigTermNotifier->setEnabled(true);
}

//Interrumpir proceso a peticion del usuario Ctrl+C
void Console::handleSigInt()
{
    sigIntNotifier->setEnabled(false);
    char tmp;
    ::read(sigIntSd[1], &tmp, sizeof(tmp));

    qDebug("Sigint");
    sslSocket_->disconnect();
    sslSocket_->deleteLater();
    qDebug("Disconnected");
    QCoreApplication::quit();

    sigHupNotifier->setEnabled(true);
}
