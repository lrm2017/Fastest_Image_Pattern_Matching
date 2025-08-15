#pragma once

#include "include.h"
#include "public.h"
#include "regs.h"
/* linux spinlock_t windows WDFSPINLOCK */
//#define WDFSPINLOCK (void*)

#define VER_A   1
#define VER_B   0
#define VER_C   0
#define VER_D   0

#define MAX_DEVICE_NUM 			64

#define DEV_NAME "dt_pcie"

//一个小buffer的大小
#define DMA_BUFFER_SIZE (2*1024*1024)

#define DISCO_TIMEOUT 500

#define RtlMoveMemory dsmemmove
#define RtlZeroMemory(x, y)  dsmemset( (x), 0, (y))
#define TraceEvents(x, y, fmt...) dsprint(fmt)

/* 如何把字符设备和PCIE采集卡对应起来 多个采集卡要区分一下 */

#if 1 //for camera
#define RD_REG(x) dt_pcie_read(fdoData->IoBaseAddress, x) 
#define WR_REG(x, v) dt_pcie_write(fdoData->IoBaseAddress, x, v) 

#define RD_REG_CH(x, ch) dt_pcie_read(fdoData->IoBaseAddress, (x+0x100*(ch)) ) 
#define WR_REG_CH(x, ch, v) dt_pcie_write(fdoData->IoBaseAddress, (x+0x100*(ch)), v) 

#define RD_REG_CH_BLK(x, ch, blk) dt_pcie_read(fdoData->IoBaseAddress, x+0x100*(ch)+8*(blk)) 
#define WR_REG_CH_BLK(x, ch, blk, v) dt_pcie_write(fdoData->IoBaseAddress, x+0x100*(ch)+8*(blk), v) 
#else //for testbox
#define RD_REG(x) dt_pcie_read(fdoData->IoBaseAddress, x) 
#define WR_REG(x, v) dt_pcie_write(fdoData->IoBaseAddress, x, v) 
#define RD_REG_CH(x, ch)	  RD_REG((x+(ch<<8)))
#define WR_REG_CH(x, ch, v)	   WR_REG((x+(ch<<8)), v)
#define RD_REG_CH_BLK(x, ch, blk)     RD_REG(x+(ch<<8)+(blk<<3))
#define WR_REG_CH_BLK(x, ch, blk, v) WR_REG((x+(ch<<8)+(blk<<3)), v)
#endif


// DMA BLOCK控制
// 不能一次申请很大的空间，分成很多小块申请
typedef struct DtDmaBlockControl_s
{
	//PMDL				mdl;			// 保护非连续page的物理内存信息
    ULONG               cnt;            // 申请了多少个小块内存 
	//dma_addr_t  		mdl[256];       // 供 PCIE DMA使用  pci_bus address
	u64  				mdl[256];       // 供 PCIE DMA使用  pci_bus address
	PVOID				sysAddr[256];   // 映射的内核虚拟地址  供内核访问
	PVOID				usrAddr;		// 映射的用户虚拟地址  用户程序的地址
	ULONG				tabStart;		// 在地址转换表上对应的起始位置
	ULONG				tabEnd;			// 在地址转换表上对应的结束位置
    
	ULONG				dma_len;		// dma长度
}DtDmaBlockControl_t;

// 通道控制
typedef struct DtChannelControl_s 
{
	BOOLEAN				bAvailable;			// 是否可用
	BOOLEAN				bInUse;				// 是否正在被使用（已经被应用软件打开）
	USHORT				RmtAddr;			// 远端设备地址
	UCHAR				RmtPort;			// 远端设备端口
	ULONG				xxx;				// 用作对齐分隔
	UCHAR				RmtEthAddr[6];		// Xg模式下，远端设备的MAC地址
	UCHAR				LocalEthAddr[6];	// Xg模式下，本地的MAC地址

    /* linux add */
    uint32_t            uSendPackLen;       /* 应用程序发送的数据长度 */
    uint32_t            uRecvPackLen;       /* 应用程序接收的数据长度 */
    uint8_t             CmdBuf[576];
   // wait_queue_head_t   CmdQueue;           /* 收命令包用来等待同步接收回应 */
   	void*   			CmdQueue;           /* 收命令包用来等待同步接收回应 */
    int32_t             CmdQueueFlag;       /* 配合CmdQueue一起使用 */
    //wait_queue_head_t   ImageQueue;         /* 收帧 用来等待同步接收回应 */
    void*   			ImageQueue;         /* 收帧 用来等待同步接收回应 */
    int32_t             ImageQueueFlag;     /* 配合ImageQueue一起使用 */

	//WDFSPINLOCK			lock;	
//	WDFQUEUE			rcvQueue;
//	WDFSPINLOCK			grabLock;
//	WDFSPINLOCK			cmdLock;
//	WDFQUEUE			grabQueue;

	void*				lock;
	void*				grabLock;
	void*				cmdLock;

	BOOLEAN				grabInitOk;
	BOOLEAN				grabStarted;
	BOOLEAN				grabMemOk;			// 用DMA采集的内存是否有效
	ULONG				blockNumAlloc;		// 内存分配对应的块数
	ULONG				blockSizeAlloc;		// 内存分配对应的块大小
	ULONG				blockNum;			// 实际使用的块数
	ULONG				blockSize;			// 实际使用的块大小
	ULONG				nextBlockID;		// 下一次将输出的BlockID
	DtDmaBlockControl_t	blocks[MAX_BLOCK_NUM];
	ULONG				bitmapInUse;
	DtChGrabStatus_t	grabStatus;
	DtChLinkStatus_t	linkStatus;
	ULONG				cmdRecvStatus;		
	ULONG				resv[126];
}DtChannelControl_t;

