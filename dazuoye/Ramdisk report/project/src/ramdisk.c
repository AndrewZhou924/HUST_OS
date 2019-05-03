/*
Ramdisk.c
Ramdisk驱动程序，将内存的一部分模拟成硬盘，将所有的读写请求转到内存中
创建一个非分页池，并将其作为存储介质公开
用户可以在磁盘管理器(disk manager)中找到该设备并格式化该媒体以用作FAT或NTFS卷。
仅限在内核模式使用

author : 数媒1701班 周展科 李一泓 杨青
title  : 2019 操作系统原理 大作业 题目10
*/

#include "ramdisk.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, RamDiskEvtDeviceAdd)
#pragma alloc_text(PAGE, RamDiskEvtDeviceContextCleanup)
#pragma alloc_text(PAGE, RamDiskQueryDiskRegParameters)
#pragma alloc_text(PAGE, RamDiskFormatDisk)
#endif


/*
入口函数，即驱动加载函数
Description: 
Installable driver initialization entry point.
This entry point is called directly by the I/O system.

parameters:
DriverObject - pointer to the driver object
RegistryPath - pointer to a unicode string representing the path
                to driver-specific key in the registry
Return Value: STATUS_SUCCESS if successful.

关于 NTSTATUS:
An NTSTATUS code is an error code used in Windows NT, Windows XP, Windows Vista, Windows, and Windows 8 
to communicate error results throughout the operating system. 
Device driver developers use NTSTATUS codes both when they make calls into Windows APIs 
as well as when driver devleopers return error codes to Windows.
*/
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath) {
    // 初始化可配置变量config
    WDF_DRIVER_CONFIG config;

    // KdPrint来输出调试信息
    KdPrint(("Windows Ramdisk Driver - Driver Framework Edition.\n"));
    KdPrint(("Built %s %s\n", __DATE__, __TIME__));

    // 指定RamDiskEvtDeviceAdd的地址，这里的RamDiskEvtDeviceAdd相当于wdm中AddDevice回调函数
    // 当即插即用管理器发现了一个新设备 ，则调用的函数
    WDF_DRIVER_CONFIG_INIT( &config, RamDiskEvtDeviceAdd );

    //调用WdfDriverCreate返回，WdfDriverCreate创建了WDF驱动框架
    return WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE);
}



/*
设备读取请求, 类型NT式驱动的IRP_MJ_READ
Description:
This event is called when the framework receives IRP_MJ_READ request.

parameters:
Queue -  Handle to the framework queue object that is associated with the I/O request.
Request - Handle to a framework request object.
Length - Length of the data buffer associated with the request.
        The default property of the queue is to not dispatch
        zero length read & write requests to the driver and
        complete is with status success. So we will never get
        a zero length request.
*/
VOID RamDiskEvtIoRead(IN WDFQUEUE Queue, IN WDFREQUEST Request, IN size_t Length) {
    PDEVICE_EXTENSION      devExt = QueueGetExtension(Queue)->DeviceExtension;
    NTSTATUS               Status = STATUS_INVALID_PARAMETER;
    WDF_REQUEST_PARAMETERS Parameters;
    LARGE_INTEGER          ByteOffset;
    WDFMEMORY              hMemory;

    _Analysis_assume_(Length > 0);

    WDF_REQUEST_PARAMETERS_INIT(&Parameters);
    WdfRequestGetParameters(Request, &Parameters);

    ByteOffset.QuadPart = Parameters.Parameters.Read.DeviceOffset;

    if (RamDiskCheckParameters(devExt, ByteOffset, Length)) {

        Status = WdfRequestRetrieveOutputMemory(Request, &hMemory);
        if(NT_SUCCESS(Status)) {

            Status = WdfMemoryCopyFromBuffer(hMemory,   // Destination
                                             0,         // Offset into the destination
                                             devExt->DiskImage + ByteOffset.LowPart, // source
                                             Length);
        }
    }
    
    KdPrint(( "读取请求来了一次 ByteOffset:%p Length:%d\n",(ULONG)ByteOffset.QuadPart, Length ));

    // 类似于这号函数IoCompleteRequest
    WdfRequestCompleteWithInformation(Request, Status, (ULONG_PTR)Length);
}


