#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QThread>

#include "player.h"
#include "playlist.h"
#include "statuslabel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dropEvent(QDropEvent* event);

signals:
    void play();
    void pause();
    void nextTrack();
    void previousTrack();

private slots:
    void on_qaPlayPause_triggered();
    void on_qaNext_triggered();
    void on_qaPrevious_triggered();
    void on_qaLoadPlaylist_triggered();
    void on_qaSavePlaylist_triggered();

    void onSongChange(QString songName);
    void onRowUpdate(int row, int pattern, int channels);
    void onPlaybackStarted();
    void onPlaybackPaused();

private:
    Ui::MainWindow* ui;

    StatusLabel* slInfo;
    Playlist* playlist;
    QThread playbackThread;
    Player* player;

    void loadPlaylistFromFile(const QString& playlistPath);
    void savePlaylistToFile(const QString& playlistPath);
};

#endif // MAINWINDOW_H
