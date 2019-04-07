#include "styledmainwindow.h"

#include <QToolBar>
#include <QVBoxLayout>

#include "aboutdialog.h"

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
    centralWidget()->layout()->addWidget(_mainWindow);
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

    connect(_stb, &StyledTitleBar::minimizeClicked, this, &StyledMainWindow::onMinimizeClicked);
    connect(_stb, &StyledTitleBar::closeClicked, this, &StyledMainWindow::onCloseClicked);
    connect(_stb, &StyledTitleBar::mouseMoved, this, &StyledMainWindow::onTitleBarMouseMoved);
    connect(_stb, &StyledTitleBar::mousePressed, this, &StyledMainWindow::onTitleBarMousePressed);

    //=========================================================================
    // menu items
    _menu = new QMenu(this);
    _qaFoxbox = new QAction("foxbox 0.1.0", _menu);
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
    move(event->globalPos() - _dragPosition);
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
        _stb->setTitle("foxbox");
        _mainWindow->show();
    });
    _stb->setTitle("About foxbox");
    _mainWindow->hide();
    centralWidget()->layout()->addWidget(ad);
}
