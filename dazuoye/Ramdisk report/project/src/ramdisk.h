/*
ramdisk.h
Ramdisk驱动程序的数据声明

author : 数媒1701班 周展科 李一泓 杨青
title  : 2019 操作系统原理 大作业 题目10
*/

#ifndef _RAMDISK_H_
#define _RAMDISK_H_

// #pragma is commonly used to suppress warnings
#pragma warning(disable:4201)  // nameless struct/union warning

#include <ntddk.h>
#include <ntdddisk.h>
#include <mountmgr.h>

#pragma warning(default:4201)

#include <wdf.h>
#define NTSTRSAFE_LIB
#include <ntstrsafe.h>
#include "forward_progress.h"

#define NT_DEVICE_NAME                  L"\\Device\\Ramdisk"
#define DOS_DEVICE_NAME                 L"\\DosDevices\\"

#define RAMDISK_TAG                     'DmaR'  // "RamD"
#define DOS_DEVNAME_LENGTH              (sizeof(DOS_DEVICE_NAME)+sizeof(WCHAR)*10)
#define DRIVE_LETTER_LENGTH             (sizeof(WCHAR)*10)

#define DRIVE_LETTER_BUFFER_SIZE        10
#define DOS_DEVNAME_BUFFER_SIZE         (sizeof(DOS_DEVICE_NAME) / 2) + 10

#define RAMDISK_MEDIA_TYPE              0xF8
#define DIR_ENTRIES_PER_SECTOR          16

// 这些都是默认参数, 只有在注册表中没有该项才启用, 但是安装文件已经设置了
#define DEFAULT_DISK_SIZE               (1024*1024)     // 1 MB
#define DEFAULT_ROOT_DIR_ENTRIES        512
#define DEFAULT_SECTORS_PER_CLUSTER     2
#define DEFAULT_DRIVE_LETTER            L"Z:"

// 定义磁盘信息结构体
typedef struct _DISK_INFO {
    ULONG   DiskSize;           // 磁盘的大小以Byte计算, 所以不能够超过4G
    ULONG   RootDirEntries;     // 系统上根文件系统的进入点
    ULONG   SectorsPerCluster;  // 磁盘的每个族由多少个扇区组成
    UNICODE_STRING DriveLetter; // 磁盘的盘符
} DISK_INFO, *PDISK_INFO;

typedef struct _DEVICE_EXTENSION {
	PUCHAR              DiskImage;                  // 指向一块内存区域, 作为内存盘的实际存储空间
	DISK_GEOMETRY       DiskGeometry;               // 存储内存盘的物理信息, WinDDK提供
	DISK_INFO           DiskRegInfo;                // 我们自己定义的磁盘信息结构, 在安装时存放在注册表中
	UNICODE_STRING      SymbolicLink;               // 共享给用户态的符号链接名称
	WCHAR               DriveLetterBuffer[DRIVE_LETTER_BUFFER_SIZE];	//DiskRegInfo中的盘符的存储空间
	WCHAR               DosDeviceNameBuffer[DOS_DEVNAME_BUFFER_SIZE];	//符号连接名的存放空间
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, DeviceGetExtension)

typedef struct _QUEUE_EXTENSION {
    PDEVICE_EXTENSION DeviceExtension;
} QUEUE_EXTENSION, *PQUEUE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_EXTENSION, QueueGetExtension)

#pragma pack(1)

