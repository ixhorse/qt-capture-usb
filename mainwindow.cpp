#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "devdlg.h"

#include <qmessagebox.h>
#include <qtextstream.h>
#include <winusb.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->initTableWidget();
    this->initStatusBar();
    this->initPipeTable();

    devName = NULL;

    ui->capBtn->setText("Start Capture");
    ui->capBtn->setEnabled(false);
    ui->selectDevBtn->setEnabled(true);
    ui->sendButton->setEnabled(false);
    ui->clearButton->setEnabled(false);

    handle_thread = new Thread;
    connect(handle_thread,SIGNAL(listInfo(QStringList)),this,SLOT(showList(QStringList)));
    connect(ui->pipeTable, &QTableWidget::itemClicked, this, &MainWindow::pipeSelect);
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
    int         sz = 1;
    int         pipeNums;
    DWORD       dRet = 0;
    HANDLE      hDevice;
    PVOID       p;

    dlg->show();

    if(devName)
        free(devName);
    devName = (char *)malloc(MAX_LEN);

    if(dlg->exec() == QDialog::Accepted)
    {
        if(dlg->devValid())
        {
            dlg->getSelectedPdoName(devName, &len);

            hDevice = CreateFile((LPCTSTR)devName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,		// share mode none
                                 NULL,	// no security
                                 OPEN_EXISTING,
                                 FILE_FLAG_OVERLAPPED,
                                 NULL);
             if (hDevice == INVALID_HANDLE_VALUE)
             {
                 QMessageBox::warning(this, tr("warn!"), tr("Can't capture this device!"),QMessageBox::Yes);
                 ui->capBtn->setEnabled(false);
                 ui->sendButton->setEnabled(false);
                 return;
             }

             if(pipe_info)
                 free(pipe_info);
             pipe_info = (PPIPE_INFO)malloc(sizeof(PIPE_INFO) * 10);

             DeviceIoControl(hDevice, IOCTL_FIND_FILTER, NULL, 0, pipe_info, sizeof(PIPE_INFO) * 10, &dRet, 0);

             if(dRet <= 0)
             {
                 QMessageBox::warning(this, tr("warn!"), tr("IOCTL_FIND_FILTER failed!"),QMessageBox::Yes);
                 CloseHandle(hDevice);
                 ui->capBtn->setEnabled(false);
                 ui->sendButton->setEnabled(false);
                 return;
             }

             pipeNums = dRet / sizeof(PIPE_INFO);

             qDebug() << pipeNums;

             setPipeTable(pipe_info, pipeNums);

             ui->capBtn->setEnabled(true);

             //Interface = (PWINUSB_INTERFACE_HANDLE)LocalAlloc(LPTR, sizeof(WINUSB_INTERFACE_HANDLE) * sz);
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
        ui->clearButton->setEnabled(false);

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
        ui->clearButton->setEnabled(true);
        statusLabel->clear();
    }
}

void MainWindow::showList(QStringList data)
{
    int row = ui->tableWidget->rowCount();
    QTableWidgetItem *itemNum = new QTableWidgetItem(QString::number(row+1));
    QTableWidgetItem *itemType = new QTableWidgetItem(QString("Bulk/Interpt"));
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
    headText << "num" << "Type" << "Out" << "In";
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

void MainWindow::on_sendButton_clicked()
{
    HANDLE          hDevice;
    DWORD           dRet = 0;
    DWORD           pipeIndex;
    QString         len = ui->lenInput->text();
    QString         data = ui->dataInput->text();
    QString         temp;
    char            *buf;
    int             i;

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

    buf = (char *)malloc(len.toInt());
    pipeIndex = ui->pipeTable->currentRow();
    qDebug() << pipeIndex;

    DeviceIoControl(hDevice, IOCTL_SELECT_PIPE, &(pipe_info[pipeIndex].PipeHandle), sizeof(USBD_PIPE_HANDLE),
                    NULL, 0, &dRet, 0);

    DeviceIoControl(hDevice, IOCTL_SEND_DATA, data.toLatin1().data(), len.toInt(), buf, len.toInt(), &dRet, 0);

    if(dRet != 0)
    {
        temp.clear();
        for(i=0; i<dRet; i++)
            temp.append(QString("%1 ").arg(buf[i], 2, 16, QLatin1Char('0')));
        ui->dataInput->setText(temp);
    }

    CloseHandle(hDevice);

}

void MainWindow::initPipeTable()
{
    int         width = 700;
    int         i;
    QStringList headText;

    ui->pipeTable->setColumnCount(7);
    for(i=0; i<7; i++)
    {
        ui->pipeTable->setColumnWidth(i, width * 0.1);
    }
    ui->pipeTable->verticalHeader()->setDefaultSectionSize(25);

    headText << "Endpoint" << "Type" << "Direction" << "Class" << "Subclass" << "Protocol" << "Max Packet";
    ui->pipeTable->setHorizontalHeaderLabels(headText);
    ui->pipeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->pipeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->pipeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->pipeTable->setAlternatingRowColors(true);
    ui->pipeTable->verticalHeader()->setVisible(false);
    ui->pipeTable->horizontalHeader()->setStretchLastSection(true);
}


void MainWindow::setPipeTable(PPIPE_INFO pipeInfo, int pipeNums)
{
    int i;
    QTableWidgetItem    *itemEndpoint,
                        *itemType,
                        *itemDirection,
                        *itemClass,
                        *itemSubclass,
                        *itemProtocol,
                        *itemMaxpacket;

    for(i=0; i<pipeNums; i++)
    {
        itemEndpoint = new QTableWidgetItem(QString::number(i+1));
        itemClass = new QTableWidgetItem(QString("%1").arg(pipeInfo[i].Class, 2, 16, QLatin1Char('0')));
        itemSubclass = new QTableWidgetItem(QString("%1").arg(pipeInfo[i].Subclass, 2, 16, QLatin1Char('0')));
        itemProtocol = new QTableWidgetItem(QString("%1").arg(pipeInfo[i].Protocol, 2, 16, QLatin1Char('0')));
        itemMaxpacket = new QTableWidgetItem(QString::number(pipeInfo[i].MaximumPacketSize));
        switch(pipeInfo[i].PipeType)
        {
        case Bulk:
            itemType = new QTableWidgetItem(QString("Bulk"));
            break;
        case Interrupt:
            itemType = new QTableWidgetItem(QString("Interrupt"));
            break;
        case Control:
            itemType = new QTableWidgetItem(QString("Control"));
            break;
        default:
            itemType = new QTableWidgetItem(QString("-"));
            break;
        }
        switch(pipeInfo[i].Direction)
        {
        case In:
            itemDirection = new QTableWidgetItem(QString("In"));
            break;
        case Out:
            itemDirection = new QTableWidgetItem(QString("Out"));
            break;
        case Inout:
            itemDirection = new QTableWidgetItem(QString("In/Out"));
            break;
        }

        ui->pipeTable->insertRow(i);

        ui->pipeTable->setItem(i, 0, itemEndpoint);
        ui->pipeTable->setItem(i, 1, itemType);
        ui->pipeTable->setItem(i, 2, itemDirection);
        ui->pipeTable->setItem(i, 3, itemClass);
        ui->pipeTable->setItem(i, 4, itemSubclass);
        ui->pipeTable->setItem(i, 5, itemProtocol);
        ui->pipeTable->setItem(i, 6, itemMaxpacket);
    }

}

void MainWindow::on_clearButton_clicked()
{
    ui->tableWidget->setRowCount(0);
}

void MainWindow::pipeSelect(QTableWidgetItem *item)
{
    ui->sendButton->setEnabled(true);
}
