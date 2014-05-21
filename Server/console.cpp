#include "console.h"

//Error: undefined reference to `Console::sigTermSd'...
//defined here
int Console::sigHupSd[2];
int Console::sigTermSd[2];
int Console::sigIntSd[2];

Console::Console(int port, QByteArray key, QByteArray cert, QString outputDestination)
{
    outputDestination_ = outputDestination;
    server_= NULL;
    port_=port;
    key_=key;
    cert_=cert;

    // Crear las parejas de sockets UNIX
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigHupSd))
        qFatal("Couldn't create HUP socketpair");
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigTermSd))
        qFatal("Couldn't create TERM socketpair");
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigIntSd))
        qFatal("Couldn't create INT socketpair");

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

    ////////////////////////////////////////
    db_ = QSqlDatabase::addDatabase("QSQLITE");

    QString path(outputDestination_);
    path.append(QDir::separator()).append("my.db.sqlite");
    path = QDir::toNativeSeparators(path);
    db_.setDatabaseName(path);

    if (!db_.open())
    QMessageBox::critical(NULL, tr("Error"), tr("No se pudo acceder a los datos."));

        QSqlQuery query;

        query.exec("create table if not exists Datos"

                   "(id integer primary key autoincrement, "

                   "client varchar(40), "

                   "timestamp long, "

                   "image varchar(200))");

        QSqlQuery query2;
        query2.exec("create table if not exists ROI"

                    "(id integer primary key, "

                    "x long, "

                    "y long, "

                    "h long, "

                    "w long, "

                    "link integer )");

    ////////////////////////////////////////
    on_actionCapturar_de_red_triggered();
}

Console::~Console()
{
    delete server_;
    delete sigHupNotifier;
    delete sigTermNotifier;
    delete sigIntNotifier;
}

void Console::on_actionCapturar_de_red_triggered()
{
    //Delete the server if it already exists
    if (server_ != NULL)
    {
        qDebug() << "Restarting server..";
        delete server_;
    }

    //Start server
    qDebug()<<port_;
    server_ = new Server(this, key_, cert_, outputDestination_);
    do{
    server_->listen(QHostAddress::Any, port_);
    qDebug()<<"escuchando";
    }while(server_->isListening()==false);
}

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

    server_->disconnect();
    on_actionCapturar_de_red_triggered();

    sigHupNotifier->setEnabled(true);
}

//Terminar proceso
void Console::handleSigTerm()
{
    qDebug("Sigterm");
    sigTermNotifier->setEnabled(false);
    char tmp;
    ::read(sigTermSd[1], &tmp, sizeof(tmp));

    server_->disconnect();
    //server_->deleteLater();
    QCoreApplication::quit();
    //deleteLater();

    sigTermNotifier->setEnabled(true);
}

//Interrumpir proceso a peticion del usuario Ctrl+C
void Console::handleSigInt()
{
    qDebug("Sigint");

    sigIntNotifier->setEnabled(false);

    char tmp;
    ::read(sigIntSd[1], &tmp, sizeof(tmp));
    server_->disconnect();
    //server_->deleteLater();
    QCoreApplication::quit();
   // deleteLater();

    sigHupNotifier->setEnabled(true);
}
