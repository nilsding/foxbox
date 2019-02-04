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
    int16_t buf[BUFFER_SIZE * 2] = { 0x00 };

    ao_sample_format sample_format;
    sample_format.bits = 16;
    sample_format.rate = 24000; // 48000 / 2
    sample_format.channels = 2;
    sample_format.byte_format = AO_FMT_NATIVE;
    sample_format.matrix = nullptr;

    _ao_device = ao_open_live(_ao_driver_id, &sample_format, nullptr);

    auto song = _playlist->at(0);
    emit(songChange(song->songName()));

    _playing = true;
    emit(playbackStarted());
    while (_playing)
    {
        QApplication::processEvents();
        auto read = song->_mod->read_interleaved_stereo(48000, BUFFER_SIZE, buf);
        if (read == 0)
        {
            _playing = false;
            break;
        }

        auto row = song->_mod->get_current_row();
        auto pattern = song->_mod->get_current_pattern();
        auto channels = song->_mod->get_current_playing_channels();
        if (_row != row || _pattern != pattern || _channels != channels)
        {
            emit(
                rowUpdate(
                    row,
                    pattern,
                    channels
                )
            );
            _row = row;
            _pattern = pattern;
            _channels = channels;
        }

        ao_play(_ao_device, (char*)buf, static_cast<uint32_t>(read * 2));
    }
    emit(playbackPaused());

    ao_close(_ao_device);
}

void Player::pause()
{
    _playing = false;
}
