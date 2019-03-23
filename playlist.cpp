#include "playlist.h"

#include <QPalette>
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
    case Qt::BackgroundRole: {
        if (index.row() == _currentIndex)
        {
            QPalette pal;
            return pal.brush(QPalette::Highlight);
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

void Playlist::clear()
{
    auto oldRowCount = rowCount();
    beginRemoveRows(QModelIndex(), 0, oldRowCount);
    for (int i = 0; i < _songList.count(); i++)
    {
        _songList.at(i)->deleteLater();
    }
    _songList.clear();
    endRemoveRows();

    // For some reason this "fixes" the display of the vertical header data...
    emit(headerDataChanged(Qt::Vertical, 0, oldRowCount));
}

Song* Playlist::currentSong()
{
    return at(_currentIndex);
}

Song* Playlist::at(int index)
{
    return _songList.at(index);
}

void Playlist::setCurrentIndex(int currentIndex)
{
    // TODO: boundary check of currentIndex

    if (rowCount() == 0)
    {
        _currentIndex = 0;
        return;
    }

    auto oldIndex = _currentIndex;
    _currentIndex = currentIndex;

    auto topLeft = index(oldIndex, 0);
    auto bottomRight = index(currentIndex, columnCount());
    // horrible(?) workaround to make the dataChanged emit thingy work from
    // another thread:
    qRegisterMetaType<QVector<int> >("QVector<int>");
    emit(dataChanged(topLeft, bottomRight, { Qt::BackgroundRole }));

    emit(currentIndexChanged(oldIndex, _currentIndex));
}
