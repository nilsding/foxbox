#include "player.h"

#include <QApplication>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QIODevice>

#include <QDebug>

QMutex Player::mutex;

Player::Player(Playlist* playlist, QObject *parent) :
    QObject(parent),
    _playlist(playlist)
{}

TimeInfo Player::currentTimeInfo() const
{
    if (_mptDevice == nullptr)
    {
        return TimeInfo();
    }
    return _mptDevice->currentTimeInfo();
}

void Player::play()
{
    auto currentIndex = _playlist->currentIndex();

    if (_playlist->rowCount() == 0)
    {
        return;
    }

    auto song = _playlist->at(currentIndex);
    emit(songChange(song->songName()));

    QAudioFormat sampleFormat;
    sampleFormat.setSampleSize(16);
    sampleFormat.setSampleRate(48000);
    sampleFormat.setChannelCount(2);
    sampleFormat.setCodec("audio/pcm");
    sampleFormat.setByteOrder(QAudioFormat::LittleEndian);
    sampleFormat.setSampleType(SAMPLE_TYPE);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(sampleFormat))
    {
      qWarning() << "Selected audio format is not supported by the default output device, using nearest supported format.";
      sampleFormat = info.nearestFormat(sampleFormat);
    }

    if (_audioOutput == nullptr)
    {
      _audioOutput = new QAudioOutput(sampleFormat, nullptr);
    }

    _audioOutput->setCategory("music");
    _audioOutput->setVolume(_volume);

    if (_mptDevice == nullptr)
    {
        _mptDevice = new MptAudioDevice(_playlist);
        _mptDevice->_loop = _loop;
        _mptDevice->open(QIODevice::ReadOnly);
        connect(_mptDevice, &MptAudioDevice::songChange, [&](QString songName) {
            emit(songChange(songName));
        });
        connect(_mptDevice, &MptAudioDevice::lastSongEndedChanged, [&](bool lastSongEnded) {
            pause(!lastSongEnded);
        });
    }
    _audioOutput->start(_mptDevice);

    _playing = true;
    emit(playbackStarted());
    return;
}

void Player::pause(bool shouldFixSongPosition)
{
    emit(playbackPaused());
    _audioOutput->stop();
    _audioOutput->deleteLater();
    _audioOutput = nullptr;
    _playing = false;
    if (shouldFixSongPosition)
    {
        _mptDevice->fixSongPosition();
    }
    _mptDevice->deleteLater();
    _mptDevice = nullptr;
}

void Player::nextTrack()
{
    if (_playlist->rowCount() == 0)
    {
        return;
    }

    if (_playlist->rowCount() > _playlist->currentIndex() + 1)
    {
        if (!_playing)
        {
            auto song = _playlist->currentSong();
            song->_mod->set_position_order_row(0, 0);
            song->_mod->ctl_set("play.at_end", "stop");
        }
        _playlist->setCurrentIndex(_playlist->currentIndex() + 1);
    }
}

void Player::previousTrack()
{
    if (_playlist->rowCount() == 0)
    {
        return;
    }

    auto song = _playlist->currentSong();

    // rewind the song if it has been played for more than 3 seconds
    if (_playing && song->_mod->get_position_seconds() > 3.0)
    {
        song->_mod->set_position_order_row(0, 0);
        return;
    }

    if (_playlist->currentIndex() - 1 >= 0)
    {
        if (!_playing)
        {
            song->_mod->set_position_order_row(0, 0);
            song->_mod->ctl_set("play.at_end", "stop");
        }
        _playlist->setCurrentIndex(_playlist->currentIndex() - 1);
    }
}

void Player::setVolume(int volume)
{
    if (_audioOutput == nullptr)
    {
        return;
    }

    _volume = QAudio::convertVolume(volume / qreal(100.0),
                                    QAudio::LogarithmicVolumeScale,
                                    QAudio::LinearVolumeScale);
    _audioOutput->setVolume(_volume);
}

void Player::setLoop(bool loop)
{
    _loop = loop;

    if (_mptDevice == nullptr)
    {
        return;
    }

    auto song = _playlist->currentSong();
    song->_mod->ctl_set("play.at_end", loop ? "continue" : "stop");
    _mptDevice->_loop = loop;
}
