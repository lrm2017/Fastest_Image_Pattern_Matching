
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#include "include.h"
#include "public.h"
#include "regs.h"
#include "gige.h"

#define DEVICE_FILENAME "/dev/dtpcie"
#define SHARE_MEM_PAGE_COUNT 1

#define BUF_SIZE  (50 * 1024 * 1024)
#define MAX_ENUM_NUM	    32

//#define GVCP_UDP_PORT                       3956
//#define GVSP_UDP_PORT                       3957

//30*1024*1024


#define LOCAL_GVCP_PORT              18000
#define LOCAL_GVSP_PORT              18001
#define LOCAL_RMT_DBG_PORT           58580

//#define GVCP_HEAD_SIZE                      8
//#define GVCP_DATA_OFFSET                    GVCP_HEAD_SIZE

//#define READREG_CMD  						0x0080
//#define READREG_ACK  						0x0081

//#define WRITEREG_CMD  						0x0082
//#define WRITEREG_ACK  						0x0083

#define LOOP_TEST_NUM 5

typedef struct DtPeDeviceInfo_s  
{
	char				path[MAX_PATH];
	DtPcieDevInfo_t		DevInfo;
	DtXgLinkInfo_t		XgLinks[MAX_PHY_LINK_NUM];
	DtPhyLinkInfo_t		PhyLinks[MAX_PHY_LINK_NUM];
}DtPeDeviceInfo_t;

// 设备连接信息
typedef struct DtDevLinkInfo_s
{
    bool                bFilter;          // 是否为过滤驱动
    bool                bPeOnly;          // 只有PCIE卡，不须要盒子设备 
	uint32_t			PeIndex;	      // g_PeDeviceInfo中的索引号
	uint16_t			devAddr;	      // 盒子设备地址
	uint8_t				ssrPort;		  // sensor端口号
	char				szPath[MAX_PATH]; // 系统枚举的设备GUID信息, CreateFile须要使用
	char				szName[MAX_PATH]; // 提交给用户能方便看懂的名称
    uint8_t             fibersNum;        // 当前设备接的光纤个数，枚举阶段光纤个数信息，这个个数指示的是当前电脑枚举到的当前设备接的光纤个数
    uint32_t            fibersMask;       // 当前设备接的光纤掩码，bin0为1表示接了光纤0，bit1为1表示接了光纤1，根据这个信息可以判断当前盒子接的光纤数量

	uint8_t			DeviceMac[6];
	uint8_t			LocalMac[6];
}DtDevLinkInfo_t;

static uint16_t m_uIpIdentification = 0;
static uint16_t uDeviceGvcpPort = GVCP_UDP_PORT;
static uint16_t uRecvSockPort = LOCAL_GVCP_PORT;
//static uint16_t uRecvGvspPort = LOCAL_GVSP_PORT;

//DtXgLinkInfo_t g_linkinfo[MAX_PHY_LINK_NUM];
DtPhyLinkInfo_t g_linkinfo[MAX_PHY_LINK_NUM];
DtDevLinkInfo_t g_EnumInfo[MAX_ENUM_NUM];

int  g_dev;
uint16_t m_uCmdID = 0;
pthread_mutex_t g_mmap_lock = PTHREAD_MUTEX_INITIALIZER;
int g_uEnumUserName = 1;

void print_devinfo(int ret, DtPcieDevInfo_t *pdevinfo)
{
    printf ("ret is %d\n", ret);
    printf ("DevID        is 0x%x\n", pdevinfo->DevID);
    printf ("LinkCnt      is 0x%x\n", pdevinfo->PhyLinkCount);
    printf ("GrabChCnt    is 0x%x\n", pdevinfo->GrabChCount);
    printf ("LinkRate     is 0x%x\n", pdevinfo->PeLinkRate);
    printf ("LinkWidth    is 0x%x\n", pdevinfo->PeLinkWidth);
    printf ("Payloadsize  is 0x%x\n", pdevinfo->PePayloadSize);
    printf ("MAC          is 0x%x\n", pdevinfo->MacAddr);
    printf ("BlockNumMax  is 0x%x\n", pdevinfo->BlockNumMax);
    printf ("ModeSupport  is 0x%x\n", pdevinfo->ModeSupport);
    printf ("dna          is 0x%x 0x%x\n", pdevinfo->dna[0], pdevinfo->dna[1]);
}

uint16_t chksum(uint16_t sum, const UCHAR *data, uint16_t len)
{
    uint16_t t;
    const UCHAR *dataptr;
    const UCHAR *last_byte;

    dataptr = data;
    last_byte = data + len - 1;

    while (dataptr < last_byte) {    /* At least two more bytes */
        t = (dataptr[0] << 8) + dataptr[1];
        sum += t;
        if (sum < t) {
            sum++;        /* carry */
        }
        dataptr += 2;
    }

    if (dataptr == last_byte) {
        t = (dataptr[0] << 8) + 0;
        sum += t;
        if (sum < t) {
            sum++;        /* carry */
        }
    }

    /* Return sum in host byte order. */
    return ~sum;
}


int32_t XgCardSend(uint8_t *pData, uint32_t uSize, DtChLinkInfo_t *pLinkinfo, ULONG uChId)
{
    int32_t	ret = 0;
    uint8_t *pBuf = NULL;
	uint32_t uSendSize, uRecvSize;
	DtSendPack_t packInfo;
	DtRecvPack_t recvPack;
	ULONG *pChID;
	//int32_t t;

	pBuf = packInfo.buf;
	memcpy(pBuf, pData, uSize);

	packInfo.uSendLen = uSize; //(uSize+3)&(~0x3);
	packInfo.uRecvLen = 576;
	packInfo.bNeedAck = TRUE;

	for (int t=0; t<1; t++)
    {
        // !!!要发送4字节的整数倍
        //ret = ioctl (g_dev, IOCTL_PACK_SEND_LEN, (unsigned long)(&uSendSize));
        //ret = ioctl (g_dev, IOCTL_PACK_RECV_LEN, (unsigned long)(&uRecvSize));
        ret = ioctl (g_dev, IOCTL_PACK_SEND, (unsigned long)(&packInfo));

		printf("%s :IOCTL_PACK_SEND ret:%d, t:%d\r\n", __FUNCTION__, ret, t);
		
        if (ret == 0)
            break;
    }

	if(ret)
		return ret;
	
#if 1
{
	uint8_t *pGvcpAck = recvPack.buf+MAC_HDR_SIZE;
	pChID = (ULONG*)(&recvPack);
	*pChID = uChId;
	if(0 == ioctl(g_dev, IOCTL_PACK_RECV, (unsigned long)&recvPack))
	{
		if (GET_GVCP_ACK_STATUS(pGvcpAck) != GEV_STATUS_SUCCESS)
	    {
	    	uint16_t status = GET_GVCP_ACK_STATUS(pGvcpAck);
	        printf("%s: GVCP ACK status code: 0x%04X status:0x%x\n", __FUNCTION__, GET_GVCP_ACK_STATUS(pGvcpAck), status);
	        return -1;
	    }

		if(packInfo.buf[19] == 0x80)
		{

		    if (GET_GVCP_ACK_DATA_LEN(pGvcpAck) != (uSize - MAC_HDR_SIZE - GVCP_HEAD_SIZE))
		    {
		        printf("%s: GVCP ACK Data length is mismatch, need:%d, returned:%d\r\n", __FUNCTION__, 
						(uSize - GVCP_HEAD_SIZE - GVCP_HEAD_SIZE), GET_GVCP_ACK_DATA_LEN(pGvcpAck));
		        return -1;
		    }
		}

		memcpy(pData, pGvcpAck , uSize- MAC_HDR_SIZE);
	
	}else
		ret = -1;
}
#endif

    return ret;
}

