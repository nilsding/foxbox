#include "statuslabel.h"

StatusLabel::StatusLabel()
{
    QFont font("PxPlus IBM CGA", 8);
    font.setPixelSize(8);
    font.setStyleStrategy(QFont::NoAntialias);
    font.setHintingPreference(QFont::PreferNoHinting);
    font.setStyleHint(QFont::Monospace);
    setFont(font);

    setStyleSheet(
      "background: rgba(0, 0, 0, 255);\n"
      "color: #00ff00;\n"
      "border: 1px inset #333;\n"
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
