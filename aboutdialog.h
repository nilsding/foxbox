#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QTime>
#include <QTimer>
#include <QList>

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog();

protected:
    void closeEvent(QCloseEvent* event);
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);

private:
    static QString aboutText;

    QTime _startTime;
    QTimer* _refreshTimer;
    QList<qreal>* _textWidthTable;

    void buildTextWidthTable();
};

#endif // ABOUTDIALOG_H
