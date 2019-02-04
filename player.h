#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>

#include <ao/ao.h>

#include "playlist.h"

class Player : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool playing MEMBER _playing READ playing)

public:
    explicit Player(Playlist* playlist, QObject* parent = nullptr);

    bool playing() { return _playing; }

signals:
    void songChange(QString songName);
    void rowUpdate(int row, int pattern, int channels);
    void playbackStarted();
    void playbackPaused();

public slots:
    void play();
    void pause();
    void nextTrack();
    void previousTrack();

private:
    Playlist* _playlist;
    bool _playing;

    int _ao_driver_id;
    ao_device* _ao_device;

    int _row = 0;
    int _pattern = 0;
    int _channels = 0;

    int _currentIndex = 0;
};

#endif // PLAYER_H
