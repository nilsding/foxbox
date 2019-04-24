#ifndef SONGINFOWINDOW_H
#define SONGINFOWINDOW_H

#include <QObject>
#include <QDialog>
#include <QResizeEvent>
#include <QSizeGrip>
#include <QTabWidget>
#include <QMouseEvent>

#include "song.h"
#include "styledtitlebar.h"

class SongInfoWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SongInfoWindow(Song* song, QDialog* parent = nullptr);

signals:

public slots:
    void onMinimizeClicked();
    void onCloseClicked();

protected:
    void resizeEvent(QResizeEvent* event);

private:
    Song* _song;
    StyledTitleBar* _stb;
    QSizeGrip* _sizeGrip;
    QTabWidget* _qtw;

    QWidget* _samplesWidget;
    QWidget* _instrumentsWidget;
    QWidget* _songCommentWidget;

    QPoint _dragPosition;

    void initializeComponents();
    void moveSizeGrip();

private slots:
    void onTitleBarMouseMoved(QMouseEvent* event);
    void onTitleBarMousePressed(QMouseEvent* event);
};

#endif // SONGINFOWINDOW_H
