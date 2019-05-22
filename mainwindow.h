#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QThread>
#include <QSlider>
#include <QTimer>

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

public slots:
    void onPlayPressed();
    void onNextPressed();
    void onPreviousPressed();

private slots:
    void on_qaLoadPlaylist_triggered();
    void on_qaSavePlaylist_triggered();
    void on_qaMiniplayer_triggered(bool checked);
    void on_qaAboutFoxbox_triggered();
    void on_qaAboutQt_triggered();
    void on_tableView_doubleClicked(const QModelIndex& index);

    void onSongChange(QString songName);
    void onPlaybackStarted();
    void onPlaybackPaused();
    void updateStatusBar();

private:
    Ui::MainWindow* ui;

    StatusLabel* slInfo;
    QSlider* qsVolume;
    Playlist* playlist;
    QThread playbackThread;
    Player* player;
    QTimer* qtStatusUpdate;

    void loadPlaylistFromFile(const QString& playlistPath);
    void savePlaylistToFile(const QString& playlistPath);
};

#endif // MAINWINDOW_H
