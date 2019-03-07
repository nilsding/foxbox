#include <QApplication>
#include <QStyleFactory>

#include <ao/ao.h>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    ao_initialize();

    QApplication a(argc, argv);
    //QApplication::setStyle(QStyleFactory::create("Fusion"));
    MainWindow w;
    w.show();

    auto ret = a.exec();

    ao_shutdown();

    return ret;
}