/*
设备写入请求, 类型NT式驱动的IRP_MJ_WRITE

Description:
    This event is invoked when the framework receives IRP_MJ_WRITE request.

parameters:
    Queue -  Handle to the framework queue object that is associated with the
             I/O request.
    Request - Handle to a framework request object.
    Length - Length of the data buffer associated with the request.
             The default property of the queue is to not dispatch
             zero length read & write requests to the driver and
             complete is with status success. So we will never get
             a zero length request.
*/
VOID RamDiskEvtIoWrite(IN WDFQUEUE Queue, IN WDFREQUEST Request, IN size_t Length) {
    PDEVICE_EXTENSION      devExt = QueueGetExtension(Queue)->DeviceExtension;
    NTSTATUS               Status = STATUS_INVALID_PARAMETER;
    WDF_REQUEST_PARAMETERS Parameters;
    LARGE_INTEGER          ByteOffset;
    WDFMEMORY              hMemory;

    _Analysis_assume_(Length > 0);

    WDF_REQUEST_PARAMETERS_INIT(&Parameters);
    WdfRequestGetParameters(Request, &Parameters);

    ByteOffset.QuadPart = Parameters.Parameters.Write.DeviceOffset;

    if (RamDiskCheckParameters(devExt, ByteOffset, Length)) {

        Status = WdfRequestRetrieveInputMemory(Request, &hMemory);
        if(NT_SUCCESS(Status)){

            Status = WdfMemoryCopyToBuffer(hMemory, // Source
                                    0,              // offset in Source memory where the copy has to start
                                    devExt->DiskImage + ByteOffset.LowPart, // destination
                                    Length);
        }

    }

    WdfRequestCompleteWithInformation(Request, Status, (ULONG_PTR)Length);
}

