#ifndef STYLEDTITLEBAR_H
#define STYLEDTITLEBAR_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMouseEvent>
#include <QMenu>

class StyledTitleBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

public:
    explicit StyledTitleBar(QWidget* parent);
    explicit StyledTitleBar(const QString& title, QWidget* parent);

    QString title() const { return _title; }
    QMenu* menu() const;
    void setMenu(QMenu* menu);

signals:
    void titleChanged(const QString& title);
    void minimizeClicked();
    void closeClicked();

    void mouseMoved(QMouseEvent* event);
    void mousePressed(QMouseEvent* event);

public slots:
    void setTitle(const QString& title);

protected:
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);

private:
    QString _title;

    QPushButton* _qpbMenu;
    QPushButton* _qpbMinimize;
    QPushButton* _qpbClose;
    QLabel* _qlTitle;

    QPoint _dragPosition;

private slots:
    void onMinimizeClicked();
    void onCloseClicked();
};

#endif // STYLEDTITLEBAR_H
