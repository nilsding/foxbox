#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QMutex>

#include <QAudioOutput>

#include "playlist.h"
#include "mptaudiodevice.h"

class Player : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool playing MEMBER _playing READ playing)

public:

    explicit Player(Playlist* playlist, QObject* parent = nullptr);

    bool playing() { return _playing; }

    static QMutex mutex;

    TimeInfo currentTimeInfo() const;

signals:
    void songChange(QString songName);
    void playbackStarted();
    void playbackPaused();

public slots:
    void play();
    void pause(bool shouldFixSongPosition);
    void nextTrack();
    void previousTrack();
    void setVolume(int volume);
    void setLoop(bool loop);

private:
    Playlist* _playlist;
    bool _playing = false;
    bool _loop = false;

    QAudioOutput* _audioOutput = nullptr;
    MptAudioDevice* _mptDevice = nullptr;

    qreal _volume = 1.0;
};

#endif // PLAYER_H
