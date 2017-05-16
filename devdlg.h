#ifndef DEVDLG_H
#define DEVDLG_H

#include <QDialog>
#include <QtWidgets>
#include <Setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <shlwapi.h>
#include <Rpcdce.h>
#include <Usbiodef.h>
#include <driver.h>

#define MAX_LEN 1024

namespace Ui {
class devDlg;
}

class devDlg : public QDialog
{
    Q_OBJECT

public:
    explicit devDlg(QWidget *parent = 0);
    ~devDlg();

    int selectedIndex; 

    bool devValid();
    int getDevIndex();
    void getSelectedPdoName(char *buf, int *len);

private slots:
    void selectItem(QListWidgetItem *item);

    void on_okBtn_clicked();

private:
    Ui::devDlg *ui;

    //HDEVINFO hDevInfo;
    GUID guid;
    char *devName;
    int nameLen;
    bool validFlag;

    void EnumDevs();
};

#endif // DEVDLG_H
