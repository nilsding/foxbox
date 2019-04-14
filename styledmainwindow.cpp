#include "styledmainwindow.h"

#include <QApplication>
#include <QToolBar>
#include <QVBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QMenuBar>

#include "aboutdialog.h"

//! Range of pixels the window should snap to the screen border
#define SNAP_RANGE 10

StyledMainWindow::StyledMainWindow(QWidget* parent) :
    QMainWindow(parent)
{
    setWindowFlag(Qt::FramelessWindowHint, true);
    setMinimumHeight(12);

    initializeComponents();
}

void StyledMainWindow::installMainWindow(MainWindow* w)
{
    _mainWindow = w;
    setMinimumHeight(minimumHeight() + 37);
    setWindowTitle(_mainWindow->windowTitle());
    centralWidget()->layout()->addWidget(_mainWindow);
    auto menuBar = w->menuBar();
    auto entries = menuBar->findChildren<QMenu*>();

    for (auto &entry : entries)
    {
        _menu->addMenu(entry);
    }
}

void StyledMainWindow::onMinimizeClicked()
{
    this->setWindowState(Qt::WindowMinimized);
}

void StyledMainWindow::onCloseClicked()
{
    close();
}

void StyledMainWindow::resizeEvent(QResizeEvent* /* event */)
{
    moveSizeGrip();
}

void StyledMainWindow::initializeComponents()
{
    setContentsMargins(0, 0, 0, 0);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    this->setCentralWidget(new QWidget(this));
    centralWidget()->setLayout(layout);

    //=========================================================================
    // title bar
    _stb = new StyledTitleBar("foxbox", this);
    layout->addWidget(_stb);

    connect(this, &StyledMainWindow::windowTitleChanged, _stb, &StyledTitleBar::setTitle);
    connect(_stb, &StyledTitleBar::minimizeClicked, this, &StyledMainWindow::onMinimizeClicked);
    connect(_stb, &StyledTitleBar::closeClicked, this, &StyledMainWindow::onCloseClicked);
    connect(_stb, &StyledTitleBar::mouseMoved, this, &StyledMainWindow::onTitleBarMouseMoved);
    connect(_stb, &StyledTitleBar::mousePressed, this, &StyledMainWindow::onTitleBarMousePressed);

    //=========================================================================
    // menu items
    _menu = new QMenu(this);
    _qaFoxbox = new QAction(QString("foxbox %1").arg(QApplication::instance()->applicationVersion()), _menu);
    connect(_qaFoxbox, &QAction::triggered, this, &StyledMainWindow::onqaFoxboxTriggered);
    _menu->addAction(_qaFoxbox);
    _menu->addSeparator();
    _stb->setMenu(_menu);

    //=========================================================================
    // size grip
    _sizeGrip = new QSizeGrip(this);
    _sizeGrip->setMaximumSize(16, 16);
}

void StyledMainWindow::moveSizeGrip()
{
    auto size = _sizeGrip->size();
    QPoint targetPosition(
        width() - size.width(),
        height() - size.height()
    );
    _sizeGrip->move(targetPosition);
}

void StyledMainWindow::onTitleBarMouseMoved(QMouseEvent* event)
{
    auto targetPos = event->globalPos() - _dragPosition;
    // TODO: handle multi-monitor setups
    auto screenSize = QGuiApplication::primaryScreen()->size();

    if (qAbs(targetPos.x()) < SNAP_RANGE)
    {
        targetPos.setX(0);
    }
    if (qAbs(targetPos.y()) < SNAP_RANGE)
    {
        targetPos.setY(0);
    }
    if (qAbs(targetPos.x() + width() - screenSize.width()) < SNAP_RANGE)
    {
        targetPos.setX(screenSize.width() - width());
    }
    if (qAbs(targetPos.y() + height() - screenSize.height()) < SNAP_RANGE)
    {
        targetPos.setY(screenSize.height() - height());
    }

    move(targetPos);
}

void StyledMainWindow::onTitleBarMousePressed(QMouseEvent* event)
{
    _dragPosition = event->globalPos() - frameGeometry().topLeft();
    event->accept();
}

void StyledMainWindow::onqaFoxboxTriggered()
{
    if (_mainWindow->isHidden())
    {
        return;
    }

    auto ad = new AboutDialog(this);
    connect(ad, &AboutDialog::accepted, this, [&] {
        setWindowTitle(_mainWindow->windowTitle());
        _mainWindow->show();
    });
    setWindowTitle(ad->windowTitle());
    _mainWindow->hide();
    centralWidget()->layout()->addWidget(ad);
}
