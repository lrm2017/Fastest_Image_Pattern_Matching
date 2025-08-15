#pragma once

/*++
    Copyright (c) DOTHINKEY Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid for toaster device class.
// This GUID is used to register (IoRegisterDeviceInterface)
// an instance of an interface so that user application
// can control the toaster device.
//
#ifdef _WIN32
DEFINE_GUID (GUID_DEVINTERFACE_DTPCIE,
    0xb74cfec2, 0x9366, 0x454a, 0xba, 0x71, 0x7c, 0x27, 0xb5, 0x14, 0x70, 0xa4);
// {B74CFEC2-9366-454a-BA71-7C27B51470A4}

//
// Define a WMI GUID to get toaster device info.
//

DEFINE_GUID (PCIDRV_WMI_STD_DATA_GUID,
    0x20e35e40, 0x7179, 0x4f89, 0xa2, 0x8c, 0x12, 0xed, 0x5a, 0x3c, 0xaa, 0xa5);
#endif
// {20E35E40-7179-4f89-A28C-12ED5A3CAAA5}

//
// GUID definition are required to be outside of header inclusion pragma to avoid
// error during precompiled headers.
//


#define FILE_DEVICE_DT_PCIE  0x8000
#ifdef _WIN32
#define DT_PCIE_IOCTL(index) \
    CTL_CODE(FILE_DEVICE_DT_PCIE, index, METHOD_BUFFERED, FILE_READ_DATA)

// 查询设备信息
#define IOCTL_DEV_INFO \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 查询Link通道信息，用LinkPort索引
#define IOCTL_PHY_LINK_INFO \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 分配采集通道并绑定到设备文件
#define IOCTL_ALLOC_CHANNEL \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 释放并解除通道绑定
#define IOCTL_FREE_CHANNEL \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 发送包
#define IOCTL_PACK_SEND \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 接收包
#define IOCTL_PACK_RECV \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 版本信息
#define IOCTL_VERSION \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x80E, METHOD_BUFFERED, FILE_ANY_ACCESS)

// LINK状态查询
#define IOCTL_CHANNEL_LINK_STATUS \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x80f, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 采集器初始化 
#define IOCTL_CHANNEL_GRAB_INIT \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 采集器反初始化 
#define IOCTL_CHANNEL_GRAB_UNINIT \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x811, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 采集器初始化 
#define IOCTL_CHANNEL_GRAB_START \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x812, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 采集器反初始化 
#define IOCTL_CHANNEL_GRAB_STOP \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x813, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 采集一帧
#define IOCTL_CHANNEL_GRAB_IMAGE \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x818, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 释放采集的一帧
#define IOCTL_CHANNEL_GRAB_RELEASE \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x819, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 通知预先准备通道数
#define IOCTL_CHANNEL_PREPARE \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x81E, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 释放状态信息
#define IOCTL_CHANNEL_GRAB_STATUS \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x81f, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 预分配查询
#define IOCTL_PRE_ALLOC_INQ \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x840, METHOD_BUFFERED, FILE_ANY_ACCESS)

// 预分配设置
#define IOCTL_PRE_ALLOC_SET \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x841, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PCIE_RW \
    CTL_CODE(FILE_DEVICE_DT_PCIE, 0x880, METHOD_BUFFERED, FILE_ANY_ACCESS)
#else

#include <asm-generic/ioctl.h>

//#define DT_PCIE_IOCTL(index)          _IOR('A', index, int)

#define IOCTL_DEV_INFO                _IOWR('B', 0, DtPcieDevInfo_t)

// 查询Link通道信息，用LinkPort索引 长度有待确认tan
#define IOCTL_PHY_LINK_INFO           _IOWR('C', 0, DtPhyLinkInfo_t)
#define IOCTL_PHY_LINK_INFO_OLD		  _IOWR('C', 0, DtPhyLinkInfoOld_t)


#define IOCTL_XG_LINK_INFO            _IOWR('C', 1, DtXgLinkInfo_t)

// 分配采集通道并绑定到设备文件
#define IOCTL_ALLOC_CHANNEL           _IOWR('D', 0, DtChLinkInfo_t)

#define IOCTL_ALLOC_XG_CHANNEL        _IOWR('D', 1, DtXgChLinkInfo_t)

// 释放并解除通道绑定
#define IOCTL_FREE_CHANNEL            _IOWR('D', 2, int)

// 发送包 长度有待确认tan
#define IOCTL_PACK_SEND               _IOWR('E', 0, int)
#define IOCTL_PACK_SEND_LEN           _IOWR('E', 1, unsigned int)
#define IOCTL_PACK_RECV_LEN           _IOWR('E', 2, unsigned int)

// 接收包 长度有待确认tan
#define IOCTL_PACK_RECV               _IOWR('E', 3, int)

// 版本信息 长度有待确认tan
#define IOCTL_VERSION                 _IOWR('F', 0, DtVersionInfo_t)

// LINK状态查询
#define IOCTL_CHANNEL_LINK_STATUS     _IOWR('G', 0, DtChLinkStatus_t)

// 采集器初始化 不需要传出数据
#define IOCTL_CHANNEL_GRAB_INIT       _IOWR('H', 0, DtGrabConfig_t)

// 采集器反初始化 不需要传出数据
#define IOCTL_CHANNEL_GRAB_UNINIT     _IOWR('H', 1, int)

// 采集器初始化     不需要传出数据
#define IOCTL_CHANNEL_GRAB_START      _IOWR('I', 0, int)

// 采集器反初始化 不需要传出数据
#define IOCTL_CHANNEL_GRAB_STOP       _IOWR('I', 1, int)

// 采集器暂停采集      长度有待确认
#define IOCTL_CHANNEL_GRAB_HOLD		  _IOWR('I', 2, unsigned int)

// 采集器重启采集      长度有待确认
#define IOCTL_CHANNEL_GRAB_RESTART	  _IOWR('I', 3, unsigned int)

// 采集一帧
#define IOCTL_CHANNEL_GRAB_IMAGE      _IOWR('J', 0, DtGrabImage_t)

// 释放采集的一帧
#define IOCTL_CHANNEL_GRAB_RELEASE    _IOWR('J', 1, int)

// 通知预先准备通道数       长度有待确认tan
#define IOCTL_CHANNEL_PREPARE         _IOWR('K', 0, int)

// 释放状态信息
#define IOCTL_CHANNEL_GRAB_STATUS     _IOWR('L', 0, DtChGrabStatus_t)

// 预分配查询 长度有待确认tan
#define IOCTL_PRE_ALLOC_INQ           _IOWR('M', 0, DtPreAlloc_t)

// 预分配查询 长度有待确认tan
#define IOCTL_PRE_ALLOC_SET           _IOWR('M', 1, DtPreAlloc_t)

#define IOCTL_PCIE_RW                 _IOWR('N', 0, int)

#define IOCTL_SET_MAPMEM              _IOWR('O', 0, unsigned int)

#define IOCTL_CHANNEL_CMD_STATUS	  _IOWR('P', 0, unsigned int)

// 复位，可以复位部分模块，由控制位指定
#define IOCTL_RESET  				  _IOWR('Q', 0, unsigned long)

// 获取LOG文本
#define IOCTL_LOG_TEXT  			  _IOWR('R', 0, unsigned int)

// 请求一帧
#define IOCTL_CHANNEL_GRAB_REQ		  _IOWR('S', 0, unsigned int)

#endif

// 支持最大PhyLink数(光纤/光口数量)
#define MAX_PHY_LINK_NUM	4	
// 支持最大的Xg设备数

#define MAX_XG_LINK_NUM		16

// 支持最大通道数（采集通道）
#define MAX_CH_NUM			16

#define MAX_MEM_BLOCK_NUM	64

// 无效的通道ID
#define INVALID_CH_ID		0xffffffff

// 最大块数
#define MAX_BLOCK_NUM		8

// 小块连续区域大小	,必须是4096的整数倍		
//#define CONTIG_SIZE			4096
#define CONTIG_SIZE			0x200000

// 物理地址转换表大小
#define PAT_SIZE			256	

// 传输包中，大端模式的MAC地址转换
#define MAC_ADDR(a)			((USHORT)(a[0]<<8) | a[1])

#define LOG_TEXT_BUF_SIZE		0x80000

// 传输包中，协议部分
#define MAC_PROTOCOL(p)		((p>>4)&0xf)	
#define MAC_PROT_CMD		0	// 控制命令
#define MAC_PROT_STREAM		1	// 数据流
#define MAC_PROT_EVENT		2	// 事件
#define MAC_PROT_RESEND		3	// 重传
#define MAC_PROT_FRAME_REQ  4   // 请求帧
#define MAC_PROT_DEBUG		12	// 调试信息
#define MAC_PROT_FLOW_CTRL  13  // 包流控
#define MAC_PROT_DISCOVERY	14	// DISCOVERY
#define MAC_PROT_NOP		15	// 空信息

// IOCTL_RESET命令中的控制位定义
#define IOCTL_RESET_MAC			(1<<31)	// 对MAC复位
#define IOCTL_RESET_PHY_MASK	0xff	// 对PHY通道复位

// MAC帧头部信息
typedef  struct   DtMacHdr_s {
	UCHAR			DstAddr[2];			//  目标MAC，MSB 
	UCHAR			SrcAddr[2];			//  源MAC，MSB
	UCHAR			DstPort;			//  目标端口
	UCHAR			SrcPort;			//  源端口
	UCHAR			Protocol;			//  协议
	UCHAR			Ctrl;				//  包控制相关信息  
	UCHAR			DstLinkPort;		//  目标光口
	UCHAR			SrcLinkPort;		//  源光口
//	USHORT		    size;				//  Linux驱动使用 表示传输字节数
	UCHAR			Res[6];				//  保留6字节
}DtMacHdr_t;

#define MAC_HDR_SIZE	sizeof(DtMacHdr_t)

typedef struct _GvcpDiscoveryAck {
    unsigned char SpecVersionMajor[2];
    unsigned char SpecVersionMinor[2];
    unsigned char DeviceMode[4];
    unsigned char reserved0[2];
    unsigned char MacAddr[6];
    unsigned char IpConfigOptions[4];
    unsigned char IpConfigCurrent[4];
    unsigned char reserved1[12];
    unsigned char CurrentIP[4];
    unsigned char reserved2[12];
    unsigned char CurrentNetMask[4];
    unsigned char reserved3[12];
    unsigned char DefaultGateway[4];
    unsigned char ManufactureName[32];
    unsigned char ModelName[32];
    unsigned char DeviceVersion[32];
    unsigned char ManufacturerSpecInfo[48];
    unsigned char SerialNumber[16];
    unsigned char UserDefinedName[16];
}GvcpDiscoveryAck;

typedef struct DtDevicePhyLinkInfo_s
{
    uint32_t        RxPause;
    uint32_t        RxCrcErr;
    uint32_t        RxHdrErr;
    uint32_t        RxCmdDrop;
    uint32_t        RxCmdPack;
    uint32_t        LinkSta;
    uint32_t        TxCmdPack;
    uint32_t        Rsv[17];
}DtDevicePhyLinkInfo_t;

// DiscoveryPacket
typedef struct DtDiscoveryInfo_s {
    DtMacHdr_t      MacHdr;				// 头部信息
    UCHAR           DevAddr[2];			// 设备的MAC地址
    UCHAR           LinkNum;			// Link通道数量
    UCHAR           LinkSurpport;		// Link通道支持(状态位，如12.5G,10G,5G，支持的选项对应一个bit)		
    UCHAR           LinkConfig;			// Link配置（选项值）
    UCHAR           UserIdSwitch;		// 用户ID拨码开关控制
    UCHAR           Res;				// 保留
    UCHAR           SsrPortNum;			// sensor端口数
    UINT            SsrPortStatus;		// sensor端口被使用情况
    char            Model[16];			// 机型名称
    char            Sn[16];				// 序列号
    char            UserDefName[32];	// 用户命名
    char            DevInfo[64];		// 设备信息，包含：硬件版本、固件版本等信息
    uint32_t        Rsv1[3];
    uint32_t        InfoFunC;           // 结构体标志
    uint32_t        ChipTemp;           // 盒子温度信息   
    uint32_t        Rsv2[20];           // 保留，
                                        // 前面总共256字节
    DtDevicePhyLinkInfo_t PhyLinkInfo[4];
}DtDiscoveryInfo_t;

// DiscoveryPacket
typedef struct DtDiscoveryInfoOld_s {
    DtMacHdr_t      MacHdr;				// 头部信息
    UCHAR           DevAddr[2];			// 设备的MAC地址
    UCHAR           LinkNum;			// Link通道数量
    UCHAR           LinkSurpport;		// Link通道支持(状态位，如12.5G,10G,5G，支持的选项对应一个bit)		
    UCHAR           LinkConfig;			// Link配置（选项值）
    UCHAR           UserIdSwitch;		// 用户ID拨码开关控制
    UCHAR           Res;				// 保留
    UCHAR           SsrPortNum;			// sensor端口数
    UINT            SsrPortStatus;		// sensor端口被使用情况
    char            Model[16];			// 机型名称
    char            Sn[16];				// 序列号
    char            UserDefName[32];	// 用户命名
    char            DevInfo[64];		// 设备信息，包含：硬件版本、固件版本等信息
}DtDiscoveryInfoOld_t;

// 物理光口状态信息
typedef struct DtPhyLinkInfoOld_s
{
	BOOLEAN			bAvailable;			// 是否可用
	BOOLEAN			bConnected;			// 是否已经连接（信号链路连接）
	BOOLEAN			bFound;				// 是否有设备被找到
	BOOLEAN			bRmtOK;				// 远端设备Mac是否找到
	USHORT			RmtAddr;			// 远端设备Mac
	LARGE_INTEGER   lastDiscoTime;		// 上次收到Discovery包的时间
	ULONG			discoverySize;		// Discovery数据大小
	UCHAR			discoveryData[256];// 记录最后的Discovery信息
}DtPhyLinkInfoOld_t;

// 物理光口状态信息
typedef struct DtPhyLinkInfo_s
{
	BOOLEAN			bAvailable;			// 是否可用
	BOOLEAN			bConnected;			// 是否已经连接（信号链路连接）
	BOOLEAN			bFound;				// 是否有设备被找到
	BOOLEAN			bRmtOK;				// 远端设备Mac是否找到
	USHORT			RmtAddr;			// 远端设备Mac
	LARGE_INTEGER   lastDiscoTime;		// 上次收到Discovery包的时间
	ULONG			discoverySize;		// Discovery数据大小
	UCHAR			discoveryData[1024];// 记录最后的Discovery信息
}DtPhyLinkInfo_t;


// XGIGE PhyLink信息，以太网网口信息
typedef struct DtXgPhyLinkInfo_s
{
	BOOLEAN			bAvailable;			// 是否可用
	BOOLEAN			bConnected;			// 是否已经连接（信号链路连接）
	UCHAR			LocalEthAddr[6];	// 本地的MAC地址
	UCHAR			LocalIpAddr[4];		// 本地IP地址
}DtXgPhyLinkInfo_t;

/*
// XGIGE LINK信息
typedef struct DtXgDiscovery_s
{
	BOOLEAN			bFound;				// 远端设备Mac是否找到
	BOOLEAN			bArpOK;				// 用ARP包探测是否成功
	BOOLEAN			bWaitArpAck;		// 已经发送ARP请求，等待应答中
	LARGE_INTEGER   lastDiscoTime;		// 上次收到Discovery包的时间
	LARGE_INTEGER	lastArpTime;		// 上次发送ARP包的时间
	ULONG			uArpTimeoutCount;	// ARP的应答超时次数，达到n次，ArpOK将被清掉;应答成功一次，将被清0；
	UCHAR			RmtIpAddr[4];		// 远端的IP地址，从远端的ARP应答中获取
	UCHAR			RmtEthAddr[6];		// 远端设备的MAC地址
	UCHAR           LocalIpAddr[4];     // 本地的IP地址
	UCHAR			LocalEthAddr[6];	// 本地的MAC地址
	ULONG			discoverySize;		// Discovery数据大小
	UCHAR			discoveryData[512];	// 记录最后的Discovery信息
}DtXgLinkInfo_t;
*/

