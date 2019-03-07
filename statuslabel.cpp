#include "statuslabel.h"

StatusLabel::StatusLabel()
{
    QFont font("Monaco");
    font.setStyleHint(QFont::Monospace);
    setFont(font);

    setStyleSheet(
      "background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(72, 72, 72, 255), stop:1 rgba(0, 0, 0, 255));"
      "border: 2px solid qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(77, 77, 77, 255), stop:1 rgba(169, 169, 169, 255)) inset;"
      "border-radius: 5px;"
      "color: #ffff00;"
      "height: 2.2em;"
      "margin: 12px 0px;"
      "padding-left: .1em;"
      "padding-right: .1em;"
      "padding-top: -.5em;"
      "padding-bottom: .25em;"
#if 0

      "background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(72, 72, 72, 255), stop:1 rgba(0, 0, 0, 255));\n"
      "color: #ffff00;\n"
      "padding-left: .1em;\n"
      "padding-right: .1em;\n"
      "padding-top: .4em;\n"
      "padding-bottom: .4em;\n"
      "margin-top: 0em;\n"
      "margin-bottom: 0em;"
#endif
    );

    auto sp = sizePolicy();
    sp.setHorizontalStretch(1);
    setSizePolicy(sp);
}

void StatusLabel::updateText()
{
    setText(QString("%1\n%2").arg(_firstLine, _secondLine));
}
