#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QMutex>

#include <ao/ao.h>

#include "playlist.h"

class Player : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool playing MEMBER _playing READ playing)

public:
    explicit Player(Playlist* playlist, QObject* parent = nullptr);

    bool playing() { return _playing; }

    static QMutex mutex;

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
    void setVolume(int volume);
    void setLoop(bool loop);

    void onCurrentIndexChanged(int from, int to);

private:
    Playlist* _playlist;
    bool _playing = false;
    bool _loop = false;

    int _ao_driver_id = -1;
    ao_device* _ao_device = nullptr;

    int _row = 0;
    int _pattern = 0;
    int _channels = 0;

    double _volume = 1.0;
};

#endif // PLAYER_H