// XGIGE LINK信息 新框架修改
typedef struct DtXgDiscovery_s
{
	BOOLEAN			bFound;				// 远端设备Mac是否找到
	BOOLEAN			bArpOK;				// 用ARP包探测是否成功
	BOOLEAN			bWaitArpAck;		// 已经发送ARP请求，等待应答中
	BOOLEAN			rsv[5];
	//LARGE_INTEGER lastDiscoTime;		// 上次收到Discovery包的时间
	//LARGE_INTEGER	lastArpTime;		// 上次发送ARP包的时间
	//!这里使用LARGE_INTEGER，VS2010 32位编译导致结构体成员对齐大小有问题(dscam.cpp和dsglan.lib里不一致)，有可能是共用体嵌套导致，也可能是win系统使用LARGE_INTEGER有特定对齐
	uint64_t		lastDiscoTime;
	uint64_t		lastArpTime;
	ULONG			uArpTimeoutCount;	// ARP的应答超时次数，达到n次，ArpOK将被清掉;应答成功一次，将被清0；
	UCHAR			RmtIpAddr[4];		// 远端的IP地址，从远端的ARP应答中获取
	UCHAR			RmtEthAddr[6];		// 远端设备的MAC地址
	UCHAR           LocalIpAddr[4];     // 本地的IP地址
	UCHAR			LocalEthAddr[6];	// 本地的MAC地址
	ULONG			discoverySize;		// Discovery数据大小
	UCHAR			discoveryData[512];	// 记录最后的Discovery信息
	//UCHAR			rsv2[4];	
}DtXgLinkInfo_t;


