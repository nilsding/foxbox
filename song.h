#ifndef SONG_H
#define SONG_H

#include <QObject>

#include <libopenmpt/libopenmpt.hpp>

class Player;

class Song : public QObject
{
    friend class Player;

    Q_OBJECT
    Q_PROPERTY(QString path     MEMBER _path     READ path)
    Q_PROPERTY(QString songName MEMBER _songName READ songName)
    Q_PROPERTY(bool    valid    MEMBER _valid    READ valid)

public:
    explicit Song(const QString& path, QObject* parent = nullptr);

    QString path() const { return _path; }
    QString songName() const { return _songName; }
    bool valid() { return _valid; }



signals:

public slots:

private:
    QString _path;
    QString _songName;
    bool _valid = false;

    openmpt::module *_mod;

    void readSongData();
};

#endif // SONG_H
