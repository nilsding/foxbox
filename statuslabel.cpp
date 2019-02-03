#include "statuslabel.h"

StatusLabel::StatusLabel()
{
    QFont font("Monaco");
    font.setStyleHint(QFont::Monospace);
    setFont(font);

    setStyleSheet(
      "background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(72, 72, 72, 255), stop:1 rgba(0, 0, 0, 255));\n"
      "color: #ffff00;\n"
      "padding-left: .1em;\n"
      "padding-right: .1em;"
    );

    auto sp = sizePolicy();
    sp.setHorizontalStretch(1);
    setSizePolicy(sp);
}

void StatusLabel::updateText()
{
    setText(QString("%1\n%2").arg(_firstLine, _secondLine));
}
