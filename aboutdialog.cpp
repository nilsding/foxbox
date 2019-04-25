#include "aboutdialog.h"

#include <QPainter>
#include <QtMath>
#include <QFontMetricsF>

#define SEPARATOR "  ×××  "
QString AboutDialog::aboutText =
        "foxbox -- nilsding's personal player for tracker music"
        " using libopenmpt as backend"
        SEPARATOR
        "Yes, there had to be a scroller in the about box.  "
        "Also, I have no idea what to write here ... maybe I'll extend this "
        "in a future version..."
        SEPARATOR
        "Greetings go out to: "
        "Mh, pixeldesu, coderobe, Fisk, seatsea, and all the others I forgot"
        SEPARATOR
        "The font used throughout foxbox is called \"PxPlus IBM CGA\", which "
        "was created by VileR in 2015/2016 and is provided under the CC BY-SA "
        "4.0 licence.  https://int10h.org"
        SEPARATOR
        "I hope you enjoy this fine piece of software ;-)";
#undef SEPARATOR

AboutDialog::AboutDialog(QWidget* parent) :
    QDialog(parent),
    _refreshTimer(new QTimer),
    _textWidthTable(new QList<qreal>)
{
    // setup ui
    resize(580, 160);
    setWindowTitle("About foxbox");
    setStyleSheet(
      "background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(72, 72, 72, 255), stop:1 rgba(0, 0, 0, 255));"
      "color: #ffff00;"
    );
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    buildTextWidthTable();

    _startTime.start();
    connect(_refreshTimer, &QTimer::timeout, this, [&]()
    {
        repaint();
    });
    _refreshTimer->start(15);
}

AboutDialog::~AboutDialog()
{
    _refreshTimer->stop();
    _refreshTimer->deleteLater();
    delete _textWidthTable;
}

void AboutDialog::closeEvent(QCloseEvent* event)
{
    event->accept();
}

void AboutDialog::paintEvent(QPaintEvent* /* event */)
{
    QPainter painter(this);

    QFont font("PxPlus IBM CGA", 16);
    font.setPixelSize(16);
    font.setStyle(QFont::StyleNormal);
    font.setStyleStrategy(QFont::NoAntialias);
    font.setHintingPreference(QFont::PreferNoHinting);
    painter.setFont(font);

    for (int i = 0; i < aboutText.size(); i++)
    {
        auto textWidth = _textWidthTable->at(i);

        qreal x = width() + textWidth - (_startTime.elapsed() / 10.0);
        qreal y = (qSin(2 * M_PI * (static_cast<qreal>(i) / aboutText.size()) + x / 27)) * 25 * qSin(3 * M_PI * _startTime.elapsed() / 15000.0 + x / 100.0) + (height() / 2.0);
        painter.drawText(QPointF(x, y), QString(aboutText[i]));

        if (i == aboutText.size() - 1 && x < -25.0)
        {
            _startTime.start();
        }
    }
}

void AboutDialog::mousePressEvent(QMouseEvent* /*event*/)
{
    accept();
}

void AboutDialog::buildTextWidthTable()
{
    QFont font("PxPlus IBM CGA", 16);
    font.setPixelSize(16);
    font.setStyle(QFont::StyleNormal);
    font.setStyleStrategy(QFont::NoAntialias);
    font.setHintingPreference(QFont::PreferNoHinting);
    QFontMetricsF fontMetrics(font);

    _textWidthTable->clear();
    for (int i = 0; i < aboutText.size(); i++)
    {
         _textWidthTable->append(fontMetrics.width(aboutText.left(i + 1)));
    }
}
