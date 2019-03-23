#include <QApplication>

#include <ao/ao.h>

#include "nativefilters.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    ao_initialize();

    QApplication a(argc, argv);

#ifdef HAVE_NATIVE_FILTERS
    NativeFilter filter;
    a.installNativeEventFilter(&filter);
#endif

    MainWindow w;

#ifdef HAVE_NATIVE_FILTERS
    QObject::connect(&filter, &NativeFilter::mediaKeyDownPlay, &w, &MainWindow::onPlayPressed);
    QObject::connect(&filter, &NativeFilter::mediaKeyDownNext, &w, &MainWindow::onNextPressed);
    QObject::connect(&filter, &NativeFilter::mediaKeyDownPrevious, &w, &MainWindow::onPreviousPressed);
#endif

    w.show();

    auto ret = a.exec();

    ao_shutdown();

    return ret;
}
