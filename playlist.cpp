#include "playlist.h"

#include <QDebug>
#include <QIcon>


Playlist::Playlist()
{}

int Playlist::columnCount(const QModelIndex&) const
{
    return 2;
}

int Playlist::rowCount(const QModelIndex&) const
{
    return _songList.count();
}

QVariant Playlist::data(const QModelIndex &index, int role) const
{
    if (index.row() > _songList.count())
    {
        return QVariant::Invalid;
    }

    switch (role)
    {
    case Qt::DisplayRole: {
        switch (index.column())
        {
        case 0:
            return _songList.at(index.row())->songName();
        case 1:
            return _songList.at(index.row())->path();
        }
        break;
    }
    case Qt::DecorationRole: {
        if (index.column() == 0) {
            return QIcon(_songList.at(index.row())->valid()
                         ? ":/res/sound.png"
                         : ":/res/unknown.png");
        }
        break;
    }
    }

    return QVariant::Invalid;
}

QVariant Playlist::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant::Invalid;
    }

    if (orientation == Qt::Vertical)
    {
        return QVariant(section + 1);
    }

    switch (section)
    {
    case 0: return QVariant("Name");
    case 1: return QVariant("Path");
    }

    return QVariant::Invalid;
}

void Playlist::append(QString path)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    _songList.append(new Song(path));
    endInsertRows();
}

Song* Playlist::at(int index)
{
    return _songList.at(index);
}