// PCI-E速度/LANE
typedef enum
{
	R_2G5 = 0, // 1.0
	R_5G = 1,  // 2.0
	R_8G = 2,  // 3.0
	R_16G = 3, // 4.0
	R_32G = 4  // 5.0
}DtPeLinkRate_e;


// 设备信息
typedef struct DtPcieDevInfo_s
{
	// 板卡的标识寄存器，可以用来识别是不是我们支持的设备
	ULONG					DevID;

	// 实际硬件支持的通道数（PCI-E卡支持的通道，不是已经连接成功通道）
	ULONG					PhyLinkCount;

	//  实际硬件支持的采集通道数
	ULONG					GrabChCount;

	// 速率
	DtPeLinkRate_e			PeLinkRate;

	// 1,2,4,8,16
	UCHAR					PeLinkWidth;

	// 突发长度最大字节数
	ULONG					PePayloadSize;

	// MAC Addr
	USHORT					MacAddr;

    // BlockNumMax
    UCHAR					BlockNumMax;

    // 保留1字节
    UCHAR					ModeSupport;

    // 芯片DNA信息， 可以用做GUID
    ULONG                   dna[2];

    // 保留信息
    ULONG					resv[30];
}DtPcieDevInfo_t;


// 版本信息，4段式
typedef struct DtVersionInfo_s
{
    ULONG		KernelDriverVer[4]; // 内核驱动版本
    ULONG		PeCardVer[4];		// PCI-E卡版本
    ULONG		resv[8];			// 保留
}DtVersionInfo_t;

