#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QAbstractTableModel>
#include <QList>
#include <QVariant>

#include "song.h"

class Playlist : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

public:
    Playlist();

    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData* mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

    void append(QString path);
    void clear();

    Song* currentSong();
    Song* at(int index);

    int currentIndex() { return _currentIndex; }
    void setCurrentIndex(int currentIndex);

signals:
    void currentIndexChanged(int from, int to);

private:
    QList<Song*> _songList;

    int _currentIndex = 0;
};

#endif // PLAYLIST_H
