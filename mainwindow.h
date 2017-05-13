#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "thread.h"

#include <QMainWindow>
#include <QtWidgets>
#include <QKeyEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_selectDevBtn_clicked();
    void on_capBtn_clicked();
    void showList(QStringList);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    Ui::MainWindow *ui;
    Thread *handle_thread;
    QLabel *statusLabel;

    void initTableWidget();
    void initStatusBar();
    bool findFilter(QString pdoName);
};

#endif // MAINWINDOW_H