int32_t GvcpWriteRegs(uint32_t *pRegAddr, uint32_t *pRegData, int iNum, uint32_t uTimeout, uint32_t uRetry,  DtChLinkInfo_t *pLinkinfo, ULONG uChipId)
{
	//int32_t		iRet;
	int32_t 	i,j;
	uint8_t 	CmdBuf[576];
	//uint8_t 	AckBuf[128];
	uint16_t	uPayloadLen;
    uint8_t *pCmd = CmdBuf;

	uint8_t *pGvcpCmd = CmdBuf + sizeof(DtMacHdr_t);
    //uint8_t *pGvcpAck = AckBuf + sizeof(DtMacHdr_t);	

	for (i=0, j=0; i<iNum; i++)
	{
		pGvcpCmd[GVCP_DATA_OFFSET + j++] = (pRegAddr[i] >> 24)&0xff;
		pGvcpCmd[GVCP_DATA_OFFSET + j++] = (pRegAddr[i] >> 16)&0xff;
		pGvcpCmd[GVCP_DATA_OFFSET + j++] = (pRegAddr[i] >> 8)&0xff;
		pGvcpCmd[GVCP_DATA_OFFSET + j++] = (pRegAddr[i])&0xff;
		pGvcpCmd[GVCP_DATA_OFFSET + j++] = (pRegData[i] >> 24)&0xff;
		pGvcpCmd[GVCP_DATA_OFFSET + j++] = (pRegData[i] >> 16)&0xff;
		pGvcpCmd[GVCP_DATA_OFFSET + j++] = (pRegData[i] >> 8)&0xff;
		pGvcpCmd[GVCP_DATA_OFFSET + j++] = (pRegData[i])&0xff;
	}

	uPayloadLen = (uint16_t)iNum*8;
	//iRet = SendCommand(CmdBuf, WRITEREG_CMD, uPayloadLen, TRUE, AckBuf, uTimeout, uRetry);

	DtMacHdr_t *pMacHdr = (DtMacHdr_t*)pCmd;
	pMacHdr->DstAddr[0] = (pLinkinfo->RmtAddr >> 8) & 0xff;
	pMacHdr->DstAddr[1] = pLinkinfo->RmtAddr & 0xff;
	pMacHdr->SrcAddr[0] = (pLinkinfo->LocalAddr >> 8) & 0xff;
	pMacHdr->SrcAddr[1] = pLinkinfo->LocalAddr & 0xff;
	pMacHdr->DstPort = pLinkinfo->RmtPort;
	pMacHdr->SrcPort = pLinkinfo->ChID;
	pMacHdr->Ctrl = 0; // 由FPGA管理
	pMacHdr->Protocol = MAC_PROT_CMD << 4;
	pMacHdr->DstLinkPort = 0; // 由FPGA管理
	pMacHdr->SrcLinkPort = 0; // 由FPGA管理
	pMacHdr->Res[0] = 0;
	pMacHdr->Res[1] = 0;
	pMacHdr->Res[2] = 0;
	pMacHdr->Res[3] = 0;
	pMacHdr->Res[4] = 0;
	pMacHdr->Res[5] = 0;

	pCmd[MAC_HDR_SIZE+ 0] = 0x42;
	pCmd[MAC_HDR_SIZE+ 1] = TRUE?0x01:0x0;
	pCmd[MAC_HDR_SIZE+ 2] = WRITEREG_CMD>>8;
	pCmd[MAC_HDR_SIZE+ 3] = WRITEREG_CMD&0xff;
	pCmd[MAC_HDR_SIZE+ 4] = uPayloadLen>>8;
	pCmd[MAC_HDR_SIZE+ 5] = uPayloadLen&0xff;
	pCmd[MAC_HDR_SIZE+ 6] = m_uCmdID>>8;
	pCmd[MAC_HDR_SIZE+ 7] = m_uCmdID&0xff;
    m_uCmdID++;
    
    return XgCardSend(pCmd, uPayloadLen+GVCP_HEAD_SIZE + MAC_HDR_SIZE, pLinkinfo, uChipId);
	//return 0;
}