//
// The device extension for the device object
//
typedef struct _FDO_DATA
{
    ULONG                   Signature;       // must be PCIDRV_FDO_INSTANCE_SIGNATURE
                                             // beneath this device object.
    //WDFDEVICE               WdfDevice;

    // Power Management
    //WDF_POWER_DEVICE_STATE  DevicePowerState;   // Current power state of the device(D0 - D3)

    // Wait-Wake
    BOOLEAN                 AllowWakeArming;

	// Following fields are specific to the hardware
	// Configuration
	BOOLEAN                 MWIEnable;          // Memory Write Invalidate bit in the PCI command word
    
	// IDs
    UCHAR                   RevsionID;
    USHORT                  SubVendorID;
    USHORT                  SubSystemID;

    // HW Resources
    //BUS_INTERFACE_STANDARD  BusInterface;
    PUCHAR                  IoBaseAddress;
    ULONG                   IoRange;
    //PHYSICAL_ADDRESS        MemPhysAddress;
    //WDFINTERRUPT            WdfInterrupt;

    //WDFDMAENABLER           WdfDmaEnabler;

//    WDFQUEUE                WriteQueue;
//    WDFQUEUE                PendingWriteQueue;
//	WDFQUEUE                PendingReadQueue;

	
	//WDFSPINLOCK             RcvLock;
	//WDFSPINLOCK				SendLock;

    // spin locks for protecting misc variables
    //WDFSPINLOCK				Lock;

	void*             		RcvLock;
	void*					SendLock;

    // spin locks for protecting misc variables
    void*					Lock;
	

    // WatchDog timer related fields
    //WDFTIMER                WatchDogTimer;

    // For handling IOCTLs
//    WDFQUEUE                IoctlQueue;

	// 标识是否正在进行DPC处理
	LONG					DpcProc;

	// RecvEnable接收包使能
	BOOLEAN					RecvEnable;

	// 预分配参数
	DtPreAlloc_t			preAlloc;

	// 设备信息
	DtPcieDevInfo_t			DevInfo;

	// 最大预先准备通道数，可能会使用到这么多个的采集通道
	ULONG					MaxChPre;

	// PhyLink
	DtPhyLinkInfo_t			PhyLinks[MAX_PHY_LINK_NUM];

	// XG PhyLink
	DtXgPhyLinkInfo_t		XgPhyLinks[MAX_PHY_LINK_NUM];
	
	// 通道控制
	DtChannelControl_t		Channels[MAX_CH_NUM];

	// 版本信息
	DtVersionInfo_t			VerInfo;

	// Xg模式使能
	BOOLEAN					bXgMode;

	// 发现已连接的Xg设备数
	ULONG					XgCount;

	// Xg链路信息
	DtXgLinkInfo_t			XgLinks[MAX_XG_LINK_NUM];
}  FDO_DATA, *PFDO_DATA;

#if 0
typedef struct _dtpcie_info
{
    uint32_t               version[4];
    int32_t                uChID;         /* 当前的申请的通道 -1表示还没有申请、打开 */
    uint32_t               uCurBlockId;   /* 当前映射内存的id,配置的时候使用 */
    void                   *io_addr;      /* bar0 virtual add */
    //struct pci_dev         *pdev;
	void				   *pdev;
	PFDO_DATA              m_fdoData;     /* 对应一个采集卡的状态 */
  	//struct work_struct     grabImage_work;
	void				   *m_pGrabImageWork;
	//BOOLEAN				   m_bOK;
}dtpcie_info;
#endif


#if 0
void DtMacSoftReset(PFDO_DATA fdoData);
int DtEvtInterruptEnable (PFDO_DATA fdoData);
int DtEvtInterruptDisable(PFDO_DATA fdoData);
int DtChannelGrabStop(PFDO_DATA fdoData, ULONG uChID, dtpcie_info *dt_pcie);
int DtChannelGrabStart(PFDO_DATA fdoData, ULONG uChID, dtpcie_info *dt_pcie);
int DtChannelGrabUninit(PFDO_DATA fdoData, ULONG uChID, BOOLEAN bFreeMem, dtpcie_info *dt_pcie);
int DtDmaMemAlloc(PFDO_DATA fdoData, ULONG uChID, ULONG blockNum, ULONG blockSize, dtpcie_info *dt_pcie);
int DtFreeChannel(ULONG uChID, PFDO_DATA fdoData, dtpcie_info *dt_pcie);

void DtHandlePackRecv(PFDO_DATA fdoData);
//void DtHandleGrabImage(PFDO_DATA fdoData, ULONG uChID);

int DtReadAdapterInfo(PFDO_DATA fdoData);

int DtSendArpReq(PFDO_DATA fdoData, DtXgLinkInfo_t *pXgLink);

//void grab_image_work(struct work_struct *work);
//irqreturn_t dtpcie_interrupt (int irq, void *dev_instance);

void DtWatchDogEvtTimerFunc(PFDO_DATA fdoData);
int DtDevUpdateThread(void *pContext);

uint32_t dt_pcie_read(void *iomem, uint32_t reg);
void dt_pcie_write(void *iomem, uint32_t reg, uint32_t val);
#endif

