#include "thread.h"
#include <qstring.h>
#include <qtextstream.h>

Thread::Thread()
{
   stoped = false;
}

void Thread::stop()
{
    stoped=true;
}

void Thread::run()
{
    LIST_NODE list_node;
    DWORD dRet;
    QStringList data;
    QString temp;
    int i;
    while(!stoped)
    {
        mutex.lock();

        data.clear();
        DeviceIoControl(hDevice, IOCTL_READ_LIST, NULL, 0, &list_node, sizeof(LIST_NODE), &dRet, NULL);
        if(dRet > 0)
        {
            temp.clear();
            for(i=0; i<list_node.Bulk_in.Len; i++)
                //QTextStream(temp) << list_node.Bulk_out.Buf[i];
                //temp.append(QString("%1").arg(list_node.Bulk_out.Buf[i]));
                temp.append(QString("").sprintf("%02x ",list_node.Bulk_out.Buf[i]));
            data << temp;

            temp.clear();
            for(i=0; i<list_node.Bulk_in.Len; i++)
                temp.append(QString("%1").arg((char)list_node.Bulk_in.Buf[i]));
            data << temp;
        }
        else
        {
            data << "empty" << "empty";
            msleep(1000);
        }
        emit listInfo(data);

        mutex.unlock();
        msleep(1);
    }
    stoped=false;
}

bool Thread::openDevice(char *devName)
{
    hDevice =CreateFile((LPCTSTR)devName,
                        GENERIC_READ | GENERIC_WRITE,
                        0,		// share mode none
                        NULL,	// no security
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void Thread::setCapFlag()
{
    DWORD dRet;
    if(hDevice)
        DeviceIoControl(hDevice, IOCTL_SET_FLAG, NULL, 0, NULL, 0, &dRet, 0);
}

void Thread::clearCapFlag()
{
    DWORD dRet;

    if(hDevice)
        DeviceIoControl(hDevice, IOCTL_CLEAR_FLAG, NULL, 0, NULL, 0, &dRet, 0);
}

void Thread::closeDevice()
{
    if(hDevice)
        CloseHandle(hDevice);
}

bool Thread::findFilter(char *pdo_name, DWORD len)
{
    int find_flag = 0;
    DWORD dRet = 0;

    if(hDevice)
    {
        DeviceIoControl(hDevice, IOCTL_FIND_FILTER, pdo_name, len, &find_flag, sizeof(int), &dRet, 0);
        if(find_flag)
            return true;
    }
    return false;
}