int32_t GvcpReadRegs(uint32_t *pRegAddr, uint32_t *pRegData, int iNum, uint32_t uTimeout, uint32_t uRetry, DtChLinkInfo_t *pLinkinfo, ULONG uChipId)
{
	//int32_t		iRet;
	int32_t 	i,j;
	uint8_t 	CmdBuf[576];
	//uint8_t 	AckBuf[512];
	uint16_t	uPayloadLen;

	for (i=0, j=0; i<iNum; i++)
	{
		CmdBuf[GVCP_DATA_OFFSET + j++] = (pRegAddr[i] >> 24)&0xff;
		CmdBuf[GVCP_DATA_OFFSET + j++] = (pRegAddr[i] >> 16)&0xff;
		CmdBuf[GVCP_DATA_OFFSET + j++] = (pRegAddr[i] >> 8)&0xff;
		CmdBuf[GVCP_DATA_OFFSET + j++] = (pRegAddr[i])&0xff;
	}

	uPayloadLen = (uint16_t)iNum*4;

	DtMacHdr_t *pMacHdr = (DtMacHdr_t*)CmdBuf;
	pMacHdr->DstAddr[0] = (pLinkinfo->RmtAddr >> 8) & 0xff;
	pMacHdr->DstAddr[1] = pLinkinfo->RmtAddr & 0xff;
	pMacHdr->SrcAddr[0] = (pLinkinfo->LocalAddr >> 8) & 0xff;
	pMacHdr->SrcAddr[1] = pLinkinfo->LocalAddr & 0xff;
	pMacHdr->DstPort = pLinkinfo->RmtPort;
	pMacHdr->SrcPort = pLinkinfo->ChID;
	pMacHdr->Ctrl = 0; // 由FPGA管理
	pMacHdr->Protocol = MAC_PROT_CMD << 4;
	pMacHdr->DstLinkPort = 0; // 由FPGA管理
	pMacHdr->SrcLinkPort = 0; // 由FPGA管理
	pMacHdr->Res[0] = 0;
	pMacHdr->Res[1] = 0;
	pMacHdr->Res[2] = 0;
	pMacHdr->Res[3] = 0;
	pMacHdr->Res[4] = 0;
	pMacHdr->Res[5] = 0;

	CmdBuf[MAC_HDR_SIZE + 0] = 0x42;
	CmdBuf[MAC_HDR_SIZE + 1] = TRUE?0x01:0x0;
	CmdBuf[MAC_HDR_SIZE + 2] = READREG_CMD>>8;
	CmdBuf[MAC_HDR_SIZE + 3] = READREG_CMD&0xff;
	CmdBuf[MAC_HDR_SIZE + 4] = uPayloadLen>>8;
	CmdBuf[MAC_HDR_SIZE + 5] = uPayloadLen&0xff;
	CmdBuf[MAC_HDR_SIZE + 6] = m_uCmdID>>8;
	CmdBuf[MAC_HDR_SIZE + 7] = m_uCmdID&0xff;
    m_uCmdID++;

    XgCardSend(CmdBuf, uPayloadLen+GVCP_HEAD_SIZE + MAC_HDR_SIZE, pLinkinfo, uChipId);

    for (i=0, j=0; i<iNum; i++)
	{
		pRegData[i] = 0;
		pRegData[i] |= CmdBuf[GVCP_DATA_OFFSET + j++] << 24;
		pRegData[i] |= CmdBuf[GVCP_DATA_OFFSET + j++] << 16;
		pRegData[i] |= CmdBuf[GVCP_DATA_OFFSET + j++] << 8;
		pRegData[i] |= CmdBuf[GVCP_DATA_OFFSET + j++];
	}

	return 0;
}


int32_t GvcpReadReg(uint32_t uRegAddr, uint32_t *pRegData, uint32_t uTimeout, uint32_t uRetry, DtChLinkInfo_t *pLinkinfo,  ULONG uChipId)
{
	int32_t iRet = GvcpReadRegs(&uRegAddr, pRegData, 1, uTimeout, uRetry, pLinkinfo, uChipId);
    return iRet;
}

int32_t GvcpWriteReg(uint32_t uRegAddr, uint32_t uRegData, uint32_t uTimeout, uint32_t uRetry, DtChLinkInfo_t * pLinkinfo, ULONG uChipId)
{
#if 1
	int32_t iRet = GvcpWriteRegs(&uRegAddr, &uRegData, 1, uTimeout, uRetry, pLinkinfo, uChipId);
    return iRet;
#else
	return 0;
#endif
}

int32_t PciReadReg(uint32_t uRegAddr, uint32_t *pRegData, uint32_t uTimeout, uint32_t uRetry)
{
    int ret;
    DtPcieRw_t *pRwIn;
    pRwIn = malloc(sizeof (DtPcieRw_t));

    pRwIn->bRw = 1; //1读 0写
    pRwIn->bBlock = 0;
    pRwIn->uSize = 1;
    pRwIn->Addr[0] = uRegAddr;
    pRwIn->Data[0] = 0xfffffff;

    ret = ioctl(g_dev, IOCTL_PCIE_RW, (unsigned long)pRwIn);
    *pRegData = pRwIn->Data[0];
    free(pRwIn);

    return ret;
}

int32_t PciWriteReg(uint32_t uRegAddr, uint32_t uRegData, uint32_t uTimeout, uint32_t uRetry)
{
    int ret;
    DtPcieRw_t *pRwIn;
    pRwIn = malloc(sizeof (DtPcieRw_t));

    pRwIn->bRw = 0; //1读 0写
    pRwIn->bBlock = 0;
    pRwIn->uSize = 1;
    pRwIn->Addr[0] = uRegAddr;
    pRwIn->Data[0] = uRegData;

    ret = ioctl(g_dev, IOCTL_PCIE_RW, (unsigned long)pRwIn);
    free(pRwIn);

    return ret;
}

#if 0
int32_t OpenGvsp(uint32_t uIP, uint16_t uPort, DtXgLinkInfo_t *pLinkinfo, ULONG uChipId)
{
	int32_t iRet = 0;
	uint32_t uRegAddr, uRegData;

	/* 设置目标IP */
	uRegAddr = GVR_STREAM_CHANNEL_DEST_ADDR;
	uRegData = uIP;

	iRet = GvcpWriteReg(uRegAddr, uRegData, 0, 1, pLinkinfo, uChipId);
	if (iRet != 0)
		return iRet;

	/* 设置目标端口   GVSP和GVCP不是同一端口 */
	uRegAddr = GVR_STREAM_CHANNEL_PORT;
	uRegData = uPort;
	iRet = GvcpWriteReg(uRegAddr, uRegData, 0, 1, pLinkinfo, uChipId);
	if (iRet != 0)
		return iRet;

	printf ("%s, stream is opened for IP:%d.%d.%d.%d:%d\r\n",
            __func__,
		(uIP>>24)&0xff,
		(uIP>>16)&0xff,
		(uIP>>8)&0xff,
		uIP&0xff,
		uPort);
	return 0;
}
#endif

static bool g_camera_alive = true;

