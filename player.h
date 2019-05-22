#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QTime>

#include <QAudioOutput>

#include "playlist.h"

class Player : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool playing MEMBER _playing READ playing)

public:
    struct TimeInfo
    {
        double seconds;
        int pattern;
        int row;
        int channels;
    };

    explicit Player(Playlist* playlist, QObject* parent = nullptr);

    bool playing() { return _playing; }

    static QMutex mutex;

    TimeInfo currentTimeInfo() const { return _currentTimeInfo; }

signals:
    void songChange(QString songName);
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

    QAudioOutput* _audioOutput = nullptr;

    QTime _elapsedTime;
    double _timeInfoPosition;
    TimeInfo _currentTimeInfo;
    QQueue<TimeInfo> _timeInfos;

    qreal _volume = 1.0;

    void updateTimeInfos(Song* song, int count);
    void resetTimeInfos(double position = 0.0);
    TimeInfo lookupTimeInfo(double seconds);
    void clearCurrentTimeInfo();
};

#endif // PLAYER_H
