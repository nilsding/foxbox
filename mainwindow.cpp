#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QIcon>
#include <QMimeData>
#include <QFileDialog>
#include <QMessageBox>
#include <QPropertyAnimation>

#include "m3uparser.h"
#include "m3uwriter.h"
#include "styledmainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    slInfo(new StatusLabel),
    qsVolume(new QSlider),
    playlist(new Playlist),
    player(new Player(playlist)),
    qtStatusUpdate(new QTimer)
{
    setAcceptDrops(true);
    ui->setupUi(this);
    ui->menuBar->setNativeMenuBar(true);
    ui->menuBar->hide();

    slInfo->setFirstLine(QString("foxbox %1").arg(QApplication::instance()->applicationVersion()));
    slInfo->setSecondLine("Ready");
    ui->toolBar->insertSeparator(ui->qaLoop);
    ui->toolBar->insertWidget(ui->qaLoop, slInfo);
    ui->toolBar->insertSeparator(ui->qaLoop);
    qsVolume->setOrientation(Qt::Horizontal);
    qsVolume->setRange(0, 100);
    qsVolume->setValue(100);
    qsVolume->setCursor(QCursor(QPixmap(":/res/scrollh.png"), 0, 0));
    ui->toolBar->insertWidget(ui->qaLoop, qsVolume);
    ui->toolBar->insertSeparator(ui->qaLoop);
    setMinimumHeight(ui->toolBar->height());

    playlist->setParent(ui->tableView);
    ui->tableView->setModel(playlist);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setDefaultSectionSize(200);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView->verticalHeader()->setDefaultSectionSize(10);

    player->moveToThread(&playbackThread);
    connect(this, &MainWindow::play, player, &Player::play);
    connect(this, &MainWindow::pause, player, &Player::pause);
    connect(this, &MainWindow::nextTrack, player, &Player::nextTrack);
    connect(this, &MainWindow::previousTrack, player, &Player::previousTrack);
    connect(qsVolume, &QSlider::valueChanged, player, &Player::setVolume);
    connect(ui->qaLoop, &QAction::triggered, player, &Player::setLoop);
    connect(ui->qaPlayPause, &QAction::triggered, this, &MainWindow::onPlayPressed);
    connect(ui->qaNext, &QAction::triggered, this, &MainWindow::onNextPressed);
    connect(ui->qaPrevious, &QAction::triggered, this, &MainWindow::onPreviousPressed);
    connect(player, &Player::songChange, this, &MainWindow::onSongChange);
    connect(player, &Player::playbackStarted, this, &MainWindow::onPlaybackStarted);
    connect(player, &Player::playbackPaused, this, &MainWindow::onPlaybackPaused);
    playbackThread.start();

    qtStatusUpdate->setInterval(10);
    connect(qtStatusUpdate, &QTimer::timeout, this, &MainWindow::updateStatusBar);
}

MainWindow::~MainWindow()
{
    playbackThread.quit();

    delete qtStatusUpdate;
    delete player;
    delete playlist;
    delete qsVolume;
    delete slInfo;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    emit(pause());
    event->accept();

    playbackThread.wait(500);
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
            if (pathList.count() == 1 && pathList.first().endsWith(".m3u", Qt::CaseInsensitive))
            {
                loadPlaylistFromFile(pathList.first());
                return;
            }

            for (auto &item : pathList)
            {
                if (item.endsWith(".m3u", Qt::CaseInsensitive))
                {
                    continue;
                }
                playlist->append(item);
            }
        }
    }
}

void MainWindow::onPlayPressed()
{
    if (player->playing())
    {
        emit(pause());
        return;
    }

    emit(play());
}

void MainWindow::onNextPressed()
{
    emit(nextTrack());

    if (!player->playing())
    {
        emit(play());
    }
}

void MainWindow::onPreviousPressed()
{
    emit(previousTrack());

    if (!player->playing())
    {
        emit(play());
    }
}

void MainWindow::on_qaLoadPlaylist_triggered()
{
    QFileDialog qfdOpen(this);
    qfdOpen.setFileMode(QFileDialog::ExistingFile);
    qfdOpen.setNameFilter(tr("Playlist files (*.m3u)"));
    qfdOpen.setWindowTitle(tr("Load playlist"));

    if (!qfdOpen.exec())
    {
        return;
    }

    QStringList files = qfdOpen.selectedFiles();

    // TODO: error handling, perhaps
    if (!files.count())
    {
        return;
    }

    auto playlistPath = files.first();
    loadPlaylistFromFile(playlistPath);
}