#if 0
void *heartbeat(void *p)
{
    uint32_t regData;
    GvcpReadReg(GVR_FIRMWARE_VER, &regData, 0, 0); 
    GvcpReadReg(GVR_FPGA_VER, &regData, 0, 0); 

    while (g_camera_alive)
    {
        //GvcpWriteReg(0x094C, 0x2, 0, 0); 
        GvcpReadReg(GVR_TIMESTAMP_VALUE_HIGH, &regData, 0, 0); 
        //printf ("reg:%08x data:%08x\n", GVR_TIMESTAMP_VALUE_HIGH, regData);
        GvcpReadReg(GVR_TIMESTAMP_VALUE_LOW, &regData, 0, 0); 
        //printf ("reg:%08x data:%08x\n", GVR_TIMESTAMP_VALUE_LOW, regData);

#if 0
        PciReadReg(ADDR_MAC_TRANS0_PHY_CNT_RX_UNKNOW, &regData, 0, 0);
        printf ("ADDR_MAC_TRANS0_PHY_CNT_RX_UNKNOW:%08x\n", regData);
        PciReadReg(ADDR_MAC_TRANS0_PHY_CNT_RX_PACK, &regData, 0, 0);
        printf ("ADDR_MAC_TRANS0_PHY_CNT_RX_PACK:%08x\n", regData);
        PciReadReg(ADDR_MAC_TRANS0_PHY_CNT_RX_PAUSE, &regData, 0, 0);
        printf ("ADDR_MAC_TRANS0_PHY_CNT_RX_PAUSE:%08x\n", regData);
        PciReadReg(ADDR_MAC_TRANS0_PHY_CNT_RX_CRC_ERR, &regData, 0, 0);
        printf ("ADDR_MAC_TRANS0_PHY_CNT_RX_CRC_ERR:%08x\n", regData);
        PciReadReg(ADDR_MAC_TRANS0_PHY_CNT_RX_DROP, &regData, 0, 0);
        printf ("ADDR_MAC_TRANS0_PHY_CNT_RX_DROP:%08x\n", regData);
        PciReadReg(ADDR_MAC_TRANS0_PHY_CNT_RX_HDR_ERR, &regData, 0, 0);
        printf ("ADDR_MAC_TRANS0_PHY_CNT_RX_HDR_ERR:%08x\n", regData);
        sleep(2);
#endif
    }

    return NULL;
}
#endif

#if 0
void printf_debug_info(DtXgLinkInfo_t *pLinkInfo, ULONG uChID)
{
	int i;
	uint32_t addr[25] = {0};
	uint32_t data[25] = {0};
	char reslut[8192] = {0};
	char tmp[128] = {0};
	
	addr[0] = ADDR_FRAME_CNT;
	addr[1] = ADDR_FRAME_DROP_CNT;
	addr[2] = ADDR_NO_START_CNT;
	addr[3] = ADDR_NO_END_CNT;
	addr[4] = ADDR_BUF_INFIFO_ERR_CNT;
	addr[5] = ADDR_BUF_HDR_ERR_CNT;
	addr[6] = ADDR_BUF_ID_ERR_CNT;
	addr[7] = ADDR_BUF_CHKSUM_ERR_CNT;
	addr[8] = ADDR_NO_FRAME_START_CNT;
	addr[9] = ADDR_NO_FRAME_END_CNT;
	addr[10] = ADDR_NO_LINE_START_CNT;
	addr[11] = ADDR_NO_LINE_END_CNT;
	addr[12] = ADDR_H_CHANGED_CNT;
	addr[13] = ADDR_V_CHANGED_CNT;
	addr[14] = ADDR_ORG_H_SIZE;
	addr[15] = ADDR_ORG_V_SIZE;
	addr[16] = ADDR_PACK_SIZE;
	addr[17] = ADDR_PACK_NUM;
	addr[18] = ADDR_BUFED_SIZE;
	addr[19] = ADDR_DROP_CASE;
	addr[20] = ADDR_FRAME_ID;
	addr[21] = 0x5f;
	addr[22] = 0x5e;
	for (i=0; i<=22; i++)
	{
		addr[i] = GVR_FPGA_MAPP_START + (addr[i] << 2);
	}

	addr[i] = GVR_STREAM_CHANNEL_PACK_SIZE;
	i++;
	addr[i] = GVR_STREAM_CHANNEL_PACKET_DELAY;
	i++;

	GvcpReadRegs(addr, data, i, 0, 0, pLinkInfo, uChID);

	sprintf(reslut, "DeviceInfo %u\r\n", uChID);
	sprintf(tmp, "	FRAME CNT: %d(0x%04X),	DROP_CNT: %d\r\n", data[0], data[0], data[1]); strcat(reslut, tmp);
	sprintf(tmp, "	PG NO_START: %d,  NO_END: %d\r\n", data[2], data[3]); strcat(reslut, tmp);
	sprintf(tmp, "	BUF INFIFO_ERR: %d,  HDR_ERR: %d,  ID_ERR:%d(%d),  CHKSUM_ERR:%d(%d)\r\n", data[4], data[5], data[6], data[21], data[7], data[22]); strcat(reslut, tmp);
	sprintf(tmp, "	NO_FRAME_START: %d,  NO_FRAME_END: %d,	\r\n", data[8], data[9]); strcat(reslut, tmp);
	sprintf(tmp, "	NO_LINE_START: %d,	NO_LINE_END: %d\r\n", data[10], data[11]); strcat(reslut, tmp);
	sprintf(tmp, "	H_SIZE_CHANGED: %d,  V_SIZE_CHANGED: %d\r\n", data[12], data[13]); strcat(reslut, tmp);
	sprintf(tmp, "	ORG_IMAGE_SIZE: %d*%d\r\n", data[14], data[15]); strcat(reslut, tmp);
	sprintf(tmp, "	Buffer Packet Size: %d,  num: %d,  Usage: %.2f%%(%dpack, %dbytes)\r\n", data[16], data[17], (double)data[18]*100/data[17], data[18], data[18]*data[16]); strcat(reslut, tmp);
	sprintf(tmp, "	Buffer Drop Case: %d/%d(%dbytes)\r\n", data[19],data[17], data[19]*data[16]); strcat(reslut, tmp);
	sprintf(tmp, "	FRAME_ID: %d\r\n", data[20]); strcat(reslut, tmp);		  
	sprintf(tmp, "	GVSP Packet Size: %d(interval:%d)\r\n", data[23], data[24]); strcat(reslut, tmp);	

	printf("%s", reslut);
}
#endif

