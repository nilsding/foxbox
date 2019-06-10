#ifndef MPTAUDIODEVICE_H
#define MPTAUDIODEVICE_H

#include <QIODevice>
#include <QMutex>
#include <QTime>

#include <queue>

#include "playlist.h"

#ifdef Q_OS_MAC
// For some reason QtMultimedia on macOS does not want to deal with float sample
// types...
#define SAMPLE_CORE_TYPE int16_t
#define SAMPLE_TYPE QAudioFormat::SignedInt
#else
#define SAMPLE_CORE_TYPE float
#define SAMPLE_TYPE QAudioFormat::Float
#endif

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

class Player;

class MptAudioDevice : public QIODevice
{
    Q_OBJECT
    
    friend class Player;

public:
    MptAudioDevice(Playlist *playlist);

    TimeInfo currentTimeInfo();

    void fixSongPosition();
    
signals:
    void songChange(QString songName);
    void lastSongEndedChanged(bool playing);

public slots:
    void onCurrentIndexChanged(int from, int to);

protected:
    qint64 writeData(const char* data, qint64 size);
    qint64 readData(char* data, qint64 maxSize);
    
    void setLastSongEnded(bool lastSongEnded);

private:
    Playlist* _playlist = nullptr;
    Song* _song = nullptr; //!< current song
    int _currentIndex = 0;

    bool _loop = false;
    bool _lastSongEnded = false;

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

#endif // MPTAUDIODEVICE_H
