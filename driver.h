#ifndef DRIVER_H
#define DRIVER_H

#include <Windows.h>

#define IOCTL_READ_LIST \
    CTL_CODE(\
            FILE_DEVICE_UNKNOWN,\
            0X822,\
            METHOD_BUFFERED, \
            FILE_ANY_ACCESS)

#define IOCTL_SET_FLAG \
    CTL_CODE(\
            FILE_DEVICE_UNKNOWN,\
            0X823,\
            METHOD_BUFFERED, \
            FILE_ANY_ACCESS)

#define IOCTL_CLEAR_FLAG \
    CTL_CODE(\
            FILE_DEVICE_UNKNOWN,\
            0X824,\
            METHOD_BUFFERED, \
            FILE_ANY_ACCESS)

#define IOCTL_FIND_FILTER \
    CTL_CODE(\
            FILE_DEVICE_UNKNOWN,\
            0X825,\
            METHOD_BUFFERED, \
            FILE_ANY_ACCESS)

#define IOCTL_SEND_DATA \
    CTL_CODE(\
            FILE_DEVICE_UNKNOWN,\
            0X826,\
            METHOD_BUFFERED, \
            FILE_ANY_ACCESS)

typedef struct {
    ULONG TranferFlags;
    ULONG Len;
    UCHAR Buf[100];
    PVOID MDLbuf;
}BULK_STRUCTURE;

typedef struct {
    LIST_ENTRY list_entry;
    BULK_STRUCTURE Bulk_in;
    BULK_STRUCTURE Bulk_out;
} LIST_NODE;

#endif // DRIVER_H
