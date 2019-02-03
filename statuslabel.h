#ifndef STATUSLABEL_H
#define STATUSLABEL_H

#include <QLabel>

class StatusLabel : public QLabel
{
    Q_OBJECT

public:
    StatusLabel();

    QString firstLine()  const { return _firstLine; }
    QString secondLine() const { return _secondLine; }
    void setFirstLine(QString firstLine)   { _firstLine = firstLine;   updateText(); }
    void setSecondLine(QString secondLine) { _secondLine = secondLine; updateText(); }

private:
    QString _firstLine = "";
    QString _secondLine = "";

    void updateText();
};

#endif // STATUSLABEL_H