void* ds_test(void *p)
{
#if 1
	DtDevLinkInfo_t * pLinkinfo = (DtDevLinkInfo_t *)p;

	DtXgChLinkInfo_t chLinkInfo;
	DtGrabConfig_t m_XgGrabConfig;
	DtMmapConfig_t XgMmapConfig;
	int ret, j, k, l;
	char *ptrdata[MAX_BLOCK_NUM] = {NULL};

	memcpy(chLinkInfo.RmtEthAddr, pLinkinfo->DeviceMac, 6);
	memcpy(chLinkInfo.LocalEthAddr, pLinkinfo->LocalMac, 6);

	ret = ioctl (g_dev, IOCTL_ALLOC_XG_CHANNEL, (unsigned long)(&chLinkInfo));
	if(ret != 0)
	{
		perror("ioctl-IOCTL_ALLOC_XG_CHANNEL");
		return NULL;
	}
	printf ("ch:%d\n", chLinkInfo.ChID);

	 /* 申请内存空间 */
	m_XgGrabConfig.uBlockNum = 4;
	m_XgGrabConfig.uBlockSize = BUF_SIZE; //26*1024*1024;
	ret = ioctl (g_dev, IOCTL_CHANNEL_GRAB_INIT, (unsigned long)(&m_XgGrabConfig));
	if(ret != 0)
	{
		perror("ioctl-IOCTL_CHANNEL_GRAB_INIT");
		goto err_free_channel;
	}

	 /* 映射内存到用户空间 */
	for (j = 0; j < m_XgGrabConfig.uBlockNum; j++)
	{
		XgMmapConfig.uBlockIdx = j;
		
		pthread_mutex_lock(&g_mmap_lock);
		ioctl (g_dev, IOCTL_SET_MAPMEM, (unsigned long)(&XgMmapConfig));
		ptrdata[j] = (char *)mmap(NULL, BUF_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, g_dev, 0);
		if(ptrdata[j] == MAP_FAILED)
		{
			printf("mmap failed!\n");
			goto err_unmmap;
		}
		pthread_mutex_unlock(&g_mmap_lock);
	}

	printf ("GrabInit OK\n");

	 /* Grab start */
	ret = ioctl (g_dev, IOCTL_CHANNEL_GRAB_START, (unsigned long)(0));
	if(ret != 0)
	{
		perror("ioctl-IOCTL_CHANNEL_GRAB_START");
		goto err_stop;
	}
#if 0
	usleep(40000);
	{
		uint32_t uRegData, uRegAddr;

		uRegData = 0x1;
		uRegAddr = 0x10000;

		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
			goto err_unmmap;
		}

		uRegData = 0x80000000 | 20000;	//原来4s
		uRegAddr = 0x10040;

		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
			goto err_unmmap;
		}

		uRegData = 0x1;
		uRegAddr = 0x20000;
		
		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
			goto err_unmmap;
		}
	}

	{
		uint32_t uRegData, uRegAddr;

		uRegAddr = 0x20004;
		uRegData = 0x80000000;
		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
			goto err_stop;
		}
	}
#endif	
#if 1
{
	int i;
	char imageName[64] = {0};
	FILE * fp;
	DtGrabImage_t GrabImage;
	
	for(i=0; i< 8; i++)
	{
		//memset(imageName, 0, sizeof(imageName));
		//sprintf(imageName, "image%d.raw", i);
		//fp = fopen(imageName, "w");
		printf("Grab pic %d\n", i);
	
		ret = ioctl (g_dev, IOCTL_CHANNEL_GRAB_IMAGE, &GrabImage);
		if(ret < 0)
		{
			//if(fp != NULL)
			//	fclose(fp);
			printf("ioctl-IOCTL_CHANNEL_GRAB_IMAGE ret:%d\n", ret);
			ioctl (g_dev, IOCTL_CHANNEL_GRAB_RELEASE, &GrabImage);
			break;
		}
		//if(fp != NULL)
		//{
		//	ret = fwrite(GrabImage.pImage, 5120*5120, 1, fp);
		//	fclose(fp);
		//}
		if(ret == 0)
		{
			printf("ret:%d GRAB_IMAGE uBlockIndex:%u uBytes:%u uBlockID:%u\n", 
				ret, GrabImage.uBlockIndex, GrabImage.uBytes, GrabImage.uBlockID);

			ret = ioctl (g_dev, IOCTL_CHANNEL_GRAB_RELEASE, &GrabImage);
			if(ret < 0)
			{
				printf("ioctl-IOCTL_CHANNEL_GRAB_RELEASE ret:%d\n", ret);			
				break;
			}
		}
		//printf_debug_info(pLinkinfo, chLinkInfo.ChID);
	}
}
#endif

err_stop:
	/* Grab stop */
	ret = ioctl (g_dev, IOCTL_CHANNEL_GRAB_STOP, (unsigned long)(0));
	perror("ioctl-IOCTL_CHANNEL_GRAB_STOP");

#if 0
	//printf_debug_info(pLinkinfo, chLinkInfo.ChID);
	{
		uint32_t uRegData, uRegAddr;

		uRegAddr = 0x20008;
		uRegData = 0x1;
		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
		}

		uRegAddr = 0x20000;
		uRegData = 0x0;
		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
		}

		uRegAddr = 0x10000;
		uRegData = 0x0;
		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
		}
	}
#endif

err_unmmap:
	for (j = 0; j < m_XgGrabConfig.uBlockNum; j++)
	{
		if(ptrdata[j] != NULL)
		{
			printf("munmap block %d\n", j);
			munmap(ptrdata[j], BUF_SIZE);
		}
	}

err_free_channel:
	ret = ioctl (g_dev, IOCTL_FREE_CHANNEL, (unsigned long)(&chLinkInfo.ChID));
	perror("ioctl-IOCTL_FREE_CHANNEL");
#endif
	return NULL;
}


