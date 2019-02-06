#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QIcon>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    slInfo(new StatusLabel),
    playlist(new Playlist),
    player(new Player(playlist))
{
    setAcceptDrops(true);
    ui->setupUi(this);

    slInfo->setFirstLine("foxbox 0.1.0");
    slInfo->setSecondLine("Ready");
    ui->toolBar->insertWidget(ui->qaLoop, slInfo);

    ui->tableView->setModel(playlist);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    player->moveToThread(&playbackThread);
    connect(this, &MainWindow::play, player, &Player::play);
    connect(this, &MainWindow::pause, player, &Player::pause);
    connect(this, &MainWindow::nextTrack, player, &Player::nextTrack);
    connect(this, &MainWindow::previousTrack, player, &Player::previousTrack);
    connect(ui->qaLoop, &QAction::triggered, player, &Player::setLoop);
    connect(player, &Player::songChange, this, &MainWindow::onSongChange);
    connect(player, &Player::rowUpdate, this, &MainWindow::onRowUpdate);
    connect(player, &Player::playbackStarted, this, &MainWindow::onPlaybackStarted);
    connect(player, &Player::playbackPaused, this, &MainWindow::onPlaybackPaused);
    playbackThread.start();
}

MainWindow::~MainWindow()
{
    playbackThread.quit();
    playbackThread.wait();

    delete playlist;
    delete slInfo;
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
    // if some actions should not be usable, like move, this code must be adopted
    event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    if (mimeData->hasUrls())
    {
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();

        for (int i = 0; i < urlList.size(); i++)
        {
            pathList.append(urlList.at(i).toLocalFile());
        }

        if (!pathList.empty())
        {
            event->acceptProposedAction();
            for (auto &item : pathList)
            {
                playlist->append(item);
            }
        }
    }
}

void MainWindow::on_qaPlayPause_triggered()
{
    if (player->playing())
    {
        emit(pause());
        return;
    }

    emit(play());
}

void MainWindow::on_qaNext_triggered()
{
    emit(nextTrack());

    if (!player->playing())
    {
        emit(play());
    }
}

void MainWindow::on_qaPrevious_triggered()
{
    emit(previousTrack());

    if (!player->playing())
    {
        emit(play());
    }
}

void MainWindow::onSongChange(QString songName)
{
    slInfo->setFirstLine(songName);
}

void MainWindow::onRowUpdate(int row, int pattern, int channels)
{
    slInfo->setSecondLine(QString("Playing, Pattern %1, Row %2, %3 channels").arg(pattern).arg(row).arg(channels));
}

void MainWindow::onPlaybackStarted()
{
    ui->qaPlayPause->setIcon(QIcon(":/res/player_pause.png"));
    slInfo->setSecondLine("Playing");
}

void MainWindow::onPlaybackPaused()
{
    ui->qaPlayPause->setIcon(QIcon(":/res/1rightarrow.png"));
    slInfo->setSecondLine("Ready");
}
