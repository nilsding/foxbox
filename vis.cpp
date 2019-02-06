#include "vis.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

VisRow::VisRow(QWidget *parent) :
    QWidget(parent),
    qpbVolume(new QProgressBar),
    qlPattern(new QLabel)
{
    auto layout = new QHBoxLayout();
    layout->setMargin(0);

    qpbVolume->setOrientation(Qt::Horizontal);
    layout->addWidget(qpbVolume);

    QFont font("Monaco");
    font.setStyleHint(QFont::Monospace);
    qlPattern->setFont(font);
    layout->addWidget(qlPattern);

    setLayout(layout);
}

VisRow::~VisRow()
{
    qpbVolume->deleteLater();
    layout()->deleteLater();
}

void VisRow::setVolume(float value)
{
    auto clamped_value = static_cast<int>(
        (value > 1 ? 1 : (value < 0 ? 0 : value)) * 100
    );
    qpbVolume->setValue(clamped_value);
}

void VisRow::setPatternText(const QString& text)
{
    qlPattern->setText(text);
}

VisWindow::VisWindow(QWidget *parent) : QWidget(parent)
{
    auto layout = new QVBoxLayout();
    setLayout(layout);

    setWindowTitle("Visualisation");
}

VisWindow::~VisWindow()
{
    for (auto &row : _rows)
    {
        row->deleteLater();
    }
}

void VisWindow::setRows(int count)
{
    if (count < 0)
    {
        return;
    }

    for (auto &row : _rows)
    {
        layout()->removeWidget(row);
        row->deleteLater();
    }
    _rows.clear();

    for (int i = 0; i < count; i++)
    {
        auto row = new VisRow();
        _rows.append(row);
        layout()->addWidget(row);
    }
}

VisRow* VisWindow::at(int index)
{
    if (index < 0 || index > _rows.count())
    {
        return nullptr;
    }
    return _rows.at(index);
}

void VisWindow::onFrameUpdate(Song* song)
{
    auto count = _rows.count();
    for (int i = 0; i < count; i++)
    {
        at(i)->setVolume(song->_mod->get_current_channel_vu_mono(i));
        at(i)->setPatternText(
            QString::fromUtf8(
                song->_mod->format_pattern_row_channel(
                    song->_mod->get_current_pattern(),
                    song->_mod->get_current_row(),
                    i
                ).c_str()
            )
        );
    }
}
