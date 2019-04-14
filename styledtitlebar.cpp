#include "styledtitlebar.h"

#include <QHBoxLayout>

StyledTitleBar::StyledTitleBar(QWidget* parent)
{
    StyledTitleBar("StyledTitleBar", parent);
}

StyledTitleBar::StyledTitleBar(const QString& title, QWidget* parent) :
    QWidget(parent),
    _title(title)
{
    setContentsMargins(5, 0, 5, 0);

    setMinimumHeight(12);
    setMaximumHeight(12);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    setLayout(layout);

    _qpbMenu = new QPushButton("", this);
    _qpbMinimize = new QPushButton("-", this);
    connect(_qpbMinimize, &QPushButton::clicked, this, &StyledTitleBar::onMinimizeClicked);
    _qpbClose = new QPushButton("×", this);
    connect(_qpbClose, &QPushButton::clicked, this, &StyledTitleBar::onCloseClicked);

    layout->addWidget(_qpbMenu);

    auto spacer1 = new QWidget(this);
    spacer1->setObjectName("spacer");
    spacer1->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    layout->addWidget(spacer1);

    _qlTitle = new QLabel(_title, this);
    _qlTitle->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    _qlTitle->setForegroundRole(QPalette::BrightText);
    layout->addWidget(_qlTitle);

    auto spacer2 = new QWidget(this);
    spacer2->setObjectName("spacer");
    spacer2->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    layout->addWidget(spacer2);

    layout->addWidget(_qpbMinimize);
    layout->addWidget(_qpbClose);
}

QMenu* StyledTitleBar::menu() const
{
    return _qpbMenu->menu();
}

void StyledTitleBar::setMenu(QMenu* menu)
{
    _qpbMenu->setMenu(menu);
}

void StyledTitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        emit(mouseMoved(event));
        event->accept();
    }
}

void StyledTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit(mousePressed(event));
        event->accept();
    }
}

void StyledTitleBar::setTitle(const QString& title)
{
    _title = title;
    _qlTitle->setText(title);
    emit(titleChanged(title));
}

void StyledTitleBar::onMinimizeClicked()
{
    emit(minimizeClicked());
}

void StyledTitleBar::onCloseClicked()
{
    emit(closeClicked());
}
