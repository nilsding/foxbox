#include "player.h"

#include <QApplication>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QByteArray>
#include <QDebug>
#include <QIODevice>
#include <QThread>

#ifdef Q_OS_MAC
// For some reason QtMultimedia on macOS does not want to deal with float sample
// types...
#define SAMPLE_CORE_TYPE int16_t
#define SAMPLE_TYPE QAudioFormat::SignedInt
#else
#define SAMPLE_CORE_TYPE float
#define SAMPLE_TYPE QAudioFormat::Float
#endif

QMutex Player::mutex;

MptAudioDevice::MptAudioDevice(Playlist* playlist) :
    _playlist(playlist)
{
    connect(playlist, &Playlist::currentIndexChanged, this, &MptAudioDevice::onCurrentIndexChanged);
}

qint64 MptAudioDevice::readData(char* data, qint64 maxSize)
{
    auto multiplier = sizeof(SAMPLE_CORE_TYPE) * 2;
    std::size_t framesToRender = maxSize / multiplier;
    if (framesToRender == 0)
    {
        return 0;
    }

    if (_song == nullptr)
    {
        _song = _playlist->currentSong();
        _song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
        // fresh song started -- we'll start a new timeinfo clock
        _elapsedTime.start();
        clearCurrentTimeInfo();
        resetTimeInfos();
        updateTimeInfos(_song, 0);
    }

    if (_currentIndex != _playlist->currentIndex())
    {
        // only change the actual song if the change was caused by moving
        // the song around in the playlist
        if (_song != _playlist->currentSong())
        {
            std::cerr << "song != playlist->currentSong" << std::endl;
            _song->_mod->set_position_order_row(0, 0);
            _song->_mod->ctl_set("play.at_end", "stop");
            _song = _playlist->currentSong();
            _song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
            clearCurrentTimeInfo();
            resetTimeInfos();
            updateTimeInfos(_song, 0);
            emit(songChange(_song->songName()));
        }

        _currentIndex = _playlist->currentIndex();
    }

    std::size_t frameSize = 0;
    qint64 framesRendered = 0;
    while (framesToRender > 0)
    {
        frameSize = std::min<std::size_t>(framesToRender, 480); // 480 = 48000/100 -> 100Hz timeinfo update
        frameSize = _song->_mod->read_interleaved_stereo(48000, frameSize, reinterpret_cast<SAMPLE_CORE_TYPE*>(data));

        updateTimeInfos(_song, frameSize);

        data += frameSize * multiplier;
        framesRendered += frameSize;
        framesToRender -= frameSize;

        if (frameSize == 0)
        {
            // when module ctl `play.at_end` is set to continue, at the end of the
            // song 0 is returned.  however subsequent reads will start playing at
            // the module's loop point again.  therefore: return 0 bytes read, and
            // read again
            if (_loop)
            {
                return 0;
            }

            _song->_mod->set_position_order_row(0, 0);
            _song->_mod->ctl_set("play.at_end", "stop");
            // resetTimeInfos();
            // updateTimeInfos(_song, 0);

            // we have a next song available?  perfect, we'll just return 0 bytes
            // read and prepare the next song for the next call to readData
            if (_playlist->rowCount() > _playlist->currentIndex() + 1)
            {
                _playlist->setCurrentIndex(++_currentIndex);
                _song = _playlist->currentSong();
                _song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
                clearCurrentTimeInfo();
                // resetTimeInfos();
                // updateTimeInfos(_song, 0);
                emit(songChange(_song->songName()));
                return 0;
            }

            // end of playlist?
            return -1;
        }

    }

    return framesRendered * multiplier;
}

qint64 MptAudioDevice::writeData(const char* data, qint64 maxSize)
{
    return -1;
}

TimeInfo MptAudioDevice::currentTimeInfo()
{
    return lookupTimeInfo(_elapsedTime.elapsed() / 1000.0);
}

void MptAudioDevice::fixSongPosition()
{
    auto timeInfo = currentTimeInfo();
    _song->_mod->set_position_order_row(timeInfo.order, timeInfo.row);
}

void MptAudioDevice::onCurrentIndexChanged(int from, int /* to */)
{
    // XXX: how the fuck do we know if we are playing?  why would that even
    // matter?
    //if (!_playing)
    //{
        auto song = _playlist->at(from);
        song->_mod->set_position_order_row(0, 0);
        song->_mod->ctl_set("play.at_end", "stop");
    //}

    _song = _playlist->currentSong();
    clearCurrentTimeInfo();
    resetTimeInfos();
    updateTimeInfos(_song, 0);
    emit(songChange(_song->songName()));
}

void MptAudioDevice::updateTimeInfos(Song *song, int count)
{
    _timeInfoPosition += count / 48000.0;

    TimeInfo info;
    info.valid = true;
    info.seconds = _timeInfoPosition;
    info.order = song->_mod->get_current_order();
    info.orderCount = song->_mod->get_num_orders() - 1;
    info.pattern = song->_mod->get_current_pattern();
    info.row = song->_mod->get_current_row();
    info.channels = song->_mod->get_current_playing_channels();

    _mutex.tryLock();
    _timeInfos.push(info);
    _mutex.unlock();
}

void MptAudioDevice::resetTimeInfos(double position)
{
    _mutex.tryLock();
    while (!_timeInfos.empty())
    {
        _timeInfos.pop();
    }
    _mutex.unlock();
    _timeInfoPosition = position;
}

TimeInfo MptAudioDevice::lookupTimeInfo(double seconds)
{
    TimeInfo info = _currentTimeInfo;

    _mutex.tryLock();
    if (_timeInfos.empty())
    {
        // try to recover the timeinfo
        clearCurrentTimeInfo();
        _mutex.unlock();
        resetTimeInfos();
        updateTimeInfos(_playlist->currentSong(), 0);
        _mutex.tryLock();
        info = _currentTimeInfo;
    }

    while (_timeInfos.size() > 0 && _timeInfos.front().seconds <= seconds)
    {
        info = _timeInfos.front();
        _timeInfos.pop();
    }
    _mutex.unlock();

    _currentTimeInfo = info;
    return _currentTimeInfo;
}

void MptAudioDevice::clearCurrentTimeInfo()
{
    _currentTimeInfo = TimeInfo();
    _elapsedTime.restart();
}

// ============================================================================

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
        _mptDevice->open(QIODevice::ReadOnly);
        connect(_mptDevice, &MptAudioDevice::songChange, [&](QString songName) {
            emit(songChange(songName));
        });
    }
    _audioOutput->start(_mptDevice);

    _playing = true;
    emit(playbackStarted());
    return;
}

void Player::pause()
{
    _audioOutput->stop();
    _audioOutput->deleteLater();
    _audioOutput = nullptr;
    _playing = false;
    _mptDevice->fixSongPosition();
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
    if (_mptDevice == nullptr)
    {
        return;
    }

    auto song = _playlist->currentSong();
    song->_mod->ctl_set("play.at_end", loop ? "continue" : "stop");
    _mptDevice->_loop = loop;
}