void* test(void *p)
{
#if 1
	DtDevLinkInfo_t * pLinkinfo = (DtDevLinkInfo_t *)p;

	DtChLinkInfo_t chLinkInfo;
	DtGrabConfig_t m_XgGrabConfig;
	DtMmapConfig_t XgMmapConfig;
	int ret, j, k, l;
	char *ptrdata[MAX_BLOCK_NUM] = {NULL};

	chLinkInfo.RmtAddr = pLinkinfo->devAddr;
    chLinkInfo.RmtPort = pLinkinfo->ssrPort;

    ret = ioctl (g_dev, IOCTL_ALLOC_CHANNEL, (unsigned long)(&chLinkInfo));
	if(ret != 0)
	{
    	perror("ioctl-IOCTL_ALLOC_CHANNEL");
		return NULL;
	}
    printf ("ch:%d\n", chLinkInfo.ChID);

	 /* 申请内存空间 */
    m_XgGrabConfig.uBlockNum = 4;
    m_XgGrabConfig.uBlockSize = BUF_SIZE; //26*1024*1024;
    ret = ioctl (g_dev, IOCTL_CHANNEL_GRAB_INIT, (unsigned long)(&m_XgGrabConfig));
	if(ret != 0)
	{
		perror("ioctl-IOCTL_CHANNEL_GRAB_INIT");
		goto err_free_channel;
	}

	 /* 映射内存到用户空间 */
    for (j = 0; j < m_XgGrabConfig.uBlockNum; j++)
    {
		XgMmapConfig.uBlockIdx = j;
		
    	pthread_mutex_lock(&g_mmap_lock);
        ioctl (g_dev, IOCTL_SET_MAPMEM, (unsigned long)(&XgMmapConfig));
        ptrdata[j] = (char *)mmap(NULL, BUF_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, g_dev, 0);
		if(ptrdata[j] == MAP_FAILED)
		{
			printf("mmap failed!\n");
			goto err_unmmap;
		}
		pthread_mutex_unlock(&g_mmap_lock);
    }

	printf ("GrabInit OK\n");

	 /* Grab start */
    ret = ioctl (g_dev, IOCTL_CHANNEL_GRAB_START, (unsigned long)0);
	if(ret != 0)
	{
    	perror("ioctl-IOCTL_CHANNEL_GRAB_START");
		goto err_stop;
	}

	usleep(40000);
	{
		uint32_t uRegData, uRegAddr;

        uRegData = 0x1;
        uRegAddr = 0x10000;

    	ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
			goto err_unmmap;
		}

		uRegData = 0x80000000 | 20000;  //原来4s
        uRegAddr = 0x10040;

		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
			goto err_unmmap;
		}

		uRegData = 0x1;
        uRegAddr = 0x20000;
		
		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
			goto err_unmmap;
		}
	}

	{
		uint32_t uRegData, uRegAddr;

		uRegAddr = 0x20004;
    	uRegData = 0x80000000;
		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
			goto err_stop;
		}
	}
#if 0
{
	int i;
	char imageName[64] = {0};
	FILE * fp;
	DtGrabImage_t GrabImage;
	
	for(i=0; i< 8; i++)
	{
		//memset(imageName, 0, sizeof(imageName));
		//sprintf(imageName, "image%d.raw", i);
		//fp = fopen(imageName, "w");
		printf("Grab pic %d\n", i);

		GrabImage.ChipID = m_XgGrabConfig.ChipID;
	
		ret = ioctl (g_dev, IOCTL_CHANNEL_GRAB_IMAGE, &GrabImage);
		if(ret < 0)
		{
			//if(fp != NULL)
			//	fclose(fp);
			printf("ioctl-IOCTL_CHANNEL_GRAB_IMAGE ret:%d\n", ret);
			ioctl (g_dev, IOCTL_CHANNEL_GRAB_RELEASE, &GrabImage);
			break;
		}
		//if(fp != NULL)
		//{
		//	ret = fwrite(GrabImage.pImage, 5120*5120, 1, fp);
		//	fclose(fp);
		//}
		if(ret == 0)
		{
			printf("ret:%d GRAB_IMAGE uBlockIndex:%u uBytes:%u uBlockID:%u\n", 
				ret, GrabImage.uBlockIndex, GrabImage.uBytes, GrabImage.uBlockID);

			ret = ioctl (g_dev, IOCTL_CHANNEL_GRAB_RELEASE, &GrabImage);
			if(ret < 0)
			{
				printf("ioctl-IOCTL_CHANNEL_GRAB_RELEASE ret:%d\n", ret);			
				break;
			}
		}
		//printf_debug_info(pLinkinfo, chLinkInfo.ChID);
	}
}
#endif

err_stop:
	/* Grab stop */
    ret = ioctl (g_dev, IOCTL_CHANNEL_GRAB_STOP, (unsigned long)(&chLinkInfo.ChID));
    perror("ioctl-IOCTL_CHANNEL_GRAB_STOP");

	//printf_debug_info(pLinkinfo, chLinkInfo.ChID);
	{
		uint32_t uRegData, uRegAddr;

		uRegAddr = 0x20008;
    	uRegData = 0x1;
		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
		}

		uRegAddr = 0x20000;
    	uRegData = 0x0;
		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
		}

		uRegAddr = 0x10000;
    	uRegData = 0x0;
		ret = GvcpWriteReg(uRegAddr, uRegData, 0, 5, &chLinkInfo, chLinkInfo.ChID);
		if(ret != 0)
		{
			printf("GvcpWriteReg 0x%x failed!\n", uRegAddr);
		}
	}

err_unmmap:
	for (j = 0; j < m_XgGrabConfig.uBlockNum; j++)
	{
		if(ptrdata[j] != NULL)
		{
			printf("munmap block %d\n", j);
			munmap(ptrdata[j], BUF_SIZE);
		}
	}

err_free_channel:
    ret = ioctl (g_dev, IOCTL_FREE_CHANNEL, (unsigned long)(&chLinkInfo.ChID));
    perror("ioctl-IOCTL_FREE_CHANNEL");
#endif
	return NULL;
}

int LoadXgLinkInfo(DtPeDeviceInfo_t *pDevInfo, UINT devIndex)
{
	DWORD								lastError;
	int 								iRet = 0;
	ULONG								x;
	int *								p;
	int 								ret;

	do
	{
		ret= ioctl(g_dev, IOCTL_DEV_INFO, &pDevInfo->DevInfo);
		if (ret < 0)
        {
            printf( "DeviceIoControl(IOCTL_DEV_INFO) failed with Err:%d\r\n", ret);
            iRet = -1;
            break;
        }
		print_devinfo(ret, &pDevInfo->DevInfo);
	
		for (x=0; x<MAX_XG_LINK_NUM; x++)
		{
			//p = (int *)(&(g_XgEnumInfo[g_uXgEnumCount].XgLinks));
			DtXgLinkInfo_t *pPhyLink = &pDevInfo->XgLinks[x];
			p = (int *)pPhyLink;
			*p = x;
			ret = ioctl(g_dev, IOCTL_XG_LINK_INFO, (unsigned long)(pPhyLink));
			if (ret != 0 )
			{
				printf( "DeviceIoControl(IOCTL_XG_LINK_INFO) failed with ret:%d bFound:%d\r\n", 
					 ret, pPhyLink->bFound);
				iRet = -1;
				break;
			}
			else
			{
				if (pPhyLink->bFound)
				{
					printf("Found XG device(%d.%d.%d.%d) in XGCard%d\r\n",  
						pPhyLink->RmtIpAddr[0], 
						pPhyLink->RmtIpAddr[1],
						pPhyLink->RmtIpAddr[2],
						pPhyLink->RmtIpAddr[3],
						devIndex);
				}
			}
		}

	} while (0);

	return iRet;
}

