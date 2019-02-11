#include "m3uwriter.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

M3uWriter::M3uWriter(const QString& playlistPath) :
    _playlistPath(playlistPath)
{
}

bool M3uWriter::write(QStringList* paths)
{
    QDir baseDir = QFileInfo(_playlistPath).dir();
    QFile file(_playlistPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        return false;
    }

    QTextStream out(&file);
    for (auto &path : *paths)
    {
        out << baseDir.relativeFilePath(path) << "\n";
    }

    return true;
}
