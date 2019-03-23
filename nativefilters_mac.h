#ifndef NATIVEFILTERS_MAC_H
#define NATIVEFILTERS_MAC_H

#include <QObject>
#include <QAbstractNativeEventFilter>

class NativeFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit NativeFilter(QObject* parent = nullptr);
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override;

signals:
    void mediaKeyDownPlay();
    void mediaKeyDownNext();
    void mediaKeyDownPrevious();

private:
    void emitMediaKeyEvent(int keyCode, bool keyState, bool repeat);
};

#endif // NATIVEFILTERS_MAC_H