int DtPcieLoadDeviceInfo(DtPeDeviceInfo_t *pDevInfo, UINT devIndex)
{
	int ret, j, k, l;
	UCHAR m[MAX_PHY_LINK_NUM] = {0};
	USHORT addr[MAX_PHY_LINK_NUM] = {0xffff};
	ULONG x;
	//DtXgLinkInfo_t linkinfo;
    //DtPhyLinkInfo_t linkinfo;
	uint32_t count = 0;
	int *p;
	DtPreAlloc_t PreAlloc;
	

	 /* 获取采集卡信息 */
    ret = ioctl(g_dev, IOCTL_DEV_INFO, &(pDevInfo->DevInfo));

    print_devinfo(ret, &(pDevInfo->DevInfo));

	strcpy(pDevInfo->path, DEVICE_FILENAME);

	for (k = 0; k < pDevInfo->DevInfo.PhyLinkCount; k++)
    {
        /* 获取链接信息 */
		DtPhyLinkInfo_t *pPhyLink = &pDevInfo->PhyLinks[k];
		
        p = (int *)pPhyLink;
        *p = k;

		printf("pPhyLink:%d: 0x%x\n", k, pPhyLink);

		ret = ioctl (g_dev, IOCTL_PHY_LINK_INFO, (unsigned long)(pPhyLink));
		if(ret != 0)
		{
			printf("IOCTL_PHY_LINK_INFO failed ret:%d idx:%d\n", ret, k);
			return -1;
		}

		printf("%d %d size:%u %lu\n", pPhyLink->bAvailable, pPhyLink->bRmtOK, pPhyLink->discoverySize, sizeof(DtDiscoveryInfo_t));
		if(pPhyLink->bAvailable && pPhyLink->bRmtOK && pPhyLink->discoverySize >= sizeof(DtDiscoveryInfo_t))
		{
			//memcpy(&g_linkinfo[count], &linkinfo, sizeof (linkinfo) );
		
			DtDiscoveryInfo_t * pDisco = (DtDiscoveryInfo_t *) pPhyLink->discoveryData;
			m[k] = pDisco->SsrPortNum;
			addr[k] = pPhyLink->RmtAddr;
			count += m[k];
			for(x = k; x < pDevInfo->DevInfo.PhyLinkCount; x++)
			{
				if(addr[x] == addr[k])
					m[x] = 0;
			}
		}
    }

	ret = ioctl(g_dev, IOCTL_CHANNEL_PREPARE, &count);
	if(ret != 0 )
	{
		printf("IOCTL_CHANNEL_PREPARE failed ret:%d", ret);
	}

	{
		DtLogTextConfig_t textConfig;

		textConfig.size = 0x80000;

		ret = ioctl(g_dev, IOCTL_LOG_TEXT, &textConfig);
		if(ret != 0 )
		{
			printf("IOCTL_LOG_TEXT failed ret:%d", ret);
		}else{

			printf("%s\n", textConfig.buffer);
		}
	
	}

#if 1
	PreAlloc.bEnable = TRUE;
	PreAlloc.uChNum = 4;
	PreAlloc.uBlocks = 4;
	PreAlloc.uBlockSize = 29360128;

	
	ret = ioctl(g_dev, IOCTL_PRE_ALLOC_SET, &PreAlloc);
	if(ret != 0 )
	{
		printf("IOCTL_PRE_ALLOC_SET failed ret:%d", ret);
	}
#endif	

	return ret;
}