typedef struct  _BOOT_SECTOR {
	UCHAR       bsJump[3];          // 跳转指令, 跳到DBR中的引导程序
	CCHAR       bsOemName[8];       // 卷的OEM名称
	USHORT      bsBytesPerSec;      // 每个扇区有多少个字节
	UCHAR       bsSecPerClus;       // 每个族有多少个扇区
	USHORT      bsResSectors;       // 保留扇区数目, 指的是第一个FAT
	UCHAR       bsFATs;             // 这个卷有多少个FAT表
	USHORT      bsRootDirEnts;      // 这个卷根入口点有几个
	USHORT      bsSectors;          // 这个卷有多少个扇区, 对于大于65535个的扇区卷, 这个字段为0
	UCHAR       bsMedia;            // 这个卷的介质类型 RAMDISK_MEDIA_TYPE
	USHORT      bsFATsecs;          // 每个FAT表占用多少个扇区
	USHORT      bsSecPerTrack;      // 每个磁道有多少个扇区, 我们使用32
	USHORT      bsHeads;            // 有多少个磁头, 我们使用2
	ULONG       bsHiddenSecs;       // 有多少个隐藏分区, 我们使用0
	ULONG       bsHugeSectors;      // 一个卷如果超过65535扇区, 会使用这字段来说明总扇区数
	UCHAR       bsDriveNumber;      // 驱动器编号, 未使用
	UCHAR       bsReserved1;        // 保留字段
	UCHAR       bsBootSignature;    // 磁盘扩展引导区标志, Windows规定必须为 0x29 或者0x28
	ULONG       bsVolumeID;         // 磁盘卷ID - set to 0x12345678
	CCHAR       bsLabel[11];        // 磁盘卷标
	CCHAR       bsFileSystemType[8];// 文件系统类型 - FAT12 or FAT16
	CCHAR       bsReserved2[448];   // 保留
	UCHAR       bsSig2[2];          // DBR结束标记, 必须以0x55AA结束 - 0x55, 0xAA
}   BOOT_SECTOR, *PBOOT_SECTOR;

typedef struct  _DIR_ENTRY {
	UCHAR       deName[8];          // 文件名称
	UCHAR       deExtension[3];     // 文件扩展名
	UCHAR       deAttributes;       // 文件属性
	UCHAR       deReserved;         // 系统保留
	USHORT      deTime;             // 文件建立时间
	USHORT      deDate;             // 文件建立日期
	USHORT      deStartCluster;     // 文件第一个族的编号
	ULONG       deFileSize;         // 文件大小
}   DIR_ENTRY, *PDIR_ENTRY;


#pragma pack()

//
// Directory Entry Attributes
//
#define DIR_ATTR_READONLY   0x01
#define DIR_ATTR_HIDDEN     0x02
#define DIR_ATTR_SYSTEM     0x04
#define DIR_ATTR_VOLUME     0x08
#define DIR_ATTR_DIRECTORY  0x10
#define DIR_ATTR_ARCHIVE    0x20

DRIVER_INITIALIZE DriverEntry;

#if KMDF_VERSION_MINOR >= 7

EVT_WDF_DRIVER_DEVICE_ADD RamDiskEvtDeviceAdd;
EVT_WDF_DEVICE_CONTEXT_CLEANUP RamDiskEvtDeviceContextCleanup;
EVT_WDF_IO_QUEUE_IO_READ RamDiskEvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE RamDiskEvtIoWrite;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL RamDiskEvtIoDeviceControl;

#else

NTSTATUS
RamDiskEvtDeviceAdd(
    IN WDFDRIVER Driver,
    IN PWDFDEVICE_INIT DeviceInit
    );

VOID
RamDiskEvtDeviceContextCleanup(
    IN WDFOBJECT Device
    );

VOID
RamDiskEvtIoRead(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t Length
    );

VOID
RamDiskEvtIoWrite(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t Length
    );

VOID
RamDiskEvtIoDeviceControl(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t OutputBufferLength,
    IN size_t InputBufferLength,
    IN ULONG IoControlCode
    );

#endif

VOID
RamDiskQueryDiskRegParameters(
    _In_ PWSTR RegistryPath,
    _In_ PDISK_INFO DiskRegInfo
    );

NTSTATUS
RamDiskFormatDisk(
    IN PDEVICE_EXTENSION DeviceExtension
    );

BOOLEAN
RamDiskCheckParameters(
    IN PDEVICE_EXTENSION devExt,
    IN LARGE_INTEGER ByteOffset,
    IN size_t Length
    );

#endif    // _RAMDISK_H_

