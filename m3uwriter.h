#ifndef M3UWRITER_H
#define M3UWRITER_H

#include <QString>
#include <QStringList>

class M3uWriter
{
public:
    M3uWriter(const QString& playlistPath);

    bool write(QStringList* paths);

private:
    const QString& _playlistPath;
};

#endif // M3UWRITER_H
