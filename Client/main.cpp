#include "console.h"
//#include <QApplication>
#include <QCoreApplication>
#include <string.h>
#include <iostream>

using namespace std;


int main(int argc, char *argv[])
{
    QByteArray disp, port;
    QString host, nameClient;
    if (argc >= 2)
    {
        for(int i = 1; i < argc; ++i){
            if(strcmp(argv[i],"h") == 0 || strcmp(argv[i], "help") == 0 || strcmp(argv[i], "?") == 0)
            {
                cout << "Cambiar dispositivo: -d o --disp <num dispositivo>" <<  endl;
                cout << "Cambiar puerto: -p o --port <puerto>" <<  endl;
                cout << "Cambiar host: -o o --host <host>" <<  endl;
                cout << "Cambiar nombre cliente: n o --name" << endl;
                //exit(0);
            }

            if(strcmp(argv[i],"d") == 0 || strcmp(argv[i], "disp") == 0)
            {
                 disp = argv[i+1];
            }
            if(strcmp(argv[i],"p") == 0 || strcmp(argv[i], "port") == 0)
            {
                 port = argv[i+1];
            }
            if(strcmp(argv[i],"o") == 0 || strcmp(argv[i], "host") == 0)
            {
                 host = argv[i+1];
            }
            if(strcmp(argv[i],"n") == 0 || strcmp(argv[i], "name") == 0)
            {
                 nameClient = argv[i+1];
            }
        }
    }

    QCoreApplication a(argc, argv);
    Console m(host,port.toInt(),nameClient, disp.toInt());
    setupUnixSignalHandlers();


    qRegisterMetaType<std::vector<QRect > >("std::vector<QRect >");

    return a.exec();
}