int main(int argc, char *argv[])
{
    //int  loop;
    int  ret, j, k, l;
    char *ptrdata[MAX_BLOCK_NUM] = {NULL};
    DtPeDeviceInfo_t devinfo;
    //DtPcieRw_t *pRwIn;
    DtGrabConfig_t m_XgGrabConfig;
    //DtXgChLinkInfo_t chLinkInfo;
	DtChLinkInfo_t chLinkInfo;
    pthread_t pt;
	uint32_t g_uEnumCount = 0, cnt = 0;

	pthread_attr_t threadAttr[MAX_PHY_LINK_NUM];
	pthread_t th[MAX_PHY_LINK_NUM];
	GvcpDiscoveryAck *pDisco;

   // for (k = 0; k < MAX_PHY_LINK_NUM; k++)

	g_dev=open(DEVICE_FILENAME, O_RDWR);
    if (g_dev < 0)
    {
    	printf("open failed! %d\n", g_dev);
    	return -1;
    }

	ret = LoadXgLinkInfo(&devinfo, 0); //DtPcieLoadDeviceInfo(&devinfo, 0);
	if(ret)
	{
		return -1;
	}

	for(k=0; k < devinfo.DevInfo.PhyLinkCount; k++)
	{
		if(devinfo.XgLinks[k].bFound)
		{
			pDisco = (GvcpDiscoveryAck*)(devinfo.XgLinks[k].discoveryData+50); 
			
			memcpy(g_EnumInfo[k].DeviceMac, pDisco->MacAddr, 6);
			memcpy(g_EnumInfo[k].LocalMac, devinfo.XgLinks[k].LocalEthAddr, 6);
		
			g_uEnumCount++;
			if(g_uEnumCount >=4)
				break;
		}
	}
	

#if 0
	for(k=0; k < devinfo.DevInfo.PhyLinkCount; k++)
	{
		DtPhyLinkInfo_t *pPhyLink = &devinfo.PhyLinks[k];
		DtDiscoveryInfo_t * pDisco = (DtDiscoveryInfo_t*)pPhyLink->discoveryData;

		// 新增加mac0，mac1的crc，hdr等信息
        if (pPhyLink->bAvailable && pPhyLink->bRmtOK && pPhyLink->discoverySize >= sizeof(DtDiscoveryInfoOld_t))
        {
			cnt++;
            printf("DevAddr: 0x%04X, Model: %s, SN: %s, ID_SWI: %d, UserDef: %s, SsrPorts: %d\r\n",
                pPhyLink->RmtAddr,
                pDisco->Model,
                pDisco->Sn,
                pDisco->UserIdSwitch,
                pDisco->UserDefName,
                pDisco->SsrPortNum);

            if ((pDisco->InfoFunC & 0x1) && pPhyLink->discoverySize >168 && sizeof(DtDiscoveryInfo_t) > 168)
            {
                for (int t = 0; t < 4; t++)
                {
                    printf("phy[%d]:rxpause:%d,rxcrcerr:%d,rxhdrerr:%d,rxcmddrop:%d,rxcmdpack:%d,linksta:%d,txcmdpack:%d\r\n",
                    t,pDisco->PhyLinkInfo[t].RxPause,pDisco->PhyLinkInfo[t].RxCrcErr,pDisco->PhyLinkInfo[t].RxHdrErr,pDisco->PhyLinkInfo[t].RxCmdDrop
                    ,pDisco->PhyLinkInfo[t].RxCmdPack,pDisco->PhyLinkInfo[t].LinkSta,pDisco->PhyLinkInfo[t].TxCmdPack);
                }
            }
		}

		for (j = 0; j<pDisco->SsrPortNum; j++)
        {
            // 一个盒子上可能有多个sensor口，每个sensor口将枚举出一个设备, 名字由5部分组成
            if (g_uEnumCount < MAX_ENUM_NUM)
            {
				if (g_uEnumUserName == 0)
				{
                    snprintf(g_EnumInfo[g_uEnumCount].szName, sizeof(g_EnumInfo[g_uEnumCount].szName), "%s#%s##%02d%1d", pDisco->Model, pDisco->Sn, pDisco->UserIdSwitch, j);
				}
				else if (g_uEnumUserName == 1)
				{
                    snprintf(g_EnumInfo[g_uEnumCount].szName, sizeof(g_EnumInfo[g_uEnumCount].szName), "%s#%s#%s#%02d%1d", pDisco->Model, pDisco->Sn, pDisco->UserDefName, pDisco->UserIdSwitch, j);
				}
                printf("Enum by dtpcie: %s\r\n", g_EnumInfo[g_uEnumCount].szName);
                g_EnumInfo[g_uEnumCount].PeIndex = 0;
                g_EnumInfo[g_uEnumCount].devAddr = pPhyLink->RmtAddr;
                g_EnumInfo[g_uEnumCount].ssrPort = j;
				g_EnumInfo[g_uEnumCount].bPeOnly = false;
                g_EnumInfo[g_uEnumCount].bFilter = false;
                g_EnumInfo[g_uEnumCount].fibersNum = 1;
                g_EnumInfo[g_uEnumCount].fibersMask = 1 << pDisco->MacHdr.SrcLinkPort;
                snprintf(g_EnumInfo[g_uEnumCount].szPath, sizeof(g_EnumInfo[g_uEnumCount].szPath), "%s", devinfo.path);
				//strncpy(g_EnumInfo[g_uEnumCount].szPath, devinfo.path, sizeof(g_EnumInfo[g_uEnumCount].szPath));
				BOOL bFlag = FALSE;
				if (cnt > 1)		// 多根光纤
				{
					// 判断枚举名称是否重复，重复的不能使用
					ULONG t;
					for (t=0; t<g_uEnumCount; t++)
					{
						if (strcmp(g_EnumInfo[g_uEnumCount].szName ,g_EnumInfo[t].szName)==0)	// 遍历一遍，有一样名称的不要
						{
                            // 有一样名称表示当前盒子接了多根光纤，返回盒子接的光纤个数信息
							bFlag = TRUE;
                            g_EnumInfo[t].fibersNum++;
                            g_EnumInfo[t].fibersMask |= 1 << pDisco->MacHdr.SrcLinkPort;
						}
					}

					if (!bFlag && t == g_uEnumCount)
					{
						g_uEnumCount++;
					}
				}
				else
				{
					g_uEnumCount++;
				}
            }
        }
		
	}
#endif

	//g_uEnumCount = 0;

	if(g_uEnumCount > 1)
		g_uEnumCount = 1;

	for(k=0; k< g_uEnumCount; k++)
	{
		pthread_attr_init(&threadAttr[k]);

		ret = pthread_create(&th[k], &threadAttr[k], ds_test, (void*)(&g_EnumInfo[k]));
		if (ret == 0)
		{
			printf("create thread for %s OK\r\n", g_EnumInfo[k].szName);
		}
		else
		{
			printf("pthread_create failed, returned code:%d\r\n", ret);
		}
    }

   // uint32_t local_ip = (g_linkinfo[0].LocalIpAddr[0] << 24) | (g_linkinfo[0].LocalIpAddr[1] << 16) | (g_linkinfo[0].LocalIpAddr[2] << 8) | (g_linkinfo[0].LocalIpAddr[3]);
  //  printf ("local ip is %08x \n", local_ip);
    //local_ip = 0;

	for (j=0; j<g_uEnumCount; j++)
	{
		if (th[j] != NULL)
		{
			void *s;
			pthread_join(th[j], &s);
		}
	}

#if 0
{//set camera test image out
	uint32_t value;

	GvcpWriteReg(GVR_SENSOR_REG, 0x0, 0, 5);
	GvcpWriteReg(GVR_SENSOR_REG_CONTROL, 0x1, 0, 5);
	GvcpReadReg(GVR_SENSOR_REG_VALUE, &value, 0, 5);

	printf("test reg:0x%x\n", value);

	GvcpWriteReg(GVR_SENSOR_REG, 0x0, 0, 5);
	GvcpWriteReg(GVR_SENSOR_REG_VALUE, 0x80 | value , 0, 5);
	GvcpWriteReg(GVR_SENSOR_REG_CONTROL, 0x2, 0, 5);
}
#endif

#if 0
	uint32_t tmpReg, tmpData;
    uint8_t choice;

    while (1)
    {
        scanf("%c", &choice);
        switch(choice)
        {
            case '1':
                scanf("%x", &tmpReg);
                PciReadReg(tmpReg, &tmpData, 0, 0);
                printf ("Rd pcie:reg:%08x data:%08x\n", tmpReg, tmpData);
                break;
            case '2':
                scanf("%x", &tmpReg);
                GvcpReadReg(tmpReg, &tmpData, 0, 0);
                printf ("Rd GenICam:reg:%08x data:%08x\n", tmpReg, tmpData);
                break;
            case '3':
                scanf("%x %x", &tmpReg, &tmpData);
                PciWriteReg(tmpReg, tmpData, 0, 0);
                printf ("Wr pcie:reg:%08x data:%08x\n", tmpReg, tmpData);
                break;
            case '4':
                scanf("%x %x", &tmpReg, &tmpData);
                GvcpWriteReg(tmpReg, tmpData, 0, 0);
                printf ("Wr GenICam:reg:%08x data:%08x\n", tmpReg, tmpData);
                break;
            default:break;
        }
        if (choice == '0')
            break;
    }
#endif
#if 0
	while(1)
	{
		static int i = 0;
		if(i++ > 5)
		{
			break;
		}

		printf_debug_info();
		usleep(1000 * 1000);
	}
#endif
	close(g_dev);
    return 0;
}
