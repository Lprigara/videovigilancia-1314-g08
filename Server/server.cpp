#include "server.h"

Server::Server(QObject *parent, QByteArray key, QByteArray cert, QString outputDestination): QTcpServer(parent)
{
    key_ = key;
    cert_ = cert;
    outputDestination_ = outputDestination;
    if (!outputDestination_.endsWith("/")) outputDestination_.append("/");
}

Server::~Server()
{
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    //Create new clienthread for incoming connection and run it
    ClientThread* clientThread = new ClientThread(socketDescriptor, key_, cert_, outputDestination_, this);
    connect(clientThread, SIGNAL(finished()), clientThread, SLOT(deleteLater()));
    clientThread->start();
}


