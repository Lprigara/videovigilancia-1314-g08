#include "mainwindow.h"
#include <QApplication>


typedef std::vector<std::vector<cv::Point> > ContoursType;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    qRegisterMetaType<std::vector<std::vector<int> > >("std::vector<std::vector<int> >");

    return a.exec();
}
