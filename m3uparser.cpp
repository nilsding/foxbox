#include "m3uparser.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

M3uParser::M3uParser(const QString& playlistPath) :
    _playlistPath(playlistPath)
{
}

QStringList* M3uParser::parse()
{
    QStringList* paths = new QStringList;

    QDir baseDir = QFileInfo(_playlistPath).dir();

    QFile file(_playlistPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return nullptr;
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();

        // Drop lines from extended M3U files
        if (line.startsWith("#EXTM3U") || line.startsWith("#EXTINF"))
        {
            continue;
        }

        // Unix only (for now): add absolute paths to the path list, and try to
        // resolve relative paths in the playlist file to absolute paths.  Those
        // are then also added to the path list, yay!
        //
        // Patches for Windows welcome.
#ifdef Q_OS_WIN
#error "Unfortunately, I have not implemented support for relative paths in playlists for the Windows platform.  Patches welcome!"
#endif
        if (line.startsWith("/"))
        {
            if (QFile::exists(line))
            {
                paths->append(line);
            }
            continue;
        }

        auto absolutePath = baseDir.filePath(line);
        if (QFile::exists(absolutePath))
        {
            paths->append(absolutePath);
        }
    }

    return paths;
}