/*
设备控制请求, 类似NT式驱动的IRP_MJ_DEVICE_CONTROL

Description:
    This event is called when the framework receives IRP_MJ_DEVICE_CONTROL
    requests from the system.

parameters:
    Queue - Handle to the framework queue object that is associated
            with the I/O request.
    Request - Handle to a framework request object.

    OutputBufferLength - length of the request's output buffer,
                        if an output buffer is available.
    InputBufferLength - length of the request's input buffer,
                        if an input buffer is available.

    IoControlCode - the driver-defined or system-defined I/O control code
                    (IOCTL) that is associated with the request.
*/
VOID
RamDiskEvtIoDeviceControl(IN WDFQUEUE Queue, 
                            IN WDFREQUEST Request, 
                            IN size_t OutputBufferLength, 
                            IN size_t InputBufferLength, 
                            IN ULONG IoControlCode) {
    NTSTATUS          Status = STATUS_INVALID_DEVICE_REQUEST;
    ULONG_PTR         information = 0;
    size_t            bufSize;
    PDEVICE_EXTENSION devExt = QueueGetExtension(Queue)->DeviceExtension;

    switch (IoControlCode) {
    case IOCTL_DISK_GET_PARTITION_INFO: {

            PPARTITION_INFORMATION outputBuffer;
            PBOOT_SECTOR bootSector = (PBOOT_SECTOR) devExt->DiskImage;

            information = sizeof(PARTITION_INFORMATION);

            Status = WdfRequestRetrieveOutputBuffer(Request, sizeof(PARTITION_INFORMATION), &outputBuffer, &bufSize);
            if(NT_SUCCESS(Status) ) {

                outputBuffer->PartitionType =
                    (bootSector->bsFileSystemType[4] == '6') ? PARTITION_FAT_16 : PARTITION_FAT_12;

                outputBuffer->BootIndicator       = FALSE;
                outputBuffer->RecognizedPartition = TRUE;
                outputBuffer->RewritePartition    = FALSE;
                outputBuffer->StartingOffset.QuadPart = 0;
                outputBuffer->PartitionLength.QuadPart = devExt->DiskRegInfo.DiskSize;
                outputBuffer->HiddenSectors       = (ULONG) (1L);
                outputBuffer->PartitionNumber     = (ULONG) (-1L);

                Status = STATUS_SUCCESS;
            }
        }
        break;

    case IOCTL_DISK_GET_DRIVE_GEOMETRY:  {

            PDISK_GEOMETRY outputBuffer;

            //
            // Return the drive geometry for the ram disk. Note that
            // we return values which were made up to suit the disk size.
            //
            information = sizeof(DISK_GEOMETRY);

            Status = WdfRequestRetrieveOutputBuffer(Request, sizeof(DISK_GEOMETRY), &outputBuffer, &bufSize);
            if(NT_SUCCESS(Status) &&
               bufSize >= sizeof(DISK_GEOMETRY)) {

                RtlCopyMemory(outputBuffer, &(devExt->DiskGeometry), sizeof(DISK_GEOMETRY));
                Status = STATUS_SUCCESS;
            }
        }
        break;

    case IOCTL_DISK_CHECK_VERIFY:
    case IOCTL_DISK_IS_WRITABLE:

        //
        // Return status success
        //

        Status = STATUS_SUCCESS;
        break;

    case IOCTL_STORAGE_QUERY_PROPERTY: {

            PSTORAGE_PROPERTY_QUERY storagePropertyQuery;

            Status = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, &storagePropertyQuery, NULL);
            if (NT_SUCCESS(Status)) {
                switch (storagePropertyQuery->PropertyId) {
                case StorageDeviceProperty: {

                        PSTORAGE_DEVICE_DESCRIPTOR storageDeviceDescriptor;

                        Status = WdfRequestRetrieveOutputBuffer(Request, OutputBufferLength, &storageDeviceDescriptor, NULL);
                        if (NT_SUCCESS(Status)) {
                            RtlZeroMemory(storageDeviceDescriptor, OutputBufferLength);
                            storageDeviceDescriptor->Version = sizeof(STORAGE_DEVICE_DESCRIPTOR);
                            storageDeviceDescriptor->Size = sizeof(STORAGE_DEVICE_DESCRIPTOR);
                            storageDeviceDescriptor->DeviceType = FILE_DEVICE_DISK;
                            storageDeviceDescriptor->DeviceTypeModifier = 0;
                            storageDeviceDescriptor->RemovableMedia = TRUE;
                            storageDeviceDescriptor->CommandQueueing = FALSE;
                            storageDeviceDescriptor->VendorIdOffset = 0;
                            storageDeviceDescriptor->ProductIdOffset = 0;
                            storageDeviceDescriptor->ProductRevisionOffset = 0;
                            storageDeviceDescriptor->SerialNumberOffset = 0;
                            storageDeviceDescriptor->BusType = BusTypeVirtual;
                            storageDeviceDescriptor->RawPropertiesLength = 0;
                        }
                    }
                    break;

                default:
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }
            }
        }
        break;

    case IOCTL_DISK_GET_LENGTH_INFO: {

            PGET_LENGTH_INFORMATION lengthInfo;

            Status = WdfRequestRetrieveOutputBuffer(Request, OutputBufferLength, &lengthInfo, NULL);
            if (NT_SUCCESS(Status)) {
                lengthInfo->Length.QuadPart = DEFAULT_DISK_SIZE;
            }
        }
        break;

    case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME: {

            PMOUNTDEV_NAME name;

            Status = WdfRequestRetrieveOutputBuffer(Request, OutputBufferLength, &name, NULL);
            if (NT_SUCCESS(Status)) {
                RtlZeroMemory(name, OutputBufferLength);
                UNICODE_STRING ntDeviceName = RTL_CONSTANT_STRING(NT_DEVICE_NAME);
                name->NameLength = ntDeviceName.Length;
                RtlCopyMemory(name->Name, ntDeviceName.Buffer,name->NameLength);
            }
        }
        break;

    default:

        //
        // Display unhandled IoControlCode for future updates.
        //

        KdPrint(("RamDiskEvtIoDeviceControl: Unhandled IoControlCode 0x%x.\n", IoControlCode));
        break;
    }

    WdfRequestCompleteWithInformation(Request, Status, information);
}