// 通道链路信息
typedef struct DtChLinkInfo_s
{
	USHORT	RmtAddr;	// 远端地址
	UCHAR	RmtPort;	// 远端端口 
	ULONG	ChID;		// Channel ID
	USHORT	LocalAddr;	// 本地（采集卡）地址
}DtChLinkInfo_t;

// Xg通道链路信息
typedef struct DtXgChLinkInfo_s
{
	UCHAR	RmtEthAddr[6];		// 远端设备的MAC地址
	UCHAR	LocalEthAddr[6];	// 本地的MAC地址
	ULONG	ChID;				// Channel ID
	ULONG	resv[16];			// 保留
}DtXgChLinkInfo_t;

// PCI寄存器读写
typedef struct DtPcieRw_s
{
	BOOLEAN		bRw;			// 0:写，1:读
	BOOLEAN		bBlock;			// Block方式，连续地址
	ULONG		uSize;			// 寄存器个数，或者Mem大小（x4 bytes）
	ULONG		Addr[128];
	ULONG		Data[128];
}DtPcieRw_t;


// 采集相关配置
#if 0
typedef struct DtGrabConfig_s
{
	ULONG		uBlockNum;		// 块个数，不超过8个
	ULONG		uBlockSize;		// 块大小

	ULONG		resv[16];		// 保留信息
}DtGrabConfig_t;
#else
typedef struct DtGrabConfig_s
{
	ULONG		uBlockNum;					// 块个数，不超过8个
	ULONG		uBlockSize;					// 块大小
	ULONG		ChipID;
	UCHAR		*pData[MAX_BLOCK_NUM];		// image data address
	ULONG		resv[7];					// 保留信息
}DtGrabConfig_t;

