#include "console.h"
#include <QCoreApplication>
#include <iostream>
#include "errno.h"
#include "stdio.h"
#include <syslog.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

using namespace std;
int main(int argc, char *argv[])
{
    QByteArray port, key, cert;
    if (argc >= 2)
    {
        for(int i = 1; i < argc; ++i){
            if(strcmp(argv[i],"h") == 0 || strcmp(argv[i], "help") == 0 || strcmp(argv[i], "?") == 0)
            {
                cout << "Cambiar puerto: -p o --port <puerto>" <<  endl;
                cout << "Cambiar ruta de clave: -k o --key <ruta>" <<  endl;
                cout << "Cambiar ruta de certificado: -c o --cert <ruta>" <<  endl;
                //exit(0);
            }

            if(strcmp(argv[i],"p") == 0 || strcmp(argv[i], "port") == 0)
            {
                 port = argv[i+1];
            }
            if(strcmp(argv[i],"k") == 0 || strcmp(argv[i], "key") == 0)
            {
                 key = argv[i+1];
            }
            if(strcmp(argv[i],"c") == 0 || strcmp(argv[i], "cert") == 0)
            {
                 cert = argv[i+1];
            }
            if(strcmp(argv[i],"d") == 0 || strcmp(argv[i], "daemon") == 0)
            {

                 pid_t pid;
                 pid_t sid;

                 // Nos clonamos a nosotros mismos creando un proceso hijo
                 pid = fork();

                 // Si pid es < 0, fork() falló
                 if (pid < 0) {
                     // Mostrar la descripción del error y terminar
                     std::cerr << strerror(errno) << std::endl;
                     exit(10);
                 }

                 // Si pid es > 0, estamos en el proceso padre
                 if (pid > 0) {
                     // Terminar el proceso
                     exit(0);
                 }

                 // Si la ejecución llega a este punto, estamos en el hijo

                 //Cambiar umask,  autorizar todos los permisos
                 umask(0);

                 // Abrir una conexión al demonio syslog
                 openlog(argv[0], LOG_NOWAIT | LOG_PID, LOG_USER);


                 // Intentar crear una nueva sesión
                 sid = setsid();
                 if (sid < 0) {
                     syslog(LOG_ERR, "No fue posible crear una nueva sesión\n");
                     exit(11);
                 }

                 // Cambiar el directorio de trabajo de proceso
                 if ((chdir("/")) < 0) {
                     syslog(LOG_ERR, "No fue posible cambiar el directorio de "
                                     "trabajo a /\n");
                     exit(12);
                 }

                 // Cerrar los descriptores de la E/S estandar
                 close(STDIN_FILENO);            // fd 0
                 close(STDOUT_FILENO);           // fd 1
                 close(STDERR_FILENO);           // fd 2

                 //Los reabre pero conectados al archivo estandar /dev/null
                 int fd0 = open("/dev/null", O_RDONLY);  // fd0 == 0
                 int fd1 = open("/dev/null", O_WRONLY);  // fd0 == 1
                 int fd2 = open("/dev/null", O_WRONLY);  // fd0 == 2

                 // Cambiar el usuario efectivo del proceso a 'midemonio'
                // struct passwd* user = getpwnam("midemonio");
                // seteuid(user->pw_uid);

                 // Cambiar el grupo efectivo del proceso a 'midemonio'
               //  struct group* group = getgrnam("midemonio");
               // setegid(group->gr_gid);

                 // Enviar un mensaje al demonio syslog
                 syslog(LOG_NOTICE, "Demonio iniciado con éxito\n");

                 // Archivo que contiene identificador de proceso del demonio
                 QFile file("/var/run/daemon.pid");
                 QTextStream out(&file);
                 out<<pid;
                 file.close();

                 // Cuando el demonio termine, cerrar la conexión con
                 // el servicio syslog
                 closelog();
            }
        }
    }

    QCoreApplication a(argc, argv);
    Console m(port.toInt(),key, cert);
    setupUnixSignalHandlers();
    return a.exec();
}
