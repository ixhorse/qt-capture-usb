#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "devdlg.h"

#include <qmessagebox.h>
#include <qtextstream.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->initTableWidget();
    this->initStatusBar();

    devName = NULL;

    ui->capBtn->setText("Start Capture");
    ui->capBtn->setEnabled(false);
    ui->selectDevBtn->setEnabled(true);

    handle_thread = new Thread;
    connect(handle_thread,SIGNAL(listInfo(QStringList)),this,SLOT(showList(QStringList)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_selectDevBtn_clicked()
{
    //ui->selectDevBtn->setEnabled(false);

    devDlg      *dlg = new devDlg;
    int         len = 0;
    DWORD       dRet = 0;
    HANDLE      hDevice;
    PVOID       p;

    dlg->show();

    if(devName)
        free(devName);
    devName = (char *)malloc(MAX_LEN);
    if(dlg->exec() == QDialog::Accepted)
    {
        //QMessageBox::warning(this, tr("warn!"),QString::number(dlg->selectedIndex),QMessageBox::Yes);
        if(dlg->devValid())
        {
            //ui->selectDevBtn->setEnabled(false);

            dlg->getSelectedPdoName(devName, &len);
            /*
            if(findFilter(pdo_name, len))
                ui->capBtn->setEnabled(true);
            else
                QMessageBox::warning(this, tr("warn!"), tr("Can't capture this device!"),QMessageBox::Yes);
            */

            hDevice = CreateFile((LPCTSTR)devName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,		// share mode none
                                 NULL,	// no security
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);
             if (hDevice == INVALID_HANDLE_VALUE)
             {
                 QMessageBox::warning(this, tr("warn!"), tr("Can't capture this device!"),QMessageBox::Yes);
                 return;
             }

             DeviceIoControl(hDevice, IOCTL_FIND_FILTER, NULL, 0, &p, 4, &dRet, 0);
             if(dRet != sizeof(PVOID))
             {
                 QMessageBox::warning(this, tr("warn!"), tr("Can't capture this device!"),QMessageBox::Yes);
                 CloseHandle(hDevice);
                 return;
             }
             qDebug() << dRet;

             ui->capBtn->setEnabled(true);
             CloseHandle(hDevice);
        }
    }

}

void MainWindow::on_capBtn_clicked()
{
    DWORD dRet;
    QString str;

    if(ui->capBtn->text() == "Start Capture")
    {
        ui->capBtn->setText("Stop Capture");
        ui->selectDevBtn->setEnabled(false);

        if (!handle_thread->openDevice(devName))
        {
           QMessageBox::warning(this, tr("warn!"),tr("打开设备失败!"),QMessageBox::Yes);
           ui->statusBar->showMessage(tr("Open failed."), 2000);
        }
        else
        {
            ui->statusBar->showMessage(tr("Open success,start capture."), 2000);
            statusLabel->setText("Ctrl+P to pause.");

            handle_thread->setCapFlag();
            handle_thread->start();
        }
    }
    else
    {
        handle_thread->stop();
        handle_thread->clearCapFlag();
        handle_thread->closeDevice();

        ui->capBtn->setText("Start Capture");
        ui->selectDevBtn->setEnabled(true);
        statusLabel->clear();
    }
}

void MainWindow::showList(QStringList data)
{
    int row = ui->tableWidget->rowCount();
    QTableWidgetItem *itemNum = new QTableWidgetItem(QString::number(row+1));
    QTableWidgetItem *itemType = new QTableWidgetItem(QString("Bulk/Interpt:out"));
    QTableWidgetItem *itemHex = new QTableWidgetItem(data.at(0));
    QTableWidgetItem *itemDec = new QTableWidgetItem(data.at(1));


    ui->tableWidget->insertRow(row);

    ui->tableWidget->setItem(row, 0, itemNum);
    ui->tableWidget->setItem(row, 1, itemType);
    ui->tableWidget->setItem(row, 2, itemHex);
    ui->tableWidget->setItem(row, 3, itemDec);

    ui->tableWidget->scrollToBottom();
}

void MainWindow::initTableWidget(){
    int width = 800;

    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setColumnWidth(0, width * 0.1);
    ui->tableWidget->setColumnWidth(1, width * 0.15);
    ui->tableWidget->setColumnWidth(2, width * 0.35);
    //ui->tableWidget->setColumnWidth(3, width * 0.4);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(25);

    QStringList headText;
    headText << "num" << "Type" << "Hex" << "Dec";
    ui->tableWidget->setHorizontalHeaderLabels(headText);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::initStatusBar()
{
    statusLabel = new QLabel;
    statusLabel->setAlignment(Qt::AlignRight);


    ui->statusBar->addPermanentWidget(statusLabel);
    statusBar()->setStyleSheet(QString("QStatusBar::item{border: 0px}"));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_P:
        if(event->modifiers() == Qt::ControlModifier)
        {
            ui->capBtn->click();
        }
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{

}

bool MainWindow::findFilter(char *pdoName, int len)
{
    //qDebug() << len;
    //qDebug("%c", pdoName[len-2]);

    if(handle_thread->openDevice(devName))
    {
        if(handle_thread->findFilter(pdoName, (DWORD)len))
        {
            handle_thread->closeDevice();
            return true;
        }
        handle_thread->closeDevice();
    }

    return false;

}

void MainWindow::on_pushButton_clicked()
{
    HANDLE          hDevice;
    DWORD           dRet = 0;
    QString         len = ui->lenInput->text();
    QString         data = ui->dataInput->text();
    if(len.isEmpty() || data.isEmpty())
    {
        QMessageBox::warning(this, tr("warn!"), tr("input length and data!"),QMessageBox::Yes);
        return;
    }

    hDevice = CreateFile((LPCTSTR)devName,
                     GENERIC_READ | GENERIC_WRITE,
                     0,		// share mode none
                     NULL,	// no security
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
         //QMessageBox::warning(this, tr("warn!"), tr("Can't capture this device!"),QMessageBox::Yes);
         return;
    }

    DeviceIoControl(hDevice, IOCTL_SEND_DATA, data.toLatin1().data(), len.toInt(), NULL, 0, &dRet, 0);

    CloseHandle(hDevice);

}