/*
设备的IRP_MJ_CLOSE消息
Description:
   EvtDeviceContextCleanup event callback cleans up anything done in
   EvtDeviceAdd, except those things that are automatically cleaned
   up by the Framework.

   In the case of this sample, everything is automatically handled.  In a
   driver derived from this sample, it's quite likely that this function could
   be deleted.

parameters:
    Device - Handle to a framework device object.
*/
VOID RamDiskEvtDeviceContextCleanup(IN WDFOBJECT Device) {
    PDEVICE_EXTENSION pDeviceExtension = DeviceGetExtension(Device);
    PAGED_CODE();

    if(pDeviceExtension->DiskImage) {
        ExFreePool(pDeviceExtension->DiskImage);
    }
}


/*
类似WDM的AddDevice函数, 由Pnp管理器调用
Description:
    EvtDeviceAdd is called by the framework in response to AddDevice
    call from the PnP manager. We create and initialize a device object to
    represent a new instance of the device.

parameters:

    Driver - Handle to a framework driver object created in DriverEntry

    DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

Return Value:

    NTSTATUS

*/
NTSTATUS
RamDiskEvtDeviceAdd(IN WDFDRIVER Driver, IN PWDFDEVICE_INIT DeviceInit) {
    //将要建立的设备对象的属性描述变量
    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
    //将要调用的各种函数的状态返回值
    NTSTATUS                status;
    //将要建立的设备对象
    WDFDEVICE               device;
    //将要建立的队列对象的属性描述变量
    WDF_OBJECT_ATTRIBUTES   queueAttributes;
    //将要简历的队列配置变量
    WDF_IO_QUEUE_CONFIG     ioQueueConfig;
    //设备对象的扩展域的指针
    PDEVICE_EXTENSION       pDeviceExtension;
    //将要建立的队列扩展域指针
    PQUEUE_EXTENSION        pQueueContext = NULL;
    //将要建立的队列对象
    WDFQUEUE                queue;
    //初始化字符串
     DECLARE_CONST_UNICODE_STRING(ntDeviceName, NT_DEVICE_NAME);
    //保证这个函数可以操作paged内存
    PAGED_CODE();
    //防止产生警告
     UNREFERENCED_PARAMETER(Driver);
    //初始化设备的名字
    status = WdfDeviceInitAssignName(DeviceInit, &ntDeviceName);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    //磁盘设备的类型  必须是FILE_DEVICE_DISK
    WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_DISK);
    //设备的io类型   WdfDeviceIoBuffered 使用缓冲区接受数据  WdfDeviceIoDirect 直接接收数据 IRP所携带的缓冲区  可以直接使用
    //WdfDeviceIoBufferedOrDirect  前面两种方式   都可以使用
    WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);
    //Exclusive：独占    设置设备为非独占的
    WdfDeviceInitSetExclusive(DeviceInit, FALSE);
    //设置属性描述变量   就是说  设备对象的扩展  使用什么样的数据结构存储数据
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_EXTENSION);
    //制定设备的清理回调函数
    deviceAttributes.EvtCleanupCallback = RamDiskEvtDeviceContextCleanup;
 
    //建立这个设备
    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    //将指针指向新建立设备的设备扩展
    pDeviceExtension = DeviceGetExtension(device);
    //将队列的配置对象初始化为默认值
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE (
        &ioQueueConfig,
        WdfIoQueueDispatchSequential
        );
    //wdf中我们需要将发往自己创建的设备的请求处理函数  在队列对象的设置对象中设置
 
    //我们暂时只关心IoDeviceControl和读写事件
    ioQueueConfig.EvtIoDeviceControl = RamDiskEvtIoDeviceControl;
    ioQueueConfig.EvtIoRead          = RamDiskEvtIoRead;
    ioQueueConfig.EvtIoWrite         = RamDiskEvtIoWrite;
    //指定这个队列设备的属性描述对象
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&queueAttributes, QUEUE_EXTENSION);
 
    //创建这个队列对象  这里第一个参数是设备对象   说明这个队列在创建的时候   已经和设备绑定了
    status = WdfIoQueueCreate( device,
                               &ioQueueConfig,
                               &queueAttributes,
                               &queue );
    if (!NT_SUCCESS(status)) {
        return status;
    }
    //获取队列设备的扩展指针
    pQueueContext = QueueGetExtension(queue);
    //设置队列对象的扩展的设备对象扩展
    pQueueContext->DeviceExtension = pDeviceExtension;
 
    //设置进度条显示
    //status = SetForwardProgressOnQueue(queue);
    //if (!NT_SUCCESS(status)) {
    //    return status;
    //}
 
    //初始化设备扩展中一些变量
    pDeviceExtension->DiskRegInfo.DriveLetter.Buffer =
        (PWSTR) &pDeviceExtension->DriveLetterBuffer;
    pDeviceExtension->DiskRegInfo.DriveLetter.MaximumLength =
        sizeof(pDeviceExtension->DriveLetterBuffer);
    //从注册表中获取    这个注册表信息  其实是由inf文件指定
    RamDiskQueryDiskRegParameters(
        WdfDriverGetRegistryPath(WdfDeviceGetDriver(device)),
        &pDeviceExtension->DiskRegInfo
        );
    //分配非分页内存给这个磁盘  DiskImage是磁盘镜像的意思
    //分页内存和非分页内存   分页内存的存储介质有可能在内存  也有可能在硬盘
    //非分页内存的存储介质一定是内存    所以分配非分页内存  不会引起复杂的换页操作和一些缺页中断
    //RAMDISK_TAG 代表空间标识  便于调试
    pDeviceExtension->DiskImage = ExAllocatePoolWithTag(
        NonPagedPool,
        pDeviceExtension->DiskRegInfo.DiskSize,
        RAMDISK_TAG
        );
 
    if (pDeviceExtension->DiskImage) {
 
        UNICODE_STRING deviceName;
        UNICODE_STRING win32Name;
        //初始化磁盘空间
        RamDiskFormatDisk(pDeviceExtension);
 
        status = STATUS_SUCCESS;
 
        //    /DosDevice/xxx    windows下代表了磁盘设备
        RtlInitUnicodeString(&win32Name, DOS_DEVICE_NAME);
        RtlInitUnicodeString(&deviceName, NT_DEVICE_NAME);
 
        pDeviceExtension->SymbolicLink.Buffer = (PWSTR)
            &pDeviceExtension->DosDeviceNameBuffer;
        pDeviceExtension->SymbolicLink.MaximumLength =
            sizeof(pDeviceExtension->DosDeviceNameBuffer);
        pDeviceExtension->SymbolicLink.Length = win32Name.Length;
 
        RtlCopyUnicodeString(&pDeviceExtension->SymbolicLink, &win32Name);
        //初始化磁盘盘符
        RtlAppendUnicodeStringToString(&pDeviceExtension->SymbolicLink,
                                       &pDeviceExtension->DiskRegInfo.DriveLetter);
 
        status = WdfDeviceCreateSymbolicLink(device,
                                             &pDeviceExtension->SymbolicLink);
    }
    return status;
}


