#include "playlist.h"

#include <QPalette>
#include <QIcon>
#include <QMimeData>
#include <QTableView>
#include <algorithm>

#define DRAG_DROP_MIME_TYPE "application/x-foxboxplaylistindexnumbers"

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
    case Qt::ForegroundRole: {
        if (index.row() == _currentIndex)
        {
            QPalette pal;
            return pal.brush(QPalette::BrightText);
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

Qt::ItemFlags Playlist::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if (index.isValid())
    {
        return Qt::ItemIsDragEnabled | defaultFlags;
    }
    return Qt::ItemIsDropEnabled | defaultFlags;
}

Qt::DropActions Playlist::supportedDropActions() const
{
    return Qt::MoveAction;
}

QStringList Playlist::mimeTypes() const
{
    return QAbstractItemModel::mimeTypes() << DRAG_DROP_MIME_TYPE;
}

QMimeData* Playlist::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (auto &index : indexes)
    {
        if (!(index.isValid() && index.column() == 0))
        {
            continue;
        }
        stream << index.row();
    }

    mimeData->setData(DRAG_DROP_MIME_TYPE, encodedData);

    return mimeData;
}

bool Playlist::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (!canDropMimeData(data, action, row, column, parent))
    {
        return false;
    }

    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    int beginRow;

    if (row != -1)
    {
        beginRow = row;
    }
    else
    {
        beginRow = rowCount();
    }

    QByteArray encodedData = data->data(DRAG_DROP_MIME_TYPE);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<int> sourceRows;
    int rows = 0;

    while (!stream.atEnd())
    {
        int rowNumber;
        stream >> rowNumber;
        sourceRows << rowNumber;
        ++rows;
    }

    // all of this that follows is probably just some darn workaround/hack
    // around something that Qt provides but I'm too lazy to implement for now.
    // works good enough for me, so ehhh ¯\_(ツ)_/¯

    // sort the row numbers
    std::sort(sourceRows.begin(), sourceRows.end(),
              [](const int a, const int b) { return a < b; });

    // get the song objects for the row numbers
    QList<Song*> selectedSongs;
    for (auto sourceRow : sourceRows)
    {
        selectedSongs << _songList.at(sourceRow);

        // edge cases: beginRow is set to sourceRow, which means that we want to
        // insert the songs after it.  increment it
        if (beginRow == sourceRow)
        {
            beginRow++;
        }
    }

    // make sure beginRow (drop target row) is not out of bounds
    // this means that the dropped songs should come after the last song in the
    // playlist
    bool shouldAppend = false;
    if (beginRow >= _songList.count())
    {
        beginRow = _songList.count() - 1;
        shouldAppend = true;
    }

    // get the song object for the target row
    Song* targetSong = _songList.at(beginRow);
    // get the current playing song object
    Song* currentSongObj = currentSong();

    // remove all songs from their previous positions from back to front
    for (auto it = sourceRows.rbegin(); it != sourceRows.rend(); it++)
    {
        _songList.removeAt(*it);
    };

    // figure out the new target row
    // this is perfectly fine, since we create a new Song object is for each
    // loaded file.  we are comparing pointers here, not file paths!
    int newSourceRow = _songList.indexOf(targetSong);
    if (shouldAppend)
    {
        newSourceRow++;
    }

    // insert the new songs at their desired position
    for (auto it = selectedSongs.rbegin(); it != selectedSongs.rend(); it++)
    {
        _songList.insert(newSourceRow, *it);
    }

    // and select them ;-)
    auto tview = qobject_cast<QTableView*>(this->parent());
    tview->selectionModel()->clearSelection();
    auto topLeft = index(newSourceRow, 0);
    auto bottomRight = index(newSourceRow + selectedSongs.count() - 1, columnCount() - 1);
    auto selection = QItemSelection(topLeft, bottomRight);
    tview->selectionModel()->select(selection, QItemSelectionModel::Select);

    // finally, reassign the current song index to the song that's actually
    // playing
    _currentIndex = _songList.indexOf(currentSongObj);

    return true;
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
