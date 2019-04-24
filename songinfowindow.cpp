#include "songinfowindow.h"

#include <QVBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QLabel>
#include <QPlainTextEdit>
#include <QDebug>

//! Range of pixels the window should snap to the screen border
#define SNAP_RANGE 10

SongInfoWindow::SongInfoWindow(Song* song, QDialog* parent) :
    QDialog(parent),
    _song(song)
{
    setWindowFlag(Qt::FramelessWindowHint, true);
    setMinimumHeight(12 + 240);
    setWindowTitle(tr("Song Information"));

    initializeComponents();
}

void SongInfoWindow::onMinimizeClicked()
{
    this->setWindowState(Qt::WindowMinimized);
}

void SongInfoWindow::onCloseClicked()
{
    close();
}

void SongInfoWindow::resizeEvent(QResizeEvent* /* event */)
{
    moveSizeGrip();
}

void SongInfoWindow::initializeComponents()
{
    setContentsMargins(0, 0, 0, 0);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    //=========================================================================
    // title bar
    _stb = new StyledTitleBar(tr("Song Information"), this);
    layout->addWidget(_stb);

    connect(_stb, &StyledTitleBar::minimizeClicked, this, &SongInfoWindow::onMinimizeClicked);
    connect(_stb, &StyledTitleBar::closeClicked, this, &SongInfoWindow::onCloseClicked);
    connect(_stb, &StyledTitleBar::mouseMoved, this, &SongInfoWindow::onTitleBarMouseMoved);
    connect(_stb, &StyledTitleBar::mousePressed, this, &SongInfoWindow::onTitleBarMousePressed);

    //=========================================================================
    // samples widget
    _samplesWidget = new QWidget;
    auto samplesWidgetLayout = new QVBoxLayout();
    _samplesWidget->setLayout(samplesWidgetLayout);
    auto samples = _song->_mod->get_sample_names();
    for (auto sampleName : samples) {
        auto l = new QLabel(sampleName.c_str());
        samplesWidgetLayout->addWidget(l);
    }

    //=========================================================================
    // instruments widget
    _instrumentsWidget = new QWidget;
    auto instrumentsWidgetLayout = new QVBoxLayout();
    _instrumentsWidget->setLayout(instrumentsWidgetLayout);
    auto instruments = _song->_mod->get_instrument_names();
    for (auto instrumentName : instruments) {
        auto l = new QLabel(instrumentName.c_str());
        instrumentsWidgetLayout->addWidget(l);
    }

    //=========================================================================
    // song comments widget
    _songCommentWidget = new QWidget;
    auto songCommentWidgetLayout = new QVBoxLayout();
    _songCommentWidget->setLayout(songCommentWidgetLayout);
    auto pte = new QPlainTextEdit();
    songCommentWidgetLayout->addWidget(pte);
    pte->setReadOnly(true);
    auto msg_cpp = _song->_mod->get_metadata("message_raw");
    pte->setPlainText(QString::fromLatin1(msg_cpp.c_str()));

    //=========================================================================
    // tab widget
    _qtw = new QTabWidget(this);
    _qtw->addTab(_samplesWidget, tr("Samples"));
    _qtw->addTab(_instrumentsWidget, tr("Instruments"));
    _qtw->addTab(_songCommentWidget, tr("Song Comment"));
    _qtw->setMinimumSize(320, 240);
    layout->addWidget(_qtw);

    //=========================================================================
    // size grip
    _sizeGrip = new QSizeGrip(this);
    _sizeGrip->setMaximumSize(16, 16);
}

void SongInfoWindow::moveSizeGrip()
{
    auto size = _sizeGrip->size();
    QPoint targetPosition(
        width() - size.width(),
        height() - size.height()
    );
    _sizeGrip->move(targetPosition);
}

void SongInfoWindow::onTitleBarMouseMoved(QMouseEvent* event)
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

void SongInfoWindow::onTitleBarMousePressed(QMouseEvent* event)
{
    _dragPosition = event->globalPos() - frameGeometry().topLeft();
    event->accept();
}
