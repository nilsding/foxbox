#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QIcon>
#include <QMimeData>
#include <QFileDialog>
#include <QPropertyAnimation>

#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include "m3uparser.h"
#include "m3uwriter.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    slInfo(new StatusLabel),
    qsVolume(new QSlider),
    playlist(new Playlist),
    player(new Player(playlist))
{
    setAcceptDrops(true);
    ui->setupUi(this);

    QWidget* widget = new QWidget(this);
    widget->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    QVBoxLayout* vbox = new QVBoxLayout(widget);
    vbox->setSpacing(5);
    vbox->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(vbox);

    QWidget* buttWidget = new QWidget(this);
    buttWidget->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    QHBoxLayout* hbox = new QHBoxLayout(buttWidget);
    hbox->setSpacing(0);
    hbox->setContentsMargins(0, 0, 0, 0);
    buttWidget->setLayout(hbox);
    QPushButton* qpbPrevious = new QPushButton();
    qpbPrevious->setMinimumSize(QSize(32, 32));
    qpbPrevious->setFlat(true);
    qpbPrevious->setIcon(QIcon(":/res/player_start.png"));
    qpbPrevious->setIconSize(QSize(32, 32));
    connect(qpbPrevious, &QPushButton::clicked, this, &MainWindow::on_qaPrevious_triggered);
    hbox->addWidget(qpbPrevious);
    QPushButton* qpbPlayPause = new QPushButton();
    qpbPlayPause->setMinimumSize(QSize(32, 32));
    qpbPlayPause->setFlat(true);
    qpbPlayPause->setIcon(QIcon(":/res/1rightarrow.png"));
    qpbPlayPause->setIconSize(QSize(32, 32));
    connect(qpbPlayPause, &QPushButton::clicked, this, &MainWindow::on_qaPlayPause_triggered);
    hbox->addWidget(qpbPlayPause);
    QPushButton* qpbNext = new QPushButton();
    qpbNext->setMinimumSize(QSize(32, 32));
    qpbNext->setFlat(true);
    qpbNext->setIcon(QIcon(":/res/player_end.png"));
    qpbNext->setIconSize(QSize(32, 32));
    connect(qpbNext, &QPushButton::clicked, this, &MainWindow::on_qaNext_triggered);
    hbox->addWidget(qpbNext);
    vbox->addWidget(buttWidget);
#if 0
    QToolBar* qtbwtf = new QToolBar("", this);
    QPalette pal;
    pal.setColor(QPalette::Background, Qt::transparent);
    qtbwtf->setPalette(pal);
    qtbwtf->addAction(ui->qaPrevious);
    qtbwtf->addAction(ui->qaPlayPause);
    qtbwtf->addAction(ui->qaNext);
    vbox->addWidget(qtbwtf);
#endif

    qsVolume->setOrientation(Qt::Horizontal);
    qsVolume->setRange(0, 100);
    qsVolume->setValue(100);
    qsVolume->setMaximumWidth(128);
    vbox->addWidget(qsVolume);
    ui->toolBar->insertWidget(ui->qaLoop, widget);

    slInfo->setFirstLine("foxbox 0.1.0");
    slInfo->setSecondLine("Ready");
    ui->toolBar->insertWidget(ui->qaLoop, slInfo);

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
    connect(player, &Player::songChange, this, &MainWindow::onSongChange);
    connect(player, &Player::rowUpdate, this, &MainWindow::onRowUpdate);
    connect(player, &Player::playbackStarted, this, &MainWindow::onPlaybackStarted);
    connect(player, &Player::playbackPaused, this, &MainWindow::onPlaybackPaused);
    playbackThread.start();
}

MainWindow::~MainWindow()
{
    playbackThread.quit();

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
    else if (mimeData->hasText())
    {
        slInfo->setStyleSheet(mimeData->text());
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
    static int previousHeight = 320;
    auto animation = new QPropertyAnimation(this, "size");
    animation->setDuration(500);
    animation->setEasingCurve(QEasingCurve::InOutQuart);

    connect(animation, &QPropertyAnimation::finished, this, [=]()
    {
        if (checked)
        {
            setMaximumHeight(height());
        }
        else
        {
            ui->tableView->show();
        }
    });

    if (checked)
    {
        previousHeight = height();
        animation->setStartValue(QSize(width(), previousHeight));
        animation->setEndValue(QSize(width(), ui->toolBar->height()));
    }
    else
    {
        setMaximumHeight(maximumWidth()); // maximumWidth is always max
        animation->setStartValue(QSize(width(), height()));
        animation->setEndValue(QSize(width(), previousHeight));
    }

    ui->toolBar->setMovable(!checked);
    ui->tableView->hide();
    animation->start();
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
