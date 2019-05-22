#include "player.h"

#include <QApplication>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QByteArray>
#include <QDebug>
#include <QIODevice>
#include <QThread>

QMutex Player::mutex;

Player::Player(Playlist* playlist, QObject *parent) :
    QObject(parent),
    _playlist(playlist)
{
    connect(playlist, &Playlist::currentIndexChanged, this, &Player::onCurrentIndexChanged);
}

#define FRAME_SIZE 2
#ifdef Q_OS_MAC
// For some reason I am too lazy to debug now either libopenmpt or QtMultimedia
// on macOS does not want to deal with float sample types ...
#define SAMPLE_CORE_TYPE int16_t
#define SAMPLE_TYPE QAudioFormat::SignedInt
#else
#define SAMPLE_CORE_TYPE float
#define SAMPLE_TYPE QAudioFormat::Float
#endif

void Player::play()
{
    auto currentIndex = _playlist->currentIndex();
    QByteArray buf(FRAME_SIZE * sizeof(SAMPLE_CORE_TYPE) * 2, 0x00); // frame size * byte size * 2 (channels)

    if (_playlist->rowCount() == 0)
    {
        return;
    }

    auto song = _playlist->at(currentIndex);
    song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
    _elapsedTime.start();
    clearCurrentTimeInfo();
    resetTimeInfos();
    updateTimeInfos(song, 0);
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

    QIODevice *output = _audioOutput->start();

    _playing = true;
    emit(playbackStarted());
    while (_playing)
    {
        QApplication::processEvents();
        mutex.lock();
        if (currentIndex != _playlist->currentIndex())
        {
            // only change the actual song if the change was caused by moving
            // the song around in the playlist
            if (song != _playlist->currentSong())
            {
                song->_mod->set_position_order_row(0, 0);
                song->_mod->ctl_set("play.at_end", "stop");
                song = _playlist->currentSong();
                song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
                clearCurrentTimeInfo();
                resetTimeInfos();
                updateTimeInfos(song, 0);
                emit(songChange(song->songName()));
            }

            currentIndex = _playlist->currentIndex();
        }

        auto read = song->_mod->read_interleaved_stereo(48000, FRAME_SIZE, reinterpret_cast<SAMPLE_CORE_TYPE*>(buf.data()));
        if (read == 0)
        {
            // when module ctl `play.at_end` is set to continue, at the end of the
            // song 0 is returned.  however subsequent reads will start playing at
            // the module's loop point again.  therefore: redo this iteration.
            if (_loop)
            {
                mutex.unlock();
                continue;
            }

            song->_mod->set_position_order_row(0, 0);
            song->_mod->ctl_set("play.at_end", "stop");
            resetTimeInfos();
            updateTimeInfos(song, 0);
            if (_playlist->rowCount() > _playlist->currentIndex() + 1)
            {
                _playlist->setCurrentIndex(++currentIndex);
                song = _playlist->currentSong();
                song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
                clearCurrentTimeInfo();
                resetTimeInfos();
                updateTimeInfos(song, 0);
                emit(songChange(song->songName()));
                mutex.unlock();
                continue;
            }
            _playing = false;
            mutex.unlock();
            break;
        }
        updateTimeInfos(song, FRAME_SIZE);
        // lookupTimeInfo(song->_mod->get_position_seconds());
        lookupTimeInfo(_elapsedTime.elapsed() / 1000.0);

        output->write(buf, static_cast<qint64>(read * sizeof(SAMPLE_CORE_TYPE) * 2));
        // wait until audio output needs more data
        while (_audioOutput->bytesFree() < _audioOutput->periodSize())
        {
            //lookupTimeInfo(song->_mod->get_position_seconds());
            lookupTimeInfo(_elapsedTime.elapsed() / 1000.0);
            // Do not eat up all CPU time while waiting
            QThread::msleep(15);
        };
        mutex.unlock();
    }
    song->_mod->ctl_set("play.at_end", "stop");
    emit(playbackPaused());

    _audioOutput->stop();
    _audioOutput->deleteLater();
    _audioOutput = nullptr;
}

void Player::pause()
{
    _playing = false;
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
                                    QAudio::LinearVolumeScale,
                                    QAudio::LogarithmicVolumeScale);
    _audioOutput->setVolume(_volume);
}

void Player::setLoop(bool loop)
{
    _loop = loop;
    if (_playing)
    {
        auto song = _playlist->currentSong();
        song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
    }
}

void Player::onCurrentIndexChanged(int from, int /* to */)
{
    if (!_playing)
    {
        auto song = _playlist->at(from);
        song->_mod->set_position_order_row(0, 0);
        song->_mod->ctl_set("play.at_end", "stop");
    }

    auto song = _playlist->currentSong();
    clearCurrentTimeInfo();
    resetTimeInfos();
    updateTimeInfos(song, 0);
    emit(songChange(song->songName()));
}

void Player::updateTimeInfos(Song *song, int count)
{
    _timeInfoPosition += count / 48000.0;

    TimeInfo info;
    info.seconds = _timeInfoPosition;
    info.pattern = song->_mod->get_current_pattern();
    info.row = song->_mod->get_current_row();
    info.channels = song->_mod->get_current_playing_channels();

    _timeInfos.enqueue(info);
}

void Player::resetTimeInfos(double position)
{
    _timeInfos.clear();
    _timeInfoPosition = position;
}

Player::TimeInfo Player::lookupTimeInfo(double seconds)
{
    TimeInfo info = _currentTimeInfo;

    if (_timeInfos.empty())
    {
        // try to recover the timeinfo
        clearCurrentTimeInfo();
        resetTimeInfos();
        updateTimeInfos(_playlist->currentSong(), 0);
        info = _currentTimeInfo;
    }

    while (_timeInfos.size() > 0 && _timeInfos.front().seconds <= seconds)
    {
        info = _timeInfos.dequeue();
    }

    _currentTimeInfo = info;
    return _currentTimeInfo;
}

void Player::clearCurrentTimeInfo()
{
    _currentTimeInfo = TimeInfo();
    _elapsedTime.restart();
}
