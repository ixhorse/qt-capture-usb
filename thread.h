#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QMutex>
#include "driver.h"

class Thread : public QThread
{
    Q_OBJECT

public:
    Thread();

    volatile bool stoped;

    void stop();
    //HANDLE getHandle();
    bool openDevice();
    void setCapFlag();
    void clearCapFlag();
    void closeDevice();
    bool findFilter(char *pdo_name, DWORD len);

protected:
    void run();

signals:
    void listInfo(const QStringList &a);

private:
    QMutex mutex;
    HANDLE hDevice;
};

#endif // THREAD_H
