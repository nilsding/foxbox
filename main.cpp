#include <QApplication>
#include <QFontDatabase>
#include <QStyleFactory>

#include "nativefilters.h"
#include "mainwindow.h"
#include "styledmainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("foxbox");
    a.setApplicationVersion("0.1.99");
    a.setOrganizationName("nilsding");
    a.setOrganizationDomain("org.nilsding");

    a.setStyle(QStyleFactory::create("Fusion"));
    QPalette pal;
    pal.setColor(QPalette::Base, QColor(0, 0, 0));
    pal.setColor(QPalette::Text, QColor(0, 255, 0));
    pal.setColor(QPalette::Highlight, QColor(0, 0, 255));
    pal.setColor(QPalette::HighlightedText, QColor(0, 255, 0));
    pal.setColor(QPalette::BrightText, QColor(255, 255, 255));
    pal.setColor(QPalette::ButtonText, QColor(47, 55 ,77));
    pal.setColor(QPalette::Button, QColor(189, 206, 214)); // 112, 116, 116));
    pal.setColor(QPalette::Background, QColor(52, 52, 75));
    pal.setColor(QPalette::Midlight, QColor(255, 0, 0));
    a.setPalette(pal);

    QFile f(":/stylesheet.qss");
    f.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(f.readAll());
    f.close();
    a.setStyleSheet(styleSheet);

    QFontDatabase::addApplicationFont(":/res/PxPlus_IBM_CGA.ttf");
    QFont font("PxPlus IBM CGA", 8);
    font.setPixelSize(8);
    font.setStyle(QFont::StyleNormal);
    font.setStyleStrategy(QFont::NoAntialias);
    font.setHintingPreference(QFont::PreferNoHinting);
    a.setFont(font);

#ifdef HAVE_NATIVE_FILTERS
    NativeFilter filter;
    a.installNativeEventFilter(&filter);
#endif

    StyledMainWindow sw;
    MainWindow w;
    sw.installMainWindow(&w);

#ifdef HAVE_NATIVE_FILTERS
    QObject::connect(&filter, &NativeFilter::mediaKeyDownPlay, &w, &MainWindow::onPlayPressed);
    QObject::connect(&filter, &NativeFilter::mediaKeyDownNext, &w, &MainWindow::onNextPressed);
    QObject::connect(&filter, &NativeFilter::mediaKeyDownPrevious, &w, &MainWindow::onPreviousPressed);
#endif

    sw.show();

    auto ret = a.exec();

    return ret;
}