void MainWindow::on_qaSavePlaylist_triggered()
{
    QFileDialog qfdSave(this);
    qfdSave.setFileMode(QFileDialog::AnyFile);
    qfdSave.setNameFilter(tr("Playlist files (*.m3u)"));
    qfdSave.setWindowTitle(tr("Save playlist"));
    qfdSave.setAcceptMode(QFileDialog::AcceptSave);
    qfdSave.setConfirmOverwrite(true);

    if (!qfdSave.exec())
    {
        return;
    }

    QStringList files = qfdSave.selectedFiles();

    // TODO: error handling, perhaps
    if (!files.count())
    {
        return;
    }

    auto playlistPath = files.first();
    savePlaylistToFile(playlistPath);
}

void MainWindow::on_qaMiniplayer_triggered(bool checked)
{
    QMainWindow* targetObject = this;
    if (parent() != nullptr)
    {
        // parent -> centralWidget of StyledMainWindow,
        //  ` parent -> the actual StyledMainWindow
        targetObject = reinterpret_cast<QMainWindow*>(parent()->parent());
    }
    static int previousHeight = 320;
    auto animation = new QPropertyAnimation(targetObject, "size");
    animation->setDuration(500);
    animation->setEasingCurve(QEasingCurve::InOutQuart);

    connect(animation, &QPropertyAnimation::finished, this, [=]()
    {
        if (checked)
        {
            ui->tableView->hide();
            targetObject->setMaximumHeight(targetObject->height());
        }
    });

    if (checked)
    {
        previousHeight = targetObject->height();
        animation->setStartValue(QSize(targetObject->width(), previousHeight));
        animation->setEndValue(QSize(targetObject->width(), targetObject->minimumHeight()));
    }
    else
    {
        targetObject->setMaximumHeight(targetObject->maximumWidth()); // maximumWidth is always max
        animation->setStartValue(QSize(targetObject->width(), targetObject->height()));
        animation->setEndValue(QSize(targetObject->width(), previousHeight));
        ui->tableView->show();
    }

    animation->start();
}

void MainWindow::on_qaAboutFoxbox_triggered()
{
    auto win = qobject_cast<StyledMainWindow*>(parent()->parent());
    win->onqaFoxboxTriggered();
}

void MainWindow::on_qaAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex& index)
{
    playlist->setCurrentIndex(index.row());
    if (!player->playing())
    {
        emit(play());
    }
}

void MainWindow::onSongChange(QString songName)
{
    slInfo->setFirstLine(songName);
}

void MainWindow::onPlaybackStarted()
{
    ui->qaPlayPause->setIcon(QIcon(":/res/player_pause.png"));
    slInfo->setSecondLine("Playing");
    qtStatusUpdate->start();
}

void MainWindow::onPlaybackPaused()
{
    ui->qaPlayPause->setIcon(QIcon(":/res/1rightarrow.png"));
    slInfo->setSecondLine("Ready");
    qtStatusUpdate->stop();
}

void MainWindow::updateStatusBar()
{
    auto info = player->currentTimeInfo();

    if (!info.valid)
    {
        return;
    }

    slInfo->setSecondLine(
        QString("Playing, Order %1/%2, Pattern %3, Row %4, %5 channels")
        .arg(QString::number(info.order),
             QString::number(info.orderCount),
             QString::number(info.pattern),
             QString::number(info.row),
             QString::number(info.channels)));
}

void MainWindow::loadPlaylistFromFile(const QString& playlistPath)
{
    auto parser = new M3uParser(playlistPath);
    auto resolvedFiles = parser->parse();

    if (resolvedFiles != nullptr && !resolvedFiles->empty())
    {
        player->mutex.lock();
        bool wasPlaying = player->playing();
        if (wasPlaying)
        {
            player->pause();
            onPlaybackPaused();
        }

        playlist->clear();
        for (auto &item : *resolvedFiles)
        {
            playlist->append(item);
        }
        playlist->setCurrentIndex(0);

        if (wasPlaying)
        {
            emit(play());
        }
        player->mutex.unlock();
    }

    delete parser;
}

void MainWindow::savePlaylistToFile(const QString& playlistPath)
{
    auto writer = new M3uWriter(playlistPath);

    QStringList paths;
    auto itemCount = playlist->rowCount();
    for (int i = 0; i < itemCount; i++)
    {
        paths << playlist->at(i)->path();
    }
    writer->write(&paths);

    delete writer;
}
