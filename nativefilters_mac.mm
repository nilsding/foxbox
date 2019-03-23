#include "nativefilters_mac.h"
#include <QDebug>

#import <AppKit/AppKit.h>
#import <IOKit/hidsystem/ev_keymap.h>

NativeFilter::NativeFilter(QObject* parent) : QObject(parent)
{}

bool NativeFilter::nativeEventFilter(const QByteArray& eventType, void* message, long*)
{
    if (eventType != "mac_generic_NSEvent")
    {
        return false;
    }

    NSEvent *event = static_cast<NSEvent *>(message);
    if ([event type] == NSEventTypeSystemDefined && [event subtype] == 8)
    {
        int keyCode = (([event data1] & 0xFFFF0000) >> 16);
        int keyFlags = ([event data1] & 0x0000FFFF);
        bool keyState = (((keyFlags & 0xFF00) >> 8)) == 0xA;
        bool keyRepeat = (keyFlags & 0x1);

        emitMediaKeyEvent(keyCode, keyState, keyRepeat);
    }

    return false;
}

void NativeFilter::emitMediaKeyEvent(int keyCode, bool pressed, bool /* repeat */)
{
    // only care about released keys
    if (pressed)
    {
        return;
    }

    switch (keyCode)
    {
    case NX_KEYTYPE_PLAY:
        emit(mediaKeyDownPlay());
        break;
    case NX_KEYTYPE_NEXT:
        emit(mediaKeyDownNext());
        break;
    case NX_KEYTYPE_PREVIOUS:
        emit(mediaKeyDownPrevious());
        break;
    }
}
