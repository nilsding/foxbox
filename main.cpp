#include <QApplication>

#include <ao/ao.h>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    ao_initialize();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    auto ret = a.exec();

    ao_shutdown();

    return ret;
}
