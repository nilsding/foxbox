#include "player.h"

#include <QApplication>

Player::Player(Playlist* playlist, QObject *parent) :
    QObject(parent),
    _playlist(playlist)
{
    _ao_driver_id = ao_default_driver_id();
}

#define BUFFER_SIZE 2

void Player::play()
{
    auto currentIndex = _currentIndex;
    int16_t buf[BUFFER_SIZE * 2] = { 0x00 };

    if (_playlist->rowCount() == 0)
    {
        return;
    }

    auto song = _playlist->at(currentIndex);
    song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
    emit(songChange(song->songName()));

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
        if (currentIndex != _currentIndex)
        {
            song->_mod->set_position_order_row(0, 0);
            song->_mod->ctl_set("play.at_end", "stop");
            song = _playlist->at(_currentIndex);
            song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
            currentIndex = _currentIndex;
            emit(songChange(song->songName()));
        }

        auto read = song->_mod->read_interleaved_stereo(48000, BUFFER_SIZE, buf);
        if (read == 0)
        {
            // when module ctl `play.at_end` is set to continue, at the end of the
            // song 0 is returned.  however subsequent reads will start playing at
            // the module's loop point again.  therefore: redo this iteration.
            if (_loop)
            {
                continue;
            }

            song->_mod->set_position_order_row(0, 0);
            song->_mod->ctl_set("play.at_end", "stop");
            if (_playlist->rowCount() > _currentIndex + 1)
            {
                song = _playlist->at(++_currentIndex);
                currentIndex = _currentIndex;
                song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
                emit(songChange(song->songName()));
                continue;
            }
            _playing = false;
            break;
        }

        auto row = song->_mod->get_current_row();
        auto pattern = song->_mod->get_current_pattern();
        auto channels = song->_mod->get_current_playing_channels();
        if (_row != row || _pattern != pattern || _channels != channels)
        {
            emit(rowUpdate(row, pattern, channels));
            _row = row;
            _pattern = pattern;
            _channels = channels;
        }

        ao_play(_ao_device, (char*)buf, static_cast<uint32_t>(read * 2));
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

    if (_playlist->rowCount() > _currentIndex + 1)
    {
        if (!_playing)
        {
            auto song = _playlist->at(_currentIndex);
            song->_mod->set_position_order_row(0, 0);
            song->_mod->ctl_set("play.at_end", "stop");
        }
        _currentIndex++;
    }
}

void Player::previousTrack()
{
    if (_playlist->rowCount() == 0)
    {
        return;
    }

    auto song = _playlist->at(_currentIndex);

    // rewind the song if it has been played for longer than 1 second
    if (_playing && song->_mod->get_position_seconds() > 1.0)
    {
        song->_mod->set_position_order_row(0, 0);
        return;
    }

    if (_currentIndex - 1 >= 0)
    {
        if (!_playing)
        {
            song->_mod->set_position_order_row(0, 0);
            song->_mod->ctl_set("play.at_end", "stop");
        }
        _currentIndex--;
    }
}

void Player::setLoop(bool loop)
{
    _loop = loop;
    if (_playing)
    {
        auto song = _playlist->at(_currentIndex);
        song->_mod->ctl_set("play.at_end", _loop ? "continue" : "stop");
    }
}
