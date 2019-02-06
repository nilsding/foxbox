#ifndef VIS_H
#define VIS_H

#include <QWidget>

#include <QLabel>
#include <QList>
#include <QProgressBar>

#include "song.h"

class VisRow : public QWidget
{
    Q_OBJECT
public:
    explicit VisRow(QWidget *parent = nullptr);
    ~VisRow();

signals:

public slots:
    void setVolume(float value);
    void setPatternText(const QString& text);

private:
    QProgressBar* qpbVolume;
    QLabel* qlPattern;
};

class VisWindow : public QWidget
{
    Q_OBJECT
public:
    explicit VisWindow(QWidget *parent = nullptr);
    ~VisWindow();

    void setRows(int count);
    VisRow* at(int index);

signals:

public slots:
    void onFrameUpdate(Song* song);

private:
    QList<VisRow*> _rows;
};

#endif // VIS_H
