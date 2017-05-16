#include "devdlg.h"
#include "ui_devdlg.h"
#include <qmessagebox.h>

devDlg::devDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::devDlg)
{
    QUuid Qguid = QUuid("{36fc9e60-c465-11cf-8056-444553540000}");
    guid = GUID(Qguid);
    validFlag = false;
    selectedIndex = -1;
    devName = NULL;
    nameLen = 0;


    ui->setupUi(this);
    this->setWindowModality(Qt::ApplicationModal);

    ui->okBtn->setEnabled(false);

    ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    EnumDevs();

    connect(ui->refreshBtn, &QPushButton::clicked, this, &devDlg::EnumDevs);
    connect(ui->listWidget, &QListWidget::itemClicked, this, &devDlg::selectItem);
}

devDlg::~devDlg()
{
    delete ui;
}

void devDlg::selectItem(QListWidgetItem *item)
{
    selectedIndex = ui->listWidget->currentRow();

    //QMessageBox::warning(this, tr("warn!"),QString::number(selectedIndex),QMessageBox::Yes);

    ui->okBtn->setEnabled(true);
}

void devDlg::EnumDevs()
{
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD i;

    //clear list
    ui->listWidget->clear();

    guid = GUID(QUuid("{A5DCBF10-6530-11D2-901F-00C04FB951ED}"));
    hDevInfo = SetupDiGetClassDevs(&guid,
                                    0,        // Enumerator
                                    0,
                                   DIGCF_PRESENT |
                                   DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        // Insert error handling here.
        return;
    }
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    // 枚举指定设备信息集合的成员，并将数据放在PSP_DEVINFO_DATA中
    for (i=0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
    {
        DWORD DataT;
        char buffer[MAX_LEN] = {0};
        DWORD buffersize = 0;

        // 获取设备信息
        if (SetupDiGetDeviceRegistryProperty(
            hDevInfo,
            &DeviceInfoData,
            SPDRP_FRIENDLYNAME,
            0L,
            (PBYTE)buffer,
            MAX_LEN,
            &buffersize))
        {
            ui->listWidget->addItem(QString::fromUtf8(buffer, buffersize));
        }
        else if(SetupDiGetDeviceRegistryProperty(
            hDevInfo,
            &DeviceInfoData,
            SPDRP_DEVICEDESC,
            0L,
            (PBYTE)buffer,
            MAX_LEN,
            &buffersize))
        {
            ui->listWidget->addItem(QString::fromUtf8(buffer, buffersize));
        }
    }
    if ( GetLastError()!=NO_ERROR && GetLastError()!=ERROR_NO_MORE_ITEMS )
        return;
    //SetupDiDestroyDeviceInfoList(hDevInfo);
}

void devDlg::on_okBtn_clicked()
{
    int index = selectedIndex;
    //int i;
    SP_DEVINFO_DATA DeviceInfoData;
    HDEVINFO hDevInfo;

    hDevInfo = SetupDiGetClassDevs(&guid,
            0,        // Enumerator
            0,
            DIGCF_PRESENT);

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if(SetupDiEnumDeviceInfo(hDevInfo,
                             index,
                             &DeviceInfoData))
    {
        /*for(i=0; ;i++)
        {
            SP_DEVICE_INTERFACE_DATA DeviceItfcData;
            SP_DEVICE_INTERFACE_DETAIL_DATA InterfaceDetailData;
            ULONG requiredLength = 0;
            ULONG predictedLength = 0;

            DeviceItfcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
            SetupDiEnumDeviceInterfaces (hDevInfo,
                                    0,
                                    &guid,
                                    i,
                                    &DeviceItfcData);
            if (ERROR_NO_MORE_ITEMS == GetLastError())
            {
                break;
            }

            SetupDiGetDeviceInterfaceDetail (
                   hDevInfo,
                   &DeviceItfcData,
                   NULL, // probing so no output buffer yet
                   0, // probing so output buffer length of zero
                   &requiredLength,
                   NULL); // not interested in the specific dev-node

            predictedLength = requiredLength;

            InterfaceDetailData.cbSize = predictedLength;

            if (! SetupDiGetDeviceInterfaceDetail (
                           hDevInfo,
                           &DeviceItfcData,
                           &InterfaceDetailData,
                           predictedLength,
                           &requiredLength,
                           NULL)) {

            }

            wcsncpy(devName, InterfaceDetailData.DevicePath, predictedLength);
            qDebug() << QString::fromUtf8((LPSTR)InterfaceDetailData.DevicePath, predictedLength);
        }
        */

        char buffer[MAX_LEN] = {0};
        ULONG requiredLength = 0;
        ULONG predictedLength = 0;
        if(devName != NULL)
            free(devName);

        SetupDiGetDeviceRegistryProperty(hDevInfo,
                                         &DeviceInfoData,
                                         SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                         0L,
                                         NULL,
                                         0,
                                         &requiredLength);
        predictedLength = requiredLength;
        if (SetupDiGetDeviceRegistryProperty(hDevInfo,
                                         &DeviceInfoData,
                                         SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                         0L,
                                         (PBYTE)buffer,
                                         predictedLength,
                                         &requiredLength))
        {
            devName = (char *)malloc(sizeof(char)*predictedLength);
            memcpy(devName, buffer, predictedLength);
            nameLen = predictedLength;
            //qDebug() << QByteArray(buffer, predictedLength);
            //QMessageBox::warning(this, tr("warn!"),devName,QMessageBox::Yes);

            validFlag = true;
        }

    }
}

bool devDlg::devValid()
{
    return validFlag;
}

int devDlg::getDevIndex()
{
    return selectedIndex;
}

void devDlg::getSelectedPdoName(char *buf, int *len)
{
    *len = nameLen;
    memcpy(buf, devName, nameLen);
    free(devName);
}