#endif

typedef struct DtMmapConfig_s
{
	ULONG uBlockIdx;
	ULONG uChID;

}DtMmapConfig_t;

typedef struct DtLogTextConfig_s{
	uint32_t size;
	char buffer[LOG_TEXT_BUF_SIZE];
}DtLogTextConfig_t;

typedef struct DtSendPack_s{
	ULONG uSendLen;
	ULONG uRecvLen;
	UCHAR buf[576];
	BOOL bNeedAck;
	ULONG  uChID;
}DtSendPack_t;

typedef struct DtRecvPack_s{
	UCHAR buf[576];
	ULONG uRecvLen;
}DtRecvPack_t;

// 采集相关配置
typedef struct DtGrabImage_s
{
    ULONG		uBlockIndex;	// 块索引
    ULONG		uBytes;			// 采集的字节数
    ULONG		uBlockID;		// 采集块的ID
    ULONG		uCheck;			// 校验码，32位移或值
   // ULONG		ChipID;
    PVOID64		pImage;			// 图像数据指针
    //ULONG		resv[16];		// 保留信息
}DtGrabImage_t;


// 采集相关状态信息
typedef struct DtChGrabStatus_s
{
    ULONGLONG	uCount;			// 接收到的帧
    ULONGLONG	uHandled;		// 被软件获取的帧
    ULONGLONG	uProblem;		// 出问题的次数
    ULONG		uBlocksValid;	// Block有效标识
    ULONG		uBlocksLock;	// Block锁标识
    ULONG		uBlocksSize[MAX_BLOCK_NUM];	// 记录Block采集大小
    ULONG		uBlocksID[MAX_BLOCK_NUM]; // 记录Block的ID
    ULONG		uBlocksCheck[MAX_BLOCK_NUM]; // 记录Block校验码
    ULONG		uBlocksBusy;	// Block正在被写入的标识
    ULONG		resv[4];		// 保留16字节
}DtChGrabStatus_t;


