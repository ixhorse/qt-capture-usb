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

    void on_sendButton_clicked();

    void on_clearButton_clicked();

    void pipeSelect(QTableWidgetItem *item);

private:
    Ui::MainWindow *ui;
    Thread *handle_thread;
    QLabel *statusLabel;
    char *devName;
    PPIPE_INFO  pipe_info;

    void initTableWidget();
    void initPipeTable();
    void setPipeTable(PPIPE_INFO pipeInfo, int pipeNums);
    void initStatusBar();
    bool findFilter(char *pdoName, int len);
};

#endif // MAINWINDOW_H
