#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    qRegisterMetaType<std::vector<QRect > >("std::vector<QRect >");

    return a.exec();
}