/*
从注册表中读取磁盘的配置信息

Description:
    This routine is called from the DriverEntry to get the debug
    parameters from the registry. If the registry query fails, then
    default values are used.

parameters:
    RegistryPath    - Points the service path to get the registry parameters
*/
VOID RamDiskQueryDiskRegParameters(_In_ PWSTR RegistryPath, 
                                    _In_ PDISK_INFO DiskRegInfo) {
    RTL_QUERY_REGISTRY_TABLE rtlQueryRegTbl[5 + 1];  // Need 1 for NULL
    NTSTATUS                 Status;
    DISK_INFO                defDiskRegInfo;

    PAGED_CODE();

    ASSERT(RegistryPath != NULL);

    // Set the default values

    defDiskRegInfo.DiskSize          = DEFAULT_DISK_SIZE;
    defDiskRegInfo.RootDirEntries    = DEFAULT_ROOT_DIR_ENTRIES;
    defDiskRegInfo.SectorsPerCluster = DEFAULT_SECTORS_PER_CLUSTER;

    RtlInitUnicodeString(&defDiskRegInfo.DriveLetter, DEFAULT_DRIVE_LETTER);

    RtlZeroMemory(rtlQueryRegTbl, sizeof(rtlQueryRegTbl));

    //
    // Setup the query table
    //

    rtlQueryRegTbl[0].Flags         = RTL_QUERY_REGISTRY_SUBKEY;
    rtlQueryRegTbl[0].Name          = L"Parameters";
    rtlQueryRegTbl[0].EntryContext  = NULL;
    rtlQueryRegTbl[0].DefaultType   = (ULONG_PTR)NULL;
    rtlQueryRegTbl[0].DefaultData   = NULL;
    rtlQueryRegTbl[0].DefaultLength = (ULONG_PTR)NULL;

    //
    // Disk paramters
    //

    rtlQueryRegTbl[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[1].Name          = L"DiskSize";
    rtlQueryRegTbl[1].EntryContext  = &DiskRegInfo->DiskSize;
    rtlQueryRegTbl[1].DefaultType   = REG_DWORD;
    rtlQueryRegTbl[1].DefaultData   = &defDiskRegInfo.DiskSize;
    rtlQueryRegTbl[1].DefaultLength = sizeof(ULONG);

    rtlQueryRegTbl[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[2].Name          = L"RootDirEntries";
    rtlQueryRegTbl[2].EntryContext  = &DiskRegInfo->RootDirEntries;
    rtlQueryRegTbl[2].DefaultType   = REG_DWORD;
    rtlQueryRegTbl[2].DefaultData   = &defDiskRegInfo.RootDirEntries;
    rtlQueryRegTbl[2].DefaultLength = sizeof(ULONG);

    rtlQueryRegTbl[3].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[3].Name          = L"SectorsPerCluster";
    rtlQueryRegTbl[3].EntryContext  = &DiskRegInfo->SectorsPerCluster;
    rtlQueryRegTbl[3].DefaultType   = REG_DWORD;
    rtlQueryRegTbl[3].DefaultData   = &defDiskRegInfo.SectorsPerCluster;
    rtlQueryRegTbl[3].DefaultLength = sizeof(ULONG);

    rtlQueryRegTbl[4].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[4].Name          = L"DriveLetter";
    rtlQueryRegTbl[4].EntryContext  = &DiskRegInfo->DriveLetter;
    rtlQueryRegTbl[4].DefaultType   = REG_SZ;
    rtlQueryRegTbl[4].DefaultData   = defDiskRegInfo.DriveLetter.Buffer;
    rtlQueryRegTbl[4].DefaultLength = 0;


    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                 RegistryPath,
                 rtlQueryRegTbl,
                 NULL,
                 NULL
             );

    if (NT_SUCCESS(Status) == FALSE) {

        DiskRegInfo->DiskSize          = defDiskRegInfo.DiskSize;
        DiskRegInfo->RootDirEntries    = defDiskRegInfo.RootDirEntries;
        DiskRegInfo->SectorsPerCluster = defDiskRegInfo.SectorsPerCluster;
        RtlCopyUnicodeString(&DiskRegInfo->DriveLetter, &defDiskRegInfo.DriveLetter);
    }

    KdPrint(("DiskSize          = 0x%lx\n", DiskRegInfo->DiskSize));
    KdPrint(("RootDirEntries    = 0x%lx\n", DiskRegInfo->RootDirEntries));
    KdPrint(("SectorsPerCluster = 0x%lx\n", DiskRegInfo->SectorsPerCluster));
    KdPrint(("DriveLetter       = %wZ\n",   &(DiskRegInfo->DriveLetter)));

    return;
}


/*
初始化磁盘结构的一些数据,即格式化一个磁盘

Routine Description:

    This routine formats the new disk.


parameters:

    DeviceObject - Supplies a pointer to the device object that represents
                   the device whose capacity is to be read.

Return Value:

    status is returned.
*/
NTSTATUS RamDiskFormatDisk(IN PDEVICE_EXTENSION devExt) {
    //将分配的非分页内存的首地址  转化为DBR结构的指针
    PBOOT_SECTOR bootSector = (PBOOT_SECTOR) devExt->DiskImage;
    //指向FAT表的指针
    PUCHAR       firstFatSector;
    //根目录入口点个数
    ULONG        rootDirEntries;
    //一个簇由多少扇区组成
    ULONG        sectorsPerCluster;
    //fat文件系统类型
    USHORT       fatType;        // Type FAT 12 or 16
    //FAT表里面有多少表项
    USHORT       fatEntries;     // Number of cluster entries in FAT
    //一个FAT表需要占多少簇存储
    USHORT       fatSectorCnt;   // Number of sectors for FAT
    //第一个根目录入口点
    PDIR_ENTRY   rootDir;        // Pointer to first entry in root dir
    //保证这个函数可以操作paged内存
    PAGED_CODE();
 
    ASSERT();
    ASSERT(devExt->DiskImage != NULL);
    //清空磁盘镜像
    RtlZeroMemory(devExt->DiskImage, devExt->DiskRegInfo.DiskSize);
    //每个扇区有512个字节
    devExt->DiskGeometry.BytesPerSector = ;
    //每个磁道有32个扇区
    devExt->DiskGeometry.SectorsPerTrack = ;     // Using Ramdisk value
    //每个柱面有两个磁道
    devExt->DiskGeometry.TracksPerCylinder = ;    // Using Ramdisk value
    //计算得出磁柱面数
    devExt->DiskGeometry.Cylinders.QuadPart = devExt->DiskRegInfo.DiskSize /  /  / ;
    devExt->DiskGeometry.MediaType = RAMDISK_MEDIA_TYPE;
    KdPrint((
        "Cylinders: %ld\n TracksPerCylinder: %ld\n SectorsPerTrack: %ld\n BytesPerSector: %ld\n",
        devExt->DiskGeometry.Cylinders.QuadPart, devExt->DiskGeometry.TracksPerCylinder,
        devExt->DiskGeometry.SectorsPerTrack, devExt->DiskGeometry.BytesPerSector
        ));
    //初始化根目录入口点个数
    rootDirEntries = devExt->DiskRegInfo.RootDirEntries;
    //一个簇由多少扇区组成
    sectorsPerCluster = devExt->DiskRegInfo.SectorsPerCluster;
    //这里  调整根目录数目的地方很疑惑
    )) {
 
        rootDirEntries =
            (rootDirEntries + (DIR_ENTRIES_PER_SECTOR - )) &
                ~ (DIR_ENTRIES_PER_SECTOR - );
    }
 
    KdPrint((
        "Root dir entries: %ld\n Sectors/cluster: %ld\n",
        rootDirEntries, sectorsPerCluster
        ));
    //硬编码写入跳转指令
    bootSector->bsJump[] = 0xeb;
    bootSector->bsJump[] = 0x3c;
    bootSector->bsJump[] = 0x90;
    //oem名字
        bootSector->bsOemName[] = 'R';
    bootSector->bsOemName[] = 'a';
    bootSector->bsOemName[] = 'j';
    bootSector->bsOemName[] = 'u';
    bootSector->bsOemName[] = 'R';
    bootSector->bsOemName[] = 'a';
    bootSector->bsOemName[] = 'm';
    bootSector->bsOemName[] = ' ';
    //每个扇区有多少字节
    bootSector->bsBytesPerSec = (SHORT)devExt->DiskGeometry.BytesPerSector;
    //指定这个磁盘卷保留扇区   仅DBR这一个扇区为保留扇区
    bootSector->bsResSectors  = ;
    //fat表一般一式两份  但这里就创建一份就可以
    bootSector->bsFATs        = ;
    //指定根目录入口点个数
     bootSector->bsRootDirEnts = (USHORT)rootDirEntries;
     //磁盘总扇区数   磁盘大小除以扇区大小
    bootSector->bsSectors     = (USHORT)(devExt->DiskRegInfo.DiskSize /
                                         devExt->DiskGeometry.BytesPerSector);
    //磁盘介质类型
    bootSector->bsMedia       = (UCHAR)devExt->DiskGeometry.MediaType;
    //每个簇有多少个扇区
    bootSector->bsSecPerClus  = (UCHAR)sectorsPerCluster;
    //fat表项数  总扇区数-保留扇区数
    //这里很疑惑   如何得到FAT表项数
    fatEntries =
        (bootSector->bsSectors - bootSector->bsResSectors -
            bootSector->bsRootDirEnts / DIR_ENTRIES_PER_SECTOR) /
                bootSector->bsSecPerClus + ;
 
    //如果表项数  大于2的12次方   则使用FAT16
    ) {
        fatType =  ;
        //这一步的调整  我很不理解
        fatSectorCnt = (fatEntries *  + ) / ;
        fatEntries   = fatEntries + fatSectorCnt;
        fatSectorCnt = (fatEntries *  + ) / ;
    }
    else {
        fatType =  ;
        fatSectorCnt = (((fatEntries *  + ) / ) + ) / ;
        fatEntries   = fatEntries + fatSectorCnt;
        fatSectorCnt = (((fatEntries *  + ) / ) + ) / ;
    }
    //初始化FAT表所占的扇区数
    bootSector->bsFATsecs       = fatSectorCnt;
    //初始化DBR每个磁道的扇区数
    bootSector->bsSecPerTrack   = (USHORT)devExt->DiskGeometry.SectorsPerTrack;
    //初始化每个柱面的磁道数
    bootSector->bsHeads         = (USHORT)devExt->DiskGeometry.TracksPerCylinder;
    //初始化启动签名  windows要求必须是0x29或者0x28
    bootSector->bsBootSignature = 0x29;
    //卷ID  随便填写
    bootSector->bsVolumeID      = 0x12345678;
    //初始化卷标
        bootSector->bsLabel[]  = 'R';
    bootSector->bsLabel[]  = 'a';
    bootSector->bsLabel[]  = 'm';
    bootSector->bsLabel[]  = 'D';
    bootSector->bsLabel[]  = 'i';
    bootSector->bsLabel[]  = 's';
    bootSector->bsLabel[]  = 'k';
    bootSector->bsLabel[]  = ' ';
    bootSector->bsLabel[]  = ' ';
    bootSector->bsLabel[]  = ' ';
    bootSector->bsLabel[] = ' ';
    //设置磁盘文件类型
    bootSector->bsFileSystemType[] = 'F';
    bootSector->bsFileSystemType[] = 'A';
    bootSector->bsFileSystemType[] = 'T';
    bootSector->bsFileSystemType[] = ';
    bootSector->bsFileSystemType[] = '?';
    bootSector->bsFileSystemType[] = ' ';
    bootSector->bsFileSystemType[] = ' ';
    bootSector->bsFileSystemType[] = ' ';
 
    bootSector->bsFileSystemType[] = ( fatType ==  ) ? ';
    //设置DBR结束标识
    bootSector->bsSig2[] = 0x55;
    bootSector->bsSig2[] = 0xAA;
    //初始化FAT表结构
    firstFatSector    = (PUCHAR)(bootSector + );
    firstFatSector[] = (UCHAR)devExt->DiskGeometry.MediaType;
    firstFatSector[] = 0xFF;
    firstFatSector[] = 0xFF;
 
    ) {
        firstFatSector[] = 0xFF;
    }
 
     rootDir = (PDIR_ENTRY)(bootSector +  + fatSectorCnt);
 
    //接下来 初始化根目录入口点信息
    rootDir->deName[] = 'M';
    rootDir->deName[] = 'S';
    rootDir->deName[] = '-';
    rootDir->deName[] = 'R';
    rootDir->deName[] = 'A';
    rootDir->deName[] = 'M';
    rootDir->deName[] = 'D';
    rootDir->deName[] = 'R';
 
    //
    // Set device extension name to "IVE"
    // NOTE: Fill all 3 characters, eg. sizeof(rootDir->deExtension);
    //
    rootDir->deExtension[] = 'I';
    rootDir->deExtension[] = 'V';
    rootDir->deExtension[] = 'E';
 
    rootDir->deAttributes = DIR_ATTR_VOLUME;
 
    return STATUS_SUCCESS;
}

/*
磁盘传递的参数检测
*/
BOOLEAN
RamDiskCheckParameters(IN PDEVICE_EXTENSION devExt,
                        IN LARGE_INTEGER ByteOffset, IN size_t Length) {
    if( devExt->DiskRegInfo.DiskSize < Length ||
        ByteOffset.QuadPart < 0 || // QuadPart is signed so check for negative values
        ((ULONGLONG)ByteOffset.QuadPart > (devExt->DiskRegInfo.DiskSize - Length)) ||
            (Length & (devExt->DiskGeometry.BytesPerSector - 1))) {

        //
        // Do not give an I/O boost for parameter errors.
        //

        KdPrint((
            "Error invalid parameter\n"
            "ByteOffset: %I64x\n"
            "Length: %lu\n",
            ByteOffset.QuadPart,
            Length
         ));

        return FALSE;
    }

    return TRUE;
}