// 链路相关状态信息
typedef struct DtChLinkStatus_s
{
    ULONGLONG	uSendOk;		// 发送成功计数
    ULONGLONG	uSendErr;		// 发送出错计数
    ULONGLONG	uSendBytes;		// 发送字节数
    ULONGLONG	uRecvOk;		// 接收成功计数
    ULONGLONG	uRecvErr;		// 接收失败计数
    ULONGLONG	uRecvUnhandled;	// 接收到的包,没被处理的
    ULONGLONG	uRecvBytes;		// 接收字节数
}DtChLinkStatus_t;


typedef enum{
	DT_ALLOC_ENABLE = 0,
	DT_ALLOC_CHANNEL_NUM,
	DT_ALLOC_BLOCK_SIZE,
	DT_ALLOC_BLOCK_NUM,
	DT_ALLOC_MAX,
}DtPreAllocCmd;


// 预分配DMA内存
typedef struct DtPreAlloc_s
{
    BOOLEAN		bEnable;		// 使能
    ULONG		uBlockSize;		// 每个块的块大小
    ULONG		uBlocks;		// 块数
    ULONG		uChNum;			// 通道数
    ULONG		resv[8];		// 保留
}DtPreAlloc_t;

typedef struct LineBufInfo_s{
	char buf[64];
	long long pos;
}LineBufInfo;

// EEPROM读写
typedef struct DtEepromRw_s
{
	BOOLEAN		bRd;
	ULONG		uAddr;
	ULONG		uLen;
	ULONG		resv[8];		// 保留
	UCHAR		Data[1024];
}DtEepromRw_t;
