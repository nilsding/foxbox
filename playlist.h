#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QAbstractTableModel>
#include <QList>
#include <QVariant>

#include "song.h"

class Playlist : public QAbstractTableModel
{
    Q_OBJECT

public:
    Playlist();

    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void append(QString path);
    void clear();

    Song* at(int index);

private:
    QList<Song*> _songList;
};

#endif // PLAYLIST_H
