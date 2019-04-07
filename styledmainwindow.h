#ifndef STYLEDMAINWINDOW_H
#define STYLEDMAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include <QMenu>

#include "styledtitlebar.h"
#include "mainwindow.h"

class StyledMainWindow : public QMainWindow
{
    Q_OBJECT
    friend class MainWindow;

public:
    explicit StyledMainWindow(QWidget* parent = nullptr);
    void installMainWindow(MainWindow* w);

signals:

public slots:
    void onMinimizeClicked();
    void onCloseClicked();

private:
    QMainWindow* _mainWindow;
    StyledTitleBar* _stb;
    QMenu* _menu;
    QAction* _qaFoxbox;

    QPoint _dragPosition;

    void initializeComponents();

private slots:
    void onTitleBarMouseMoved(QMouseEvent* event);
    void onTitleBarMousePressed(QMouseEvent* event);
    void onqaFoxboxTriggered();
};

#endif // STYLEDMAINWINDOW_H
