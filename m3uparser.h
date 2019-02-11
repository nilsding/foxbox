#ifndef M3UPARSER_H
#define M3UPARSER_H

#include <QString>
#include <QStringList>

class M3uParser
{
public:
    M3uParser(const QString& playlistPath);

    QStringList* parse();

private:
    const QString& _playlistPath;
};

#endif // M3UPARSER_H
