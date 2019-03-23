#include "player.h"

#include <QApplication>
#include <cmath>

QMutex Player::mutex;

Player::Player(Playlist* playlist, QObject *parent) :
    QObject(parent),
    _playlist(playlist)
{
    _ao_driver_id = ao_default_driver_id();
    connect(playlist, &Playlist::currentIndexChanged, this, &Player::onCurrentIndexChanged);
}

#define BUFFER_SIZE 2

void Player::play()
{
    auto currentIndex = _playlist->currentIndex();
    int16_t buf[BUFFER_SIZE * 2] = { 0x00 };

    if (_playlist->rowCount() == 0)
    {
        return;
    }

    auto song = _playlist->at(currentIndex);
    song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
    emit(songChange(song->songName(), song->_mod->get_num_channels()));

    ao_sample_format sample_format;
    sample_format.bits = 16;
    sample_format.rate = 24000; // 48000 / 2
    sample_format.channels = 2;
    sample_format.byte_format = AO_FMT_NATIVE;
    sample_format.matrix = nullptr;

    _ao_device = ao_open_live(_ao_driver_id, &sample_format, nullptr);

    _playing = true;
    emit(playbackStarted());
    while (_playing)
    {
        QApplication::processEvents();
        mutex.lock();
        if (currentIndex != _playlist->currentIndex())
        {
            song->_mod->set_position_order_row(0, 0);
            song->_mod->ctl_set("play.at_end", "stop");
            song = _playlist->currentSong();
            song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
            currentIndex = _playlist->currentIndex();
            emit(songChange(song->songName(), song->_mod->get_num_channels()));
        }

        auto read = song->_mod->read_interleaved_stereo(48000, BUFFER_SIZE, buf);
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
            if (_playlist->rowCount() > _playlist->currentIndex() + 1)
            {
                _playlist->setCurrentIndex(++currentIndex);
                song = _playlist->currentSong();
                song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
                emit(songChange(song->songName(), song->_mod->get_num_channels()));
                mutex.unlock();
                continue;
            }
            _playing = false;
            mutex.unlock();
            break;
        }
        emit(frameUpdate(song));

        auto row = song->_mod->get_current_row();
        auto pattern = song->_mod->get_current_pattern();
        if (_row != row || _pattern != pattern)
        {
            auto channels = song->_mod->get_current_playing_channels();
            emit(rowUpdate(row, pattern, channels));
            _row = row;
            _pattern = pattern;
        }

        if (_volume < 1.0)
        {
            for (int i = 0; i < BUFFER_SIZE * 2; i++)
            {
                buf[i] = static_cast<int16_t>(buf[i] * _volume);
            }
        }

        ao_play(_ao_device, reinterpret_cast<char*>(buf), static_cast<uint32_t>(read * 2));
        mutex.unlock();
    }
    song->_mod->ctl_set("play.at_end", "stop");
    emit(playbackPaused());

    ao_close(_ao_device);
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
    _volume = std::pow(volume / 100.0, M_E);
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
    emit(songChange(song->songName(), song->_mod->get_num_channels()));
}
