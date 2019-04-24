#-------------------------------------------------
#
# Project created by QtCreator 2019-02-03T12:47:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = foxbox
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    statuslabel.cpp \
    playlist.cpp \
    song.cpp \
    player.cpp \
    m3uparser.cpp \
    m3uwriter.cpp \
    aboutdialog.cpp \
    styledmainwindow.cpp \
    styledtitlebar.cpp \
    songinfowindow.cpp

HEADERS += \
    mainwindow.h \
    statuslabel.h \
    playlist.h \
    song.h \
    player.h \
    m3uparser.h \
    m3uwriter.h \
    aboutdialog.h \
    nativefilters.h \
    styledmainwindow.h \
    styledtitlebar.h \
    songinfowindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += \
    /usr/local/include
LIBS += \
    -L/usr/local/lib -lopenmpt -lao

RESOURCES += \
    res.qrc

win32:VERSION = 0.1.0.0
else:VERSION = 0.1.0

mac {
    OBJECTIVE_SOURCES += \
        nativefilters_mac.mm
    HEADERS += \
        nativefilters_mac.h
    LIBS += \
        -framework AppKit
}
