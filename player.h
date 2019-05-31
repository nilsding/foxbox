#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QMutex>
#include <QTime>

#include <queue>

#include <QAudioOutput>
#include <QIODevice>

#include "playlist.h"

struct TimeInfo
{
    bool valid = false;

    double seconds;
    int order;
    int orderCount;
    int pattern;
    int row;
    int channels;
};

class MptAudioDevice : public QIODevice
{
    Q_OBJECT

    friend class Player;
public:
    MptAudioDevice(Playlist *playlist);

    qint64 readData(char* data, qint64 maxSize);
    qint64 writeData(const char* data, qint64 maxSize);

    TimeInfo currentTimeInfo();

    void fixSongPosition();

signals:
    void songChange(QString songName);

public slots:
    void onCurrentIndexChanged(int from, int to);

private:
    Playlist* _playlist;
    Song* _song = nullptr; //!< current song
    int _currentIndex = 0;

    bool _loop = false;

    QTime _elapsedTime;
    double _timeInfoPosition;
    TimeInfo _currentTimeInfo;
    std::queue<TimeInfo> _timeInfos;

    QMutex _mutex;

    void updateTimeInfos(Song* song, int count);
    void resetTimeInfos(double position = 0.0);
    TimeInfo lookupTimeInfo(double seconds);
    void clearCurrentTimeInfo();
};

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
    void pause();
    void nextTrack();
    void previousTrack();
    void setVolume(int volume);
    void setLoop(bool loop);

private:
    Playlist* _playlist;
    bool _playing = false;

    QAudioOutput* _audioOutput = nullptr;
    MptAudioDevice* _mptDevice = nullptr;

    qreal _volume = 1.0;

};

#endif // PLAYER_H
