#include "song.h"

#include <string>
#include <fstream>

#include <QDebug>

Song::Song(const QString& path, QObject *parent) : QObject(parent)
{
    _path = path;
    readSongData();
}

void Song::readSongData()
{
    std::ifstream file (_path.toStdString(), std::ios::binary);
    try {
        _mod = new openmpt::module(file);
    } catch (openmpt::exception e) {
        std::cerr << "could not load file because of " << e.what() << std::endl;
        return;
    }
    _valid = true;

    _songName = QString::fromUtf8(_mod->get_metadata("title").c_str());
}
