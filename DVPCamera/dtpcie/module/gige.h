/***************************************************************************************
Gige Vision命令、状态定义,为了方便查阅，各个宏定义从GigE Vision specification 文件中复制
了很详细的注释说明。
****************************************************************************************/
#ifndef __GIGE_H__
#define __GIGE_H__

#define GIGE_PROTOCOL_VERSION_1_2


/******************************************************************************
* GigeVision 各种Cmd/Ack 结构体定义
******************************************************************************/

typedef struct GVCP_CMD_HEAD_{
    unsigned char status[2];
    unsigned char answer[2];
    unsigned char lenth[2];//不包括头部长度
    unsigned char id[2];
}GVCP_CMD_HEAD;


typedef struct GVCP_CMD_{
    GVCP_CMD_HEAD   head;
    unsigned char   data[4];
}GVCP_CMD;


typedef struct GVCP_DISCOVERY_ACK_{
    GVCP_CMD_HEAD head;
    unsigned char spec_version_major[2];
    unsigned char spec_version_minor[2];
    unsigned char device_mode[4];
    unsigned char reserved0[2];
    unsigned char mac_addr[6];
    unsigned char ip_config_options[4];
    unsigned char ip_config_current[4];
    unsigned char reserved1[12];
    unsigned char current_ip[4];
    unsigned char reserved2[12];
    unsigned char current_net_mask[4];
    unsigned char reserved3[12];
    unsigned char default_gateway[4];
    unsigned char manufacturer_name [32];
    unsigned char model_name [32];
    unsigned char device_version [32];
    unsigned char manufacturer_specific_information[48];
    unsigned char serial_number [16];
    unsigned char user_defined_name [16];
}GVCP_DISCOVERY_ACK;


typedef struct GVCP_FORCEIP_LAYOUT_{
    GVCP_CMD_HEAD head;
    unsigned char ipsave;  //自定义的位
    unsigned char reserved0;
    unsigned char mac_addr[6];
    unsigned char reserved1[12];
    unsigned char static_ip[4];
    unsigned char reserved2[12];
    unsigned char static_netmask[4];
    unsigned char reserved3[12];
    unsigned char static_gateway[4];
}GVCP_FORCEIP_LAYOUT;


typedef struct GVCP_RESENDPACK_{
    GVCP_CMD_HEAD head;
    unsigned char stream_channel_index[2];
    unsigned char block_id[2];
    unsigned char reserved1;
    unsigned char first_pack_id[3];
    unsigned char reserved2;
    unsigned char last_pack_id[3];
}GVCP_RESENDPACK;


/* 用于从数据包中获取指定信息的宏 */
#define GVCP_HEAD_SIZE                      8
#define GVCP_DATA_OFFSET                    GVCP_HEAD_SIZE
#define GET_GVCP_ACK_STATUS(PHEAD)          ((PHEAD[0] << 8) |  PHEAD[1]) 
#define GET_GVCP_CMD(PHEAD)                 ((PHEAD[2] << 8) |  PHEAD[3]) 
#define GET_GVCP_ACK_DATA_LEN(PHEAD)        ((PHEAD[4] << 8) |  PHEAD[5])                                    
#define SET_GVCP_ACK_DATA_LEN(PHEAD,LEN)    {PHEAD[4] = (LEN >> 8)&0xFF; PHEAD[5] = LEN&0xFF}
#define GET_GVCP_CMD_ID(PHEAD)              ((PHEAD[6] << 8) |  PHEAD[7]) 


/******************************************************************************
* NET CONFIGURATION
******************************************************************************/
#define GVCP_UDP_PORT                       3956
#define GVSP_UDP_PORT                       3957



/******************************************************************************
* Command and Acknowledge Values
******************************************************************************/

/*Device and application MUST use the following value to represent the various messages.
Notice the value of the acknowledge message is always the associated command message
+ 1 (when such a command message exists), with PACKETRESEND_CMD and
PENDING_ACK being exceptions*/

/*Discovery Protocol Control*/
#define DISCOVERY_CMD  						0x0002
#define DISCOVERY_ACK  						0x0003

#define FORCEIP_CMD  						0x0004
#define FORCEIP_ACK  						0x0005

/*Streaming Protocol Control*/
#define PACKETRESEND_CMD  					0x0040
#define GVSP_INVALID_ON_GVCP_CHANNEL 		0x0041 

/* Device Memory Access*/
#define READREG_CMD  						0x0080
#define READREG_ACK  						0x0081

#define WRITEREG_CMD  						0x0082
#define WRITEREG_ACK  						0x0083

#define READMEM_CMD  						0x0084
#define READMEM_ACK  						0x0085

#define WRITEMEM_CMD  	                    0x0086
#define WRITEMEM_ACK                        0x0087

#define PENDING_ACK                         0x0089

/* Asynchronous Events */
#define EVENT_CMD                           0x00C0
#define EVENT_ACK                           0x00C1

#define EVENTDATA_CMD                       0x00C2
#define EVENTDATA_ACK                       0x00C3

/* Miscellaneous */
#define ACTION_CMD                          0x0100
#define ACTION_ACK                          0x0101


/* DO3THINK定义的重启命令 */
#define REBOOT_CMD                          0x0210
#define FLOW_CONTROL_ACK                    0x0220


/******************************************************************************
* status code
******************************************************************************/

/*This section lists the various status codes that can be returned in an acknowledge message or a GVSP
header. This specification defines two categories of status code:
1. Standard status code
2. Device-specific status code
Standard status codes are defined in this specification. Refer to appendix 2 for additional information about
supporting those codes.
[O-239cd] A device SHOULD return the status code that provides as much information as possible
about the source of error. [O18-1cd]
[R-240cd] If a device cannot return a more descriptive status code, the device MUST at least return
the GEV_STATUS_ERROR status code. [R18-2cd]
Status register is mapped as follows:

   0             1               2-3           4-15
severity device_specific_flag   reserved    status_value


severity 0 = info, 1 = error
device_specific_flag 0 for standard GigE Vision status codes,
1 for device-specific status codes
reserved Set to 0 on transmission, ignore on reception.
status_value Actual value of status code

*/


typedef unsigned short GevStatus;

/*Command executed successfully*/
#define GEV_STATUS_SUCCESS              0x0000 

/*Only applies to packet being resent. This flag is
preferred over the GEV_STATUS_SUCCESS when
the GVSP transmitter sends a resent packet. This can
be used by a GVSP receiver to better monitor packet
resend. Note that bit 15 of the GVSP packet flag field
must be set for a retransmitted GVSP packet when
block_id64 are used.*/
#define GEV_STATUS_PACKET_RESEND        0x0100

/*Command is not supported by the device*/
#define GEV_STATUS_NOT_IMPLEMENTED      0x8001

/*At least one parameter provided in the command is
invalid (or out of range) for the device*/
#define GEV_STATUS_INVALID_PARAMETER    0x8002

/*An attempt was made to access a non-existent address
space location.*/
#define GEV_STATUS_INVALID_ADDRESS      0x8003

/*The addressed register cannot be written to*/
#define GEV_STATUS_WRITE_PROTECT        0x8004

/*A badly aligned address offset or data size was
specified.*/
#define GEV_STATUS_BAD_ALIGNMENT        0x8005

/*An attempt was made to access an address location
which is currently/momentary not accessible. This
depends on the current state of the device, in
particular the current privilege of the application.*/
#define GEV_STATUS_ACCESS_DENIED        0x8006

/*A required resource to service the request is not
currently available. The request may be retried at a
later time.*/
#define GEV_STATUS_BUSY                 0x8007

/*Not used starting with GigE Vision 2.0.
Consult GigE Vision 1.2 for a detailed list.*/
#define GEV_STATUS_DEPRECATED           0x8008//0x8008-0x0800B  0x800F  

/*The requested packet is not available anymore.*/
#define GEV_STATUS_PACKET_UNAVAILABLE   0x800C

/*Internal memory of GVSP transmitter overrun
(typically for image acquisition)*/
#define GEV_STATUS_DATA_OVERRUN         0x800D

/*The message header is not valid. Some of its fields do
not match the specification.*/
#define GEV_STATUS_INVALID_HEADER       0x800E

/*The requested packet has not yet been acquired. Can
be used for linescan cameras device when line trigger
rate is slower than application timeout.*/
#define GEV_STATUS_PACKET_NOT_YET_AVAILABLE 0x8010

/*The requested packet and all previous ones are not
available anymore and have been discarded from the
GVSP transmitter memory. An application associated
to a GVSP receiver should not request retransmission
of these packets again.*/
#define GEV_STATUS_PACKET_AND_PREV_REMOVED_FROM_MEMORY  0x8011

/*The requested packet is not available anymore and
has been discarded from the GVSP transmitter
memory. However, applications associated to GVSP
receivers can still continue using their internal resend
algorithm on earlier packets that are still outstanding.
This does not necessarily indicate that any previous
data is actually available, just that the application
should not just assume everything earlier is no longer
available.*/
#define GEV_STATUS_PACKET_REMOVED_FROM_MEMORY           0x8012

/*The device is not synchronized to a master clock to be
used as time reference. Typically used when
Scheduled Action Commands cannot be scheduled at
a future time since the reference time coming from
IEEE 1588 is not locked.*/
#define GEV_STATUS_NO_REF_TIME                          0x0813

/*The packet cannot be resent at the moment due to
temporary bandwidth issues and should be requested
again in the future*/
#define GEV_STATUS_PACKET_TEMPORARILY_UNAVAILABLE       0x0814

/*A device queue or packet data has overflowed.
Typically used when Scheduled Action Commands
queue is full and cannot take the addition request.*/
#define GEV_STATUS_OVERFLOW                             0x0815

/*The requested scheduled action command was
requested at a time that is already past. Only available
starting with GEV 2.0 when an action_time is
specified.*/
#define GEV_STATUS_ACTION_LATE                          0x0816

/*Generic error. Try to avoid and use a more descriptive
status code from list above.*/
#define GEV_STATUS_ERROR                                0x8FFF



/****************************************************************************** 
GVSP PIXEL FORMAT DEFINE
******************************************************************************/

/* Indicate if pixel is monochrome or RGB*/
#define GVSP_PIX_MONO                           0x01000000
#define GVSP_PIX_RGB                            0x02000000 // deprecated in version 1.1
#define GVSP_PIX_COLOR                          0x02000000
#define GVSP_PIX_CUSTOM                         0x80000000
#define GVSP_PIX_COLOR_MASK                     0xFF000000

/* Indicate effective number of bits occupied by the pixel (including padding).
// This can be used to compute amount of memory required to store an image.*/
#define GVSP_PIX_OCCUPY1BIT                     0x00010000
#define GVSP_PIX_OCCUPY2BIT                     0x00020000
#define GVSP_PIX_OCCUPY4BIT                     0x00040000
#define GVSP_PIX_OCCUPY8BIT                     0x00080000
#define GVSP_PIX_OCCUPY12BIT                    0x000C0000
#define GVSP_PIX_OCCUPY16BIT                    0x00100000
#define GVSP_PIX_OCCUPY24BIT                    0x00180000
#define GVSP_PIX_OCCUPY32BIT                    0x00200000
#define GVSP_PIX_OCCUPY36BIT                    0x00240000
#define GVSP_PIX_OCCUPY48BIT                    0x00300000
#define GVSP_PIX_EFFECTIVE_PIXEL_SIZE_MASK      0x00FF0000
#define GVSP_PIX_EFFECTIVE_PIXEL_SIZE_SHIFT     16

/* Pixel ID: lower 16-bit of the pixel formats */
#define GVSP_PIX_ID_MASK                        0x0000FFFF
#define GVSP_PIX_COUNT                          0x46 /* next Pixel ID available*/

/* 27.1 Mono buffer format defines */
#define GVSP_PIX_MONO1P             (GVSP_PIX_MONO | GVSP_PIX_OCCUPY1BIT | 0x0037)
#define GVSP_PIX_MONO2P             (GVSP_PIX_MONO | GVSP_PIX_OCCUPY2BIT | 0x0038)
#define GVSP_PIX_MONO4P             (GVSP_PIX_MONO | GVSP_PIX_OCCUPY4BIT | 0x0039)
#define GVSP_PIX_MONO8              (GVSP_PIX_MONO | GVSP_PIX_OCCUPY8BIT | 0x0001)
#define GVSP_PIX_MONO8S             (GVSP_PIX_MONO | GVSP_PIX_OCCUPY8BIT | 0x0002)
#define GVSP_PIX_MONO10             (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0003)
#define GVSP_PIX_MONO10_PACKED      (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT | 0x0004)
#define GVSP_PIX_MONO12             (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0005)
#define GVSP_PIX_MONO12_PACKED      (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT | 0x0006)
#define GVSP_PIX_MONO14             (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0025)
#define GVSP_PIX_MONO16             (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0007)

/* 27.2 Bayer buffer format defines */
#define GVSP_PIX_BAYGR8             (GVSP_PIX_MONO | GVSP_PIX_OCCUPY8BIT | 0x0008)
#define GVSP_PIX_BAYRG8             (GVSP_PIX_MONO | GVSP_PIX_OCCUPY8BIT | 0x0009)
#define GVSP_PIX_BAYGB8             (GVSP_PIX_MONO | GVSP_PIX_OCCUPY8BIT | 0x000A)
#define GVSP_PIX_BAYBG8             (GVSP_PIX_MONO | GVSP_PIX_OCCUPY8BIT | 0x000B)
#define GVSP_PIX_BAYGR10            (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x000C)
#define GVSP_PIX_BAYRG10            (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x000D)
#define GVSP_PIX_BAYGB10            (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x000E)
#define GVSP_PIX_BAYBG10            (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x000F)
#define GVSP_PIX_BAYGR12            (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0010)
#define GVSP_PIX_BAYRG12            (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0011)
#define GVSP_PIX_BAYGB12            (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0012)
#define GVSP_PIX_BAYBG12            (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0013)
#define GVSP_PIX_BAYGR10_PACKED     (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT | 0x0026)
#define GVSP_PIX_BAYRG10_PACKED     (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT | 0x0027)
#define GVSP_PIX_BAYGB10_PACKED     (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT | 0x0028)
#define GVSP_PIX_BAYBG10_PACKED     (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT | 0x0029)
#define GVSP_PIX_BAYGR12_PACKED     (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT | 0x002A)
#define GVSP_PIX_BAYRG12_PACKED     (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT | 0x002B)
#define GVSP_PIX_BAYGB12_PACKED     (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT | 0x002C)
#define GVSP_PIX_BAYBG12_PACKED     (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT | 0x002D)
#define GVSP_PIX_BAYGR16            (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x002E)
#define GVSP_PIX_BAYRG16            (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x002F)
#define GVSP_PIX_BAYGB16            (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0030)
#define GVSP_PIX_BAYBG16            (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0031)

/* 27.3 RGB Packed buffer format defines */
#define GVSP_PIX_RGB8               (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY24BIT | 0x0014)
#define GVSP_PIX_BGR8               (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY24BIT | 0x0015)
#define GVSP_PIX_RGBA8              (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY32BIT | 0x0016)
#define GVSP_PIX_BGRA8              (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY32BIT | 0x0017)
#define GVSP_PIX_RGB10              (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY48BIT | 0x0018)
#define GVSP_PIX_BGR10              (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY48BIT | 0x0019)
#define GVSP_PIX_RGB12              (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY48BIT | 0x001A)
#define GVSP_PIX_BGR12              (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY48BIT | 0x001B)
#define GVSP_PIX_RGB16              (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY48BIT | 0x0033)
#define GVSP_PIX_RGB10V1_PACKED     (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY32BIT | 0x001C)
#define GVSP_PIX_RGB10P32           (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY32BIT | 0x001D)
#define GVSP_PIX_RGB12V1_PACKED     (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY36BIT | 0x0034)
#define GVSP_PIX_RGB565P            (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY16BIT | 0x0035)
#define GVSP_PIX_BGR565P            (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY16BIT | 0x0036)

/* 27.4 YUV and YCbCr Packed buffer format defines */
#define GVSP_PIX_YUV411_8_UYYVYY    (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY12BIT | 0x001E)
#define GVSP_PIX_YUV422_8_UYVY      (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY16BIT | 0x001F)
#define GVSP_PIX_YUV422_8           (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY16BIT | 0x0032)
#define GVSP_PIX_YUV8_UYV           (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY24BIT | 0x0020)
#define GVSP_PIX_YCBCR8_CBYCR       (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY24BIT | 0x003A)
#define GVSP_PIX_YCBCR422_8         (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY16BIT | 0x003B)
#define GVSP_PIX_YCBCR422_8_CBYCRY      (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY16BIT | 0x0043)
#define GVSP_PIX_YCBCR411_8_CBYYCRYY    (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY12BIT | 0x003C)
#define GVSP_PIX_YCBCR601_8_CBYCR       (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY24BIT | 0x003D)
#define GVSP_PIX_YCBCR601_422_8         (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY16BIT | 0x003E)
#define GVSP_PIX_YCBCR601_422_8_CBYCRY  (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY16BIT | 0x0044)
#define GVSP_PIX_YCBCR601_411_8_CBYYCRYY    (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY12BIT | 0x003F)
#define GVSP_PIX_YCBCR709_8_CBYCR           (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY24BIT | 0x0040)
#define GVSP_PIX_YCBCR709_422_8             (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY16BIT | 0x0041)
#define GVSP_PIX_YCBCR709_422_8_CBYCRY      (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY16BIT | 0x0045)
#define GVSP_PIX_YCBCR709_411_8_CBYYCRYY    (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY12BIT | 0x0042)

/* 27.5 RGB Planar buffer format defines */
#define GVSP_PIX_RGB8_PLANAR        (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY24BIT | 0x0021)
#define GVSP_PIX_RGB10_PLANAR       (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY48BIT | 0x0022)
#define GVSP_PIX_RGB12_PLANAR       (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY48BIT | 0x0023)
#define GVSP_PIX_RGB16_PLANAR       (GVSP_PIX_COLOR | GVSP_PIX_OCCUPY48BIT | 0x0024)




/*****************************************************************************
* Bootstrap Registers(GVR:GIGE VISION REGISTER)
******************************************************************************/

/*
Version Register
This register indicates the version of the GigE Vision specification implemented by this device. Version 2.0
of this specification shall return 0x00020000.
0 – 15 major_version This field represents the major version of the specification. For instance, GigE
Vision version 2.0 would have the major version set to 2.
16 – 31 minor_version This field represents the minor version of the specification. For instance, GigE
Vision version 2.0 would have the minor version set to 0.
Access type: Read

gige的版本，默认是2.0
*/
#define GVR_PROTOCOL_VERSION                0x0

/*
Device Mode Register
This register indicates the character set used by the various strings present in the bootstrap registers and
other device-specific information, such as the link configuration and the device class.
Access type: Read

Bits          Name                                              Description
0            endianess(E)                       Endianess might be used to interpret multi-byte data for READMEM and
                                                WRITEMEM commands. This represents the endianess of all device’s registers
                                                (bootstrap registers and manufacturer-specific registers).
                                                  0: reserved, do not use
                                                  1: big-endian device
                                                  GigE Vision 2.0 devices MUST always report big-endian and set this bit to 1.
                                                  大端和小端，gige是用的大端

1 – 3       device_class (DC)                  This field represents the class of the device. It must take one of the following
                                                values
             设备类型                           0: Transmitter   发送
                                                1: Receiver      接收
                                                2: Transceiver   收发
                                                3: Peripheral    外设
                                                other: reserved



4 – 7       current_link_configuration CLC)    Indicate the current Physical Link configuration of the device
                                                0: Single Link Configuration
                                                1: Multiple Links Configuration
                                                2: Static LAG Configuration
                                                3: Dynamic LAG Configuration
                                                Note: when Multiple Links Configuration and LAG are used concurrently, the
                                                the device shall report the configuration with the highest value.

8 – 23      reserved Always 0.                 Ignore when reading.

*/
#define GVR_DEVICE_MODE                     0x0004

/*
Device MAC Address Registers
These registers store the MAC address of the given network interface. If link aggregation is used, this is the
MAC address used by the link aggregation group.
[R-431cd] A device MUST implement the Device MAC Address registers on all of its supported
network interfaces. Both registers (High Part and Low Part) MUST be available. Devices
must return GEV_STATUS_INVALID_ADDRESS for unsupported network interfaces.
[R27-10cd]
An application needs to access the high part before accessing the low part:

High Part
Bits                Name                            Description
0 – 15      reserved Always 0.             Ignore when reading.
16 – 31     mac_address                    Hold the upper two bytes of the Device MAC address

Low Part
Bits                Name                            Description
0 – 31      mac_address                    Hold the lower four bytes of the Device MAC address
*/
#define GVR_DEVICE_MAC_ADDR_HIGH            0x0008
#define GVR_DEVICE_MAC_ADDR_LOW             0x000C


/*
Network Interface Capability Registers
These registers indicate the network and IP configuration schemes supported on the given network interface.
Multiple schemes can be supported simultaneously. If link aggregation is used, this is the IP configuration
scheme supported by the link aggregation group.
[R-432cd] The device MUST implement the Network Interface Capability register on all of its
supported network interfaces. Devices must return
GEV_STATUS_INVALID_ADDRESS for unsupported network interfaces.
Access type: Read
    Bits                Name                        Description
    0           PAUSE_reception (PR)             Set to 1 if the network interface can handle PAUSE frames.
    1           PAUSE_generation (PG)            Set to 1 if the network interface can generate PAUSE frames.
  2 – 28       reserved Always 0.               Ignore when reading.
    29          LLA (L)                          Link-local address is supported. Always 1
    30          DHCP (D)                         DHCP is supported. Always 1
    31          Persistent_IP (P)                1 if Persistent IP is supported, 0 otherwise.
*/
#define GVR_NET_INTERFACE_CAPABILITY        0x0010

/*
Network Interface Configuration Registers
These registers indicate which network and IP configurations schemes are currently activated on the given
network interface. If link aggregation is used, this is the current IP configuration scheme used by the link
aggregation group.
[R-433cd] The device MUST implement the Network Interface Configuration register on all of its
supported network interfaces. Devices must return
GEV_STATUS_INVALID_ADDRESS for unsupported network interfaces. [R27-12cd]

    Bits            Name                                Description
     0          PAUSE_reception (PR)            Set to 1 to enable the reception of PAUSE frames on this network interface.
                                                Factory default is device-specific and might be hard-coded if not configurable.
                                                New settings to take effect on next link negotiation. This setting must persist
                                                across device reset.
     1          PAUSE_generation (PG)           Set to 1 to enable the generation of PAUSE frames from this network interface.
                                                Factory default is device-specific and might be hard-coded if not configurable.
                                                New settings to take effect on next link negotiation. This setting must persist
                                                across device reset.
   2 – 28      reserved Always 0.              Ignore when reading.
     29         LLA (L)                         Link-local address is activated. Always 1.
     30         DHCP (D)                        DHCP is activated on this interface. Factor default is 1.
     31         Persistent_IP (P)               Persistent IP is activated on this interface. Factory default is 0.

*/
#define GVR_NET_INTERFACE_CONFIGURATION     0x0014

/*
Current IP Address Registers
These registers report the IP address for the given network interface once it has been configured. If link
aggregation is used, this is the IP address used by the link aggregation group.
[R-434cd] The device MUST support the Current IP Address register on all of its supported
network interfaces. Devices must return GEV_STATUS_INVALID_ADDRESS for
unsupported interfaces. [R27-13cd]
Access type: Read

  Bits         Name                 Description
0 – 31     IPv4_address    IP address for this network interface
*/
#define GVR_CURRENT_IP                      0x0024

/*
Current Subnet Mask Registers
These registers provide the subnet mask of the given interface. If link aggregation is used, this is the subnet
mask used by the link aggregation group.
[R-435cd] The device MUST support the Current Subnet Mask register on all of its supported
network interfaces. Devices must return GEV_STATUS_INVALID_ADDRESS for
unsupported network interfaces. [R27-14cd]
  Access type: Read

  Bits       Name                Description
0 – 31  IPv4_subnet_mask       Subnet mask for this network interface
*/
#define GVR_CURRENT_MASK                    0x0034

/*
Current Default Gateway Registers
These registers indicate the default gateway IP address to be used on the given network interface. If link
aggregation is used, this is the default gateway used by the link aggregation group.
[R-436cd] The device MUST support the Current Default Gateway register on all of its supported
network interfaces. Devices must return GEV_STATUS_INVALID_ADDRESS for
unsupported network interfaces. [R27-15cd]
  Access type: Read

  Bits          Name                            Description
0 – 31     IPv4_default_gateway        Default gateway for this network interface
*/
#define GVR_CURRENT_GATEWAY                 0x0044

/*
Manufacturer Name Register
This register stores a string containing the manufacturer name. This string uses the character set indicated in
the Device Mode register.
[R-437cd] A device MUST implement the Manufacturer Name register.
Access type: Read
Length :     32 bytes

Bits       Name                                    Description
 -     manufacturer_name String         indicating the manufacturer name. Returned in DISCOVERY_ACK and
                                        in the TXT record of DNS-SD.
制造商名称
*/
#define GVR_MAUFACTRURER_NAME               0x0048

/*
Model Name Register
This register stores a string containing the device model name. This string uses the character set indicated in
the Device Mode register.
[R-438cd] A device MUST implement the Model Name register.
Length :32 bytes
Access type: Read
Bits                Name                                Description
 -          model_name String                 indicating the device model. Returned in DISCOVERY_ACK and in the
                                              TXT record of DNS-SD.

设备名称
*/
#define GVR_MODEL_NAME                      0x0068

/*
Device Version Register
This register stores a string containing the version of the device. This string uses the character set indicated
in the Device Mode register. The XML device description file should also provide this information to
ensure the device matches the description file.
[R-439cd] A device MUST implement the Device Version register.
Length :32 bytes
Access type :Read
Bits                Name                                Description
 -          device_version String               indicating the version of the device. Returned in DISCOVERY_ACK
                                                and in the TXT record of DNS-SD.
设备的版本号，XML中的GVR_DEVICE_VERSION也要与之匹配
*/
#define GVR_DEVICE_VERSION                  0x0088

/*
Manufacturer Info Register
This register stores a string containing additional manufacturer-specific information about the device. This
string uses the character set indicated in the Device Mode register.
[R-440cd] A device MUST implement the Manufacturer Info register.
Length: 48 bytes
Access type: Read
Bits                Name                                Description
 -      manufacturer_specific_information       String providing additional information about this device. Returned in
                                                DISCOVERY_ACK and in the TXT record of DNS-SD.
厂家信息
*/
#define GVR_MANUFACTURER_INFO               0x00A8

/*
Serial Number Register
This register is optional. It can store a string containing the serial number of the device. This string uses the
character set indicated in the Device Mode register.
An application can check bit 1 of the GVCP Capability register at address 0x0934 to check if the serial
number register is supported by the device.
[O-441cd] A device SHOULD implement the Serial Number register.
Length :16 bytes
Access type :Read
Bits                Name                                Description
 -              serial_number                   String providing the serial number of this device. If supported, returned in
                                                DISCOVERY_ACK and in the TXT record of DNS-SD.
*/
//设备序列号
#define GVR_SERIAL_NUMER                    0x00D8

/*
User-defined Name Register
This register is optional. It can store a user-programmable string providing the name of the device. This
string uses the character set indicated in the Device Mode register.
An application can check bit 0 of the GVCP Capability register at address 0x0934 to check if the userdefined
name register is supported by the device.
[O-442cd] A device SHOULD implement the User-defined Name register.
[CR-443cd] When the User-defined Name register is supported, the device MUST provide
persistent storage to memorize the user-defined name. This name shall remain readable
across power-up cycles. The state of this register is stored in non-volatile memory as
soon as the User-defined Name register is set by an application. [CR27-18cd]
Length :16 bytes
Access type :Read/Write

Bits                Name                                Description
 -              user_defined_name               String providing the device name. If supported, returned in DISCOVERY_ACK
                                                and in the TXT record of DNS-SD.

*/
//用来存储用户设定的设备名
#define GVR_USERDEF_NAME                    0x00E8

/*
First URL Register
This register stores the first URL to the XML device description file. This string uses the character set
indicated in the Device Mode register. The first URL is used as the first choice by the application to
retrieve the XML device description file, except if a Manifest Table is available.
[R-444cd] A device MUST implement the First URL register, even if a Manifest Table is present.
Length: 512 bytes
Access type: Read
Bits            Name                                        Description
-           first_URL                           String providing the first URL to the XML device description file.
*/
#define GVR_FIRST_URL                       0x0200

/*
Second URL Register
This register stores the second URL to the XML device description file. This string uses the character set
indicated in the Device Mode register. This URL is an alternative if the application was unsuccessful to
retrieve the device description file using the first URL.
[R-445cd] A device MUST implement the Second URL register, even if a Manifest Table is present.
Length: 512 bytes
Access type: Read
Bits            Name                                        Description
-           first_URL String                    providing the first URL to the XML device description file.
*/
#define GVR_SECOND_URL                      0x0400

/*
Number of Network Interfaces Register
This register indicates the number of network interfaces supported by this device. This is generally
equivalent to the number of Ethernet connectors on the device, except when Link Aggregation is used. In
this case, the physical interfaces regrouped by the Aggregator are counted as one virtual interface.
[R-446cd] A device MUST implement the Number of Network Interfaces register.
A device need to support at least one network interfaces (the primary interface = interface #0). A device can
support at most four network interfaces.
Note that interface #0 is the only one supporting GVCP. Additional network interfaces only supports stream
channels in order to increase available bandwidth out of the device.
Length: 4 bytes
Access type :Read
Bits             Name                                       Description
0 – 28     reserved Always 0.                  Ignore when reading.
29 – 31    number_of_interfaces                Indicates the number of network interfaces. This field takes a value from 1 to 4.
                                                All other values are invalid.
                                                设置设备支持网络接口的数量，取值1~4
*/
#define GVR_NET_INTERFACE_NUMBER            0x0600

/*
Persistent IP Address Registers
These optional registers indicate the persistent IP address for the given network interface. They are only
used when the device boots with the Persistent IP configuration scheme. If link aggregation is used, this is
the persistent IP address used by the link aggregation group.
[CR-447cd] When Persistent IP is supported, the device MUST implement Persistent IP Address
register on all of its supported network interfaces. Devices must return
GEV_STATUS_INVALID_ADDRESS for unsupported network interfaces. [CR27-21cd]
Length :4 bytes
Access type :Read/Write
  Bits          Name                                        Description
0 – 31      persistent_IP_address              IPv4 persistent IP address for this network interface
*/
//设备的固定IP
#define GVR_PERSISTENT_IP                   0x064C

/*
Persistent Subnet Mask Registers
These optional registers indicate the persistent subnet mask associated with the persistent IP address on the
given network interface. They are only used when the device boots with the Persistent IP configuration
scheme. If link aggregation is used, this is the persistent subnet mask used by the link aggregation group.
[CR-448cd] When Persistent IP is supported, the device MUST implement Persistent Subnet Mask
register on all of its supported network interfaces. Devices must return
GEV_STATUS_INVALID_ADDRESS for unsupported network interfaces. [CR27-22cd]
  Bits          Name                                        Description
0 – 31     persistent_subnet_mask              IPv4 persistent subnet mask for this network interface
*/
#define GVR_PERSISTENT_MASK                 0x065C

/*
Persistent Default Gateway Registers
These optional registers indicate the persistent default gateway for the given network interface. They are
only used when the device boots with the Persistent IP configuration scheme. If link aggregation is used,
this is the persistent default gateway used by the link aggregation group.
[CR-449cd] When Persistent IP is supported, the device MUST implement Persistent Default
Gateway register on all of its supported network interfaces. Devices must return
GEV_STATUS_INVALID_ADDRESS for unsupported network interfaces. [CR27-23cd]
  Bits        Name                                          Description
0 – 31   persistent_default_gateway            IPv4 persistent default gateway for this network interface
*/
#define GVR_PERSISTENT_GATEWAY              0x066C

/*
Link Speed Registers
These optional registers provide the Ethernet link speed (in Mbits per second) for the given network
interface. This can be used to compute the transmission speed. If link aggregation is used, this is aggregated
link speed of all links participating in the aggregator.
[CR-450cd] When Link Speed registers are supported, the device MUST implement a Link Speed
register on all of its supported network interfaces. Devices must return
GEV_STATUS_INVALID_ADDRESS for unsupported network interfaces. [CR27-38cd]
For instance, if a device only implements a single network interface, then it shall only support one link
speed registers assigned to network interface #0. A link speed of 1 gigabit per second is equal to 1000 Mbits
per second while a link speed of 10 GigE is equal to 10 000 Mbits per second.
A capability bit in the GVCP Capability register (bit 3 at address 0x0934) indicates if these registers are
supported.
Length :4 bytes
Access type: Read

  Bits          Name                                    Description
0 – 31      link_speed                         Ethernet link speed value in Mbps. 0 if Link is down.
*/
#define GVR_LINK_SPEED                      0x0670

/*
Number of Message Channels Register
This register reports the number of message channels supported by this device.
[R-451cd] A device MUST implement the Number of Message Channels register.
A device may support at most 1 message channel, but it is allowed to support no message channel
Access type :Read
Bits            Name                                    Description
0 – 31 number_of_message_channels              Number of message channels supported by this device. Can be 0 or 1.
*/
//最多支持1个消息通道，但也可以没有消息通道，即值可为1也可为0
#define GVR_NUMBER_OF_MESSAGE_CHANNEL       0x0900

/*
Number of Stream Channels Register
This register reports the number of stream channels supported by this device.
[R-452cd] A device MUST implement the Number of Stream Channels register.
A product can support from 0 up to 512 stream channels.
Access type :Read
Bits                    Name                             Description
0 – 31         number_of_stream_channels       Number of stream channels supported by this device. A value from 0 to 512.
*/
//流通道支持0~512
#define GVR_NUMBER_OF_STREAM_CHANNEL        0x0904

/*
Number of Action Signals Register
This optional register reports the number of action signal supported by this device.
[CR-453cd] If a device supports Action commands, then it MUST implement the Number of Action
Signals register.
Access type :Read
Bits                    Name                            Description
0 – 31         number_of_action_signals        Number of action signals supported by this device. A value from 0 to 128.
*/
#define GVR_NUMBER_OF_ACTION_REG            0x0908

/*
Action Device Key Register
This optional register provides the device key to check the validity of action commands. The action device
key is write-only to hide the key if CCP is not in exclusive access mode. The intention is for the Primary
Application to have absolute control. The Primary Application can send the key to a Secondary Application.
This mechanism prevents a rogue application to have access to the ACTION_CMD mechanism.
[CR-454cd] If a device supports Action commands, then it MUST implement the Action Device Key
register.
Length :4 bytes
Access type :Write-only
Bits                        Name                        Description
0 – 31         action_device_key                Device key to check the validity of action commands
*/
#define GVR_ACTION_DEVICE_KEY               0x090C

/*
Number of Active Links
This register indicates the current number of active links. A link is considered active as soon as it is
connected to another Ethernet device. This happens after Ethernet link negotiation is completed.
[R-455cd] A device MUST implement the Number of Active Links register
Length: 4 bytes
Access type: Read
Bits                        Name                        Description
0 – 27             reserved Always 0.          Ignore when reading.
28 – 31            number_of_active_links      A value indicating the number of active links. This number must be
                                                lower or equal to the Number of Network Interfaces reported at
                                                address 0x0600.Single network interface device shall report a
                                                value of 1.
网口活动链接的数量，1~4
*/
#define GVR_NUMBER_OF_ACTIVE_LINKS          0x0910

/*
GVSP Capability Register
This register reports the optional GVSP stream channel features supported by this device.
[R-456cd] A transmitter, receiver or transceiver device MUST implement the GVSP Capability
register.
Length 4 :bytes
Access type :Read
Bits                        Name                        Description
 0                  SCSPx_supported (SP)        Indicates the SCSPx registers (stream channel source port) are available for all
                                                supported stream channels.
 1        legacy_16bit_block_id_supported (LB)  Indicates this GVSP transmitter or receiver can support 16-bit block_id. Note
                                                that GigE Vision 2.0 transmitters and receivers MUST support 64-bit block_id64.
                                                Note: When 16-bit block_id are used, then 24-bit packet_id must be used.
                                                When 64-bit block_id64 are used, then 32-bit packet_id32 must be used.
2 – 31             Reserved Always 0.          Ignore when reading.
*/
#define GVR_GVSP_CAPABILITY                 0x092C

/*
Message Channel Capability Register
This register reports the optional message channel features supported by this device.
[R-457cd] A device MUST implement the Message Channel Capability register.
Length :4 bytes
Access type :Read
Bits                        Name                        Description
 0                  MCSP_supported (SP)         Indicates the MCSP register (message channel source port) is available for the
                                                message channel.
1 – 31             reserved Always 0.          Ignore when reading.
*/
#define GVR_MESSAGE_CHANNEL_CAPABILITY      0x0930

/*
GVCP Capability Register
This register reports the optional GVCP commands and bootstrap registers supported by this device on its
Control Channel. When supported, some of these features are enabled through the GVCP Configuration
register (at address 0x0954).
[R-458cd] A device MUST implement the GVCP Capability register.
Length: 4 bytes
Access type: Read

Bits         Name                                           Description
0       user_defined_name (UN)                      User-defined name register is supported.
1       serial_number (SN)                          Serial number register is supported.
2       heartbeat_disable (HD)                      Heartbeat can be disabled.  心跳
3       link_speed_register (LS)                    Link Speed registers are supported.
4       CCP_application_port_IP_register (CAP)      CCP Application Port and IP address registers are supported.
5       manifest_table (MT)                         Manifest Table is supported. When supported, the application must use the
                                                    Manifest Table to retrieve the XML device description file.
6       test_data (TD)                              Test packet is filled with data from the LFSR generator.
7       discovery_ACK_delay(DD)                     Discovery ACK Delay register is supported.
8       writable_discovery_ACK_delay (WD)           When Discovery ACK Delay register is supported, this bit indicates that the
                                                    application can write it. If this bit is 0, the register is read-only.
9       extended_status_code_for_1_1 (ES)           Support generation of extended status codes introduced in specification 1.1:
                                                    GEV_STATUS_PACKET_RESEND,
                                                    GEV_STATUS_PACKET_NOT_YET_AVAILABLE,
                                                    GEV_STATUS_PACKET_AND_PREV_REMOVED_FROM_MEMORY and
                                                    GEV_STATUS_PACKET_REMOVED_FROM_MEMORY.
10      primary_application_switchover (PAS)        Primary application switchover capability is supported.
11      unconditional_ACTION(UA)                    Unconditional ACTION_CMD is supported (i.e. ACTION_CMD are processed
                                                    when the primary control channel is closed).
12      IEEE1588_support(PTP)                       Support for IEEE 1588 PTP
13      extended_status_code_for_2_0 (ES2)          Support generation of extended status codes introduces in specification 2.0:
                                                    GEV_STATUS_PACKET_TEMPORARILY_UNAVAILABLE,
                                                    GEV_STATUS_OVERFLOW, GEV_STATUS_NO_REF_TIME.
14      scheduled_action_command (SAC)              Scheduled Action Commands are supported. When this bit is set, this also
                                                    indicates that the Scheduled Action Command Queue Size register is
                                                    present.
15 – 24 reserved Always 0.                         Ignore when reading.
25      ACTION (A)                                  ACTION_CMD and ACTION_ACK are supported.
26      PENDING_ACK (PA)                            PENDING_ACK is supported.
27      EVENTDATA (ED)                              EVENTDATA_CMD and EVENTDATA_ACK are supported.
28      EVENT (E)                                   EVENT_CMD and EVENT_ACK are supported.
29      PACKETRESEND (PR)                           PACKETRESEND_CMD is supported.
30      WRITEMEM (W)                                WRITEMEM_CMD and WRITEMEM_ACK are supported.
31      concatenation (C)                           Multiple operations in a single message are supported.
*/
#define GVR_GVCP_CAPABILITY                 0x0934

/*
Heartbeat Timeout Register
This register indicates the current heartbeat timeout in milliseconds.
[R-459cd] A device MUST implement the Heartbeat Timeout register.
A value smaller than 500 ms SHOULD be defaulted to 500 ms by the device. In this case,
the Heartbeat Timeout register content is changed to reflect the actual value used by the
device. [O27-24cd]
Length: 4 bytes
Access type :Read/Write
Bits        Name                    Description
0 – 31     timeout           Heartbeat timeout in milliseconds (minimum is 500 ms)
*/
#define GVR_HEARTBEAT_TIMEOUT               0x0938

/*
Timestamp Tick Frequency Registers
These optional registers indicate the number of timestamp tick during 1 second. This corresponds to the
timestamp frequency in Hertz. For example, a 100 MHz clock would have a value of 100 000 000 =
0x05F5E100. They are combined to form a 64-bit value.
If IEEE 1588 is used, then the Timestamp Tick Frequency register must be 1 000 000 000 Hz (equivalent
to 1 GHz). This indicates that the timestamp value is reported in units of 1 ns. But the precision of the
timestamp register might be worst than 1 ns (i.e. the timestamp register does not necessarily takes all
possible values when it increments, it can skip some value depending on the precision of the IEEE 1588
clock).
Note the timestamp counter is also used to compute stream channel inter-packet delay.
No timestamp is supported if both of these registers are equal to 0.
[CR-461cd] If a device supports a device timestamp counter, then it MUST implement the
Timestamp Tick Frequency registers. Both registers MUST be available (High Part and
Low Part). If its value is set to 0 or if this register is not available, then no timestampcounter is
present. This means no mechanism is offered to control inter-packet delay. [O27-25cd]
An application needs to access the high part before accessing the low part:

High Part
Bits        Name                    Description
0 – 31 timestamp_frequency     Timestamp frequency, upper 32-bit
Low Part
Bits        Name                    Description
0 – 31 timestamp_frequency     Timestamp frequency, lower 32-bit

Access type :Read

*/
#define GVR_TIMESTAMP_TICK_FREQUNENCY_HIGH  0x093C
#define GVR_TIMESTAMP_TICK_FREQUNENCY_LOW   0x0940

/*
Timestamp Control Register
This optional register is used to control the timestamp counter.
[CR-462cd] If a device supports a device timestamp counter, then it MUST implement the
Timestamp Control register.
[CR-463ca] If a timestamp counter exists, an application MUST not attempt to read this register. This
register is write-only. [CR27-26ca]
Bits            Name                Description
0 – 29         reserved            Always 0.
30              latch (L)           Latch current timestamp counter into Timestamp Value register
                                    at address 0x0948.
31              reset (R)           Reset timestamp 64-bit counter to 0. It is not possible to reset the
                                    timestamp when operating with an IEEE 1588 disciplined clock.
*/
#define GVR_TIMESTAMP_CONTROL               0x0944

/*
Timestamp Value Registers
These optional registers report the latched value of the timestamp counter. It is necessary to latch the 64-bit
timestamp value to guarantee its integrity when performing the two 32-bit read accesses to retrieve the
higher and lower 32-bit portions.
[CR-465cd] If a device supports a device timestamp counter, then it MUST implement the
Timestamp Value registers. Both registers MUST be available (High Part and Low Part)
[CR-466ca] If an application wants to retrieve the 64-bit value of the timestamp counter and the
timestamp counter is supported by the device, it MUST write 1 into bit 30 (latch) of
Timestamp Control register for the free-running timestamp counter to be copied into
these registers. [CR27-28ca]
[CR-467cd] The Timestamp Value register MUST always be 0 if timestamp is not supported. [CR27-
29cd]
These registers do not necessarily take all possible values when they increments: they can skip some value
depending on the precision of the clock source relative to the Timestamp Tick Frequency register. For
instance, if the device supports IEEE 1588, it must report a tick frequency of 1 GHz. But the quality of the
IEEE 1588 master clock might not provide that level of accuracy.
An application needs to access the high part before accessing the low part:
High Part
Bits        Name                        Description
0 – 31  latched_timestamp_value     Latched timestamp value, upper 32-bit
Low Part
Bits        Name                        Description
0 – 31  latched_timestamp_value     Latched timestamp value, lower 32-bit

Access type :Read
*/
#define GVR_TIMESTAMP_VALUE_HIGH            0x0948
#define GVR_TIMESTAMP_VALUE_LOW             0x094C

/*
Discovery ACK Delay Register
This optional register indicates the maximum randomized delay in milliseconds the device will wait upon
reception of a DISCOVERY_CMD before it will send back the DISCOVERY_ACK. This randomized
delay can take any value from 0 second up to the value indicated by this bootstrap register.
[O-468cd] A device SHOULD implement the Discovery ACK Delay register.
[CR-469cd] If Discovery ACK Delay register is writable, then its value MUST be persistent and
stored in non-volatile memory to be re-used on next device reset or power cycle.
[CR27-41cd]
[CR-470cd] If Discovery ACK Delay is supported, the maximum factory default value for this
register MUST be lower or equal to 1000 ms (1 second). [CR27-42cd]
The previous requirement is necessary to ensure backward compatibility with version 1.0 of the
specification.
This register can be read-only if the device only supports a fixed value. Furthermore a device is allowed to
set this register has read-only with a value of 0 if it does not support such a randomized delay. Bit 8
(writable_discovery_ACK_delay) of the GVCP Capability register (at address 0x0934) indicates if this
register is writable.
An application should retrieve information from the XML device description file to see if this register is
writable and to determine the maximum value it can take.

  Bits          Name                    Description
0 – 15     reserved Always 0.      Ignore when reading.
16 – 31        delay               Maximum random delay in ms to wait before sending DISCOVERY_ACK
                                    upon reception of DISCOVERY_CMD.
Access type: Read/(Write)

*/
#define GVR_DISCOVERY_ACK_DELAY             0x0950

/*
GVCP Configuration Register
This optional register provides additional control over GVCP. These additional functions must be indicated
by the GVCP Capability register (at address 0x0934).
For instance, it can be used to disable Heartbeat when this capability is supported. This can be useful for
debugging purposes.
[O-471cd] A device SHOULD implement the GVCP Configuration register.

    Bits            Name                                Description
   0–11       reserved                     Always 0.Ignore when reading.
    12          IEEE1588_PTP_enable(PTP)    Enable usage of the IEEE 1588 Precision Time Protocol to source the
                                            timestamp register. Only available when the IEEE1588_support bit of the
                                            GVCP Capability register is set. When PTP is enabled, the Timestamp Control
                                            register cannot be used to reset the timestamp. Factory default is devicespecific.
                                            When PTP is enabled or disabled, the value of Timestamp Tick Frequency and
                                            Timestamp Value registers might change to reflect the new time domain.
    13    extended_status_code_for_2_0 (ES2)    Enable the generation of extended status codes introduced in specification 2.0:
                                                GEV_STATUS_PACKET_TEMPORARILY_UNAVAILABLE,
                                                GEV_STATUS_OVERFLOW, GEV_STATUS_NO_REF_TIME. Only
                                                available when the extended_status_code_for_2_0 bit of the GVCP Capability
                                                register is set.
                                                Factory default must be 0.
   14–27        reserved                   Always 0. Ignore when reading.
    28    unconditional_ACTION_enable (UA)  Enable unconditional action command mode where ACTION_CMD are
                                            processed even when the primary control channel is closed. Only available
                                            when the unconditional_ACTION bit of the GVCP Capability register is set.
                                            Factory default must be 0.
                                            Note: unconditional_ACTION_enable is provided to allow some level of
                                            protection against inappropriate reception of action commands. This might be
                                            useful as a security measure in industrial control for instance.
    29    extended_status_code_for_1_1(ES)  Enable generation of extended status codes introduced in specification 1.1:
                                            GEV_STATUS_PACKET_RESEND,
                                            GEV_STATUS_PACKET_NOT_YET_AVAILABLE,
                                            GEV_STATUS_PACKET_AND_PREV_REMOVED_FROM_MEMORY and
                                            GEV_STATUS_PACKET_REMOVED_FROM_MEMORY. Only available if
                                            the extended_status_code_for_1_1 bit of the GVCP Capability register is set
                                            Factory default must be 0.
    30     PENDING_ACK_enable(PE)           Enable generation of PENDING_ACK by this device. Only available when the
                                            PENDING_ACK capability bit of the GVCP Capability register is set is set.
                                            Factory default must be 0.
    31     heartbeat_disable (HD)           Disable heartbeat. Only available when heartbeat_disable capability bit of the
                                            GVCP Capability register is set is set. Factory default must be 0.

Access type :Read/Write
Length :4 bytes
*/
#define GVR_GVCP_CONFIGURATION              0x0954

/*
Pending Timeout Register
This register indicates the longest GVCP command execution time before an ACK is returned. The packet
travel time on the network is not accounted for by this register.
[R-472cd] A device MUST implement the Pending Timeout register.
Two scenarios exist:
1. If PENDING_ACK is disabled, then this register represents the worst-case single GVCP command
execution time (not including concatenated read/write access).
2. If PENDING_ACK is enabled, then this register represents the maximum amount of time it takes
before a PENDING_ACK is issued.
Therefore, a device might change the value of this read-only register when PENDING_ACK is enabled or
disabled through the GVCP Configuration register (bit 30, PENDING_ACK_enable field, at address
0x0954).
The Application can use this value to deduce a suitable ACK timeout to wait for when it issues the various
GVCP commands.
Length :4 bytes
Access type :Read

Bits            Name                        Description
0 – 31     max_execution_time      Provide the worst-case execution time (in ms) before the device will issue a
                                    PENDING_ACK, if supported and enabled, to notify the application to
                                    extend the ACK timeout for the current GVCP command. This time is for
                                    non-concatenated read/write accesses.
从执行GVCP命令到返回ACK的最大时间
*/
#define GVR_PENDING_TIMEOUT                 0x0958

/*
Control Switchover Key Register
This optional register provides the key to authenticate primary application switchover requests. The register
is write-only to hide the key from secondary applications. The primary intent is to have a mechanism to
control who can take control over a device. The primary application or a higher level system management
entity can send the key to an application that would request control over a device.
[CR-473cd] If a device supports primary application switchover, then it MUST implement the
Control Switchover Key register.
Length: 4 bytes
Access type: Write-only
  Bits          Name                        Description
 0 – 15     reserved                Always 0.
16 – 31  control_switchover_key     Key to authenticate primary application switchover requests
*/
#define GVR_CONTROL_SWITCHOVER_KEY          0x095C

/*
GVSP Configuration Register
This optional register provides additional global control over GVSP configuration. These additional
functions are indicated by the GVSP Capability register (at address 0x092C).
[O-474cd] A device SHOULD implement the GVSP Configuration register.
Length :4 bytes
Access type :Read/Write
Bits        Name                     Description
0       reserved                Always 0. Ignore when reading.
1    64bit_block_id_enable(BL)  Enable the 64-bit block_id64 for GVSP. This bit cannot be reset if the
                                stream channels do not support the standard ID mode (i.e. EI field is always 0).
                                Note: If a transmitter supports 16-bit block_id, as reported by bit 1
                                (legacy_16bit_block_id_supported) of the GVSP Capability register (at
                                address 0x092C), then it SHOULD have this bit set to 0 as the factory default to
                                start in 16-bit block_id mode. Otherwise, it cannot be advertised as a GigE
                                Vision 1.x device.
                                Note: When 16-bit block_ids are used, then 24-bit packet_ids must be used.
                                When 64-bit block_id64s are used, then 32-bit packet_id32s must be used.
2–31    reserved               Always 0. Ignore when reading.
*/
#define GVR_GVSP_CONFIGURATION              0x0960

/*
Physical Link Configuration Capability Register
This register indicates the physical link configuration supported by this device.
[R-475cd] A device MUST implement the Physical Link Configuration Capability register.
Length: 4 bytes
Access type :Read
Bits            Name                        Description
0–27       reserved Always 0.          Ignore when reading.
28         dynamic_link_aggregation_group_configuration(dLAG)
                                        This device supports dynamic LAG configuration.
29        static_link_aggregation_group_configuration(sLAG)
                                        This device supports static LAG configuration.
30 multiple_links_configuration (ML)    This device supports multiple link (ML) configuration.
31  single_link_configuration (SL)      This device supports single link (SL) configuration.
*/
#define GVR_PHYSICAL_LINK_CONFIG_CAPABILITY 0x0964

/*
Physical Link Configuration Register
This register indicates the principal physical link configuration currently enabled on this device. It should be
used in conjunction with the Physical Link Configuration Capability register to determine which values
are valid. After a change in configuration, it is necessary to restart the device (either by power cycling the
device or by some other mean) in order for the new setting to take effect.
[R-476cd] A device MUST implement the Physical Link Configuration register.
Length :4 bytes
Access type :Read/Write
  Bits          Name                                    Description
0 – 29     reserved                                Always 0. Ignore when reading.
30 - 31     link_configuration Principal            Physical Link Configuration to use on next restart/power-up of the device.
                                                    0: Single Link Configuration
                                                    1: Multiple Links Configuration
                                                    2: Static LAG Configuration
                                                    3: Dynamic LAG Configuration
                                                    Note: IP configuration is not sufficient for the device to use the new link
                                                    configuration. Hence FORCEIP_CMD cannot be used to use the new link
                                                    configuration settings.

*/
#define GVR_PHYSIC_LINK_CONFIG              0x0968

/*
IEEE 1588 Status Register
This optional register indicates the state of the IEEE 1588 clock.
[CR-477cd] If a device supports IEEE 1588, then it MUST implement the IEEE 1588 Status
register.
Length: 4 bytes
Access type :Read
 Bits       Name                                Description
0–27     reserved                           Always 0. Ignore when reading.
28–31  clock_status                         Provides the state of the IEEE 1588 clock. Values of this field must match the IEEE
                                            1588 PTP port state enumeration (INITIALIZING, FAULTY, DISABLED,
                                            LISTENING, PRE_MASTER, MASTER, PASSIVE, UNCALIBRATED, SLAVE).
                                            Please refer to IEEE 1588 for additional information.
*/
#define GVR_IEEE_1588_STATUS                0x096C

/*
Scheduled Action Command Queue Size Register
This optional register reports the number of Scheduled Action Commands that can be queued at any given
time. This is essentially the size of the queue. This register is present when bit 14
(scheduled_action_command) of the GVCP Capability register is set
[CR-478cd] If a device supports Scheduled Action Commands, then it MUST implement the
Scheduled Action Command Queue Size register.
Length :4 bytes
Access type: Read
Bits            Name            Description
0 – 31     queue_size      Indicates the size of the queue. This number represents the maximum
                            number of Scheduled Action Commands that can be pending at a given
                            point in time.

*/
#define GVR_SCHEDULED_AC_QUEUE_SIZE         0x0970

/*
Control Channel Privilege Register (CCP)   同一时刻只能被一个应用程序占用，其它应用程序有访问的权限
This register is used to grant privilege to an application. Only one application is allowed to control the
device. This application is able to write into device’s registers. Other applications can read device’s register
only if the controlling application does not have the exclusive privilege.
[R-479cd] A device MUST implement the Control Channel Privilege register.

Since exclusive access is more restrictive than control access (without or with switchover enabled), the
device must be in exclusive access mode if the exclusive_access bit is set independently of the state of the
control_access and control_switchover_enable bits. Control access is defined according to Table 28-2.

Table 28-2: Control Access Definition
control_switchover_enable   control_access      exclusive_access        Access Mode
        x                        0                      0               Open access.
        0                        1                      0               Control access.
        1                        1                      0               Control access with switchover enabled.
        x                        x                      1               Exclusive access.  独占


Length :4 bytes
Access type :Read/Write

  Bits              Name                                      Description
0 – 15     control_switchover_key                  This field is applicable only when the device supports the primary
                                                    application switchover capability. It is taken into account only when the
                                                    control_switchover_enable bit (bit 29) was set prior to the current
                                                    transaction writing to the CCP register and the transaction comes from a
                                                    different application. In this case, the primary application switchover will
                                                    occur only if the value written to this field matches the value in the Control
                                                    Switchover Key bootstrap register.
                                                    No matter if the primary application switchover capability is supported or
                                                    not, this field always reads as zero.
16 – 28            reserved                        Always 0. Ignore when reading.
   29    control_switchover_enable(CSE)             This bit is applicable only when the device supports the primary application
                                                    switchover capability. The application writing a 1 to this bit enables another
                                                    application to request exclusive or control access to the device (without or
                                                    with switchover enabled). This bit is used in conjunction with the
                                                    control_access bit (bit 30). It is “don’t care” when the exclusive_access bit
                                                    (bit 31) is set.
                                                    This bit always reads as zero when the primary application switchover
                                                    capability is not supported.
   30       control_access (CA)                     The application writing a 1 to this bit requests control access to the device
                                                    (without or with switchover enabled depending of the value written to the
                                                    control_switchover_enable bit). If a device grants control access, no other
                                                    application can control the device if the control_switchover_enable bit is
                                                    set to zero. Otherwise, another application can request to take over
                                                    exclusive or control access of the device (without or with switchover
                                                    enabled). Other applications are still able to monitor the device.
   31       exclusive_access (EA)                   The application writing a 1 to this bit requests exclusive access to the
                                                    device. If a device grants exclusive access, no other application can control
                                                    or monitor the device. exclusive_access has priority over control_access
                                                    when both bits are set.

control_switchover_enable  control_access  exclusive_access  Access Mode
            x                       0               0           Open access.
            0                       1               0           Control access.
            1                       1               0           Control access with switchover enabled.
            x                       x               1           Exclusive access.


*/
#define GVR_CONTROL_CHANNEL_PRIVILEGE       0x0A00


/*
Primary Application Port Register
This optional register provides UDP port information about the primary application holding the control
channel privilege.
[O-480cd] A device SHOULD implement the Primary Application Port register.

  Bits          Name                            Description
0 – 15         reserved                    Always 0. Ignore when reading.
16 – 31  primary_application_port          The UDP source port of the primary application. This value must be 0 when
                                            no primary application is bound to the device (CCP register equal to 0).
Length: 4 bytes
Access type :Read
主程序登陆到相机时，主程序的端口号
*/
#define GVR_PRIMARY_APP_PORT                0x0A04

/*
Primary Application IP Address Register
This optional register provides IP address information about the primary application holding the control
channel privilege.
[O-481cd] A device SHOULD implement the Primary Application IP Address register.
  Bits          Name                                Description
0 – 31   primary_application_IP_address        The IPv4 address of the primary application. This must be a unicast address.
                                                This value must be 0 when no primary application is bound to the device
                                                (CCP register equal to 0).
Length :4 bytes
Access type: Read
主程序登陆到相机时，主程序的IP
*/
#define GVR_PRIMARY_APP_IP                  0x0A14

/*
Message Channel Port Register (MCP)
This optional register provides port information about the message channel.
[CR-482cd] If a device supports a Message Channel, then it MUST implement the Message
Channel Port register.

  Bits            Name                    Description
0 – 11         reserved            Always 0. Ignore when reading.
12 – 15  network_interface_index   Always 0 in this version. Only the primary interface (#0) supports GVCP.
16 – 31       host_port            The port to which the device must send messages. Setting this value to 0
                                    closes the message channel.    从这个端口发送消息，为0时表示消息端口关闭

Address 0x0B00 [optional]
Length :4 bytes
Access type :Read/Write
*/
#define GVR_MESSAGE_CHANNEL_PORT            0x0B00

/*
Message Channel Destination Address Register (MCDA)
This optional register indicates the destination IP address for the message channel.
[CR-483cd] If a device supports a Message Channel, then it MUST implement the Message
Channel Destination Address register.

Bits            Name                    Description
0 – 31     channel_destination_IP      Message channel destination IPv4 address. The destination address can be a
                                        multicast or a unicast address.
										消息的终点地址，可以是广播地址，也可以是单播

Address 0x0B10 [optional]
Length :4 bytes
Access type :Read/Write
*/
#define GVR_MESSAGE_CHANNEL_DESTI_ADDR      0x0B10

/*
Message Channel Transmission Timeout Register (MCTT)
This optional register provides the transmission timeout value in milliseconds.
[CR-484cd] If a device supports a Message Channel, then it MUST implement the Message
Channel Transmission Timeout register.
[CR-485cd] This register indicates the amount of time the device MUST wait for acknowledge after a
message is sent on the message channel before timeout when an acknowledge is
requested when a message channel is supported. [CR27-32cd]

Address 0x0B14 [optional]
Length :4 bytes
Access type :Read/Write

Bits        Name                        Description
0 – 31    timeout      Transmission timeout value in ms. Set to 0 to disable acknowledge on message channel.
*/
#define GVR_MESSAGE_CHANNEL_TRANS_TIMEOUT   0x0B14

/*
Message Channel Retry Count Register (MCRC)
This optional register indicates the number of retransmissions allowed when a message channel message
times out.
[CR-486cd] If a device supports a Message Channel, then it MUST implement the Message
Channel Retry Count register.

Address 0x0B18 [optional]
Length: 4 bytes
Access type :Read/Write

Bits        Name            Description
0 – 31  retry_count     Number of retransmissions allowed on the message channel.
						重传次数
*/
#define GVR_MESSAGE_CHANNEL_RETRY_COUNT     0x0B18

/*
Message Channel Source Port Register (MCSP)
This optional register indicates the source UDP port for the message channel. The purpose of this
information is to allow the host application to take measures to ensure asynchronous event traffic from the
device is allowed back to the host. The expected usage of this is to send dummy UDP packets from the
application’s message channel reception port to that device’s source port, thus making the message channel
traffic appear to be requested by the application instead of unsolicited.
[CO-487cd] If a device supports a Message Channel, then it SHOULD implement the Message
Channel Source Port register.

Address 0x0B1C [optional]
Length :4 bytes
Access type :Read-Only

Bits        Name                Description
0 - 15  reserved            Always 0. Ignore when reading.
16 - 31 source_port         Indicates the UDP source port message channel traffic will be
                            generated from while the message channel is open.

*/
#define GVR_MESSAGE_CHANNEL_SRC_PORT        0x0B1C

/*
Stream Channel Port Registers (SCPx)
These registers provide port information for the given stream channel.
[CR-488cd] If a device supports stream channels, it MUST implement the Stream Channel Port
register for all of its supported stream channels.
Address 0x0D00 + 0x40 * x
with 0 ≤ x < 512.
[optional for 0 < x < 512 for GVSP transmitters and receivers]
[Not applicable for peripherals]
Length :4 bytes
Access type :Read/Write

 Bits               Name                            Description
   0              direction (D)         This bit is read-only and reports the direction of the stream channel. It must
                                        take one of the following values
                                        0: Transmitter
                                        1: Receiver
1 – 11            reserved             Always 0. Ignore when reading.
12–15    network_interface_index       Index of network interface to use (from 0 to 3). Specific streams might be
                                        hard-coded to a specific network interfaces. Therefore, this field might not
                                        be programmable on certain devices. It is read-only for this case.
                                        For link aggregation configuration, only a single network interface is made
                                        visible by the device.
16 - 31           host_port             The port to which a GVSP transmitter must send data stream. The port from
                                        which a GVSP receiver may receive data stream. Setting this value to 0
                                        closes the stream channel. For a GVSP transmitter, the stream channel
                                        block ID must be reset to 1 when the stream channel is opened.

*/
#define GVR_STREAM_CHANNEL_PORT             0x0D00

/*
Stream Channel Packet Size Registers (SCPSx)
These registers indicate the packet size in bytes for the given stream channel. This is the total packet size,
including all headers (IP header = 20 bytes, UDP header = 8 bytes and GVSP header). For GVSP
transmitters, they also provide a way to set the IP header “don’t fragment” bit and to send stream test packet
to the GVSP receiver.

    [CR-489cd] If a device supports stream channels, it MUST implement the Stream Channel Packet
    Size register for all of its supported stream channels.
    Write access to this register is not possible while streaming (transmission) is active on this channel
    The packet size is used by the application to determine where it has to copy the data in the receiving buffer.
    As such, it is important that this value remains the same once it has been configured by the application.

    [R-490st] A GVSP transmitter is allowed to round the value of the packet_size field to the nearest
    supported value when the application writes to the SCPSx register. But the transmitter
    MUST not change the packet size value without having the application directly writing
    into this register.

For instance, it is allowed for the GVSP transmitter to round a packet size value to a multiple of 4 bytes, but
it is not allowed for the transmitter to change the packet size if the application changes the data format (such
as the image width or pixel format for a camera device).

It is recommended for the application to read back the packet size after writing to SCPSx since the value
could have been rounded by the GVSP transmitter. The application should also ensure the packet size
contains a valid value before asking for a Test Packet.

Address 0x0D04 + 0x40 * x
with 0 ≤ x < 512.
[optional for 0 < x < 512 for GVSP transmitters and receivers]
[Not applicable for peripherals]
Length :4 bytes
Access type :Read/Write

  Bits              Name                                    Description
   0         fire_test_packet (F)                   When this bit is set and the stream channel is a transmitter, the GVSP
                                                    transmitter will fire one test packet of size specified by bits 16 to 31. The
                                                    “don’t fragment” bit of IP header must be set for this test packet. If the
                                                    GVSP transmitter supports the LFSR generator, then the test packet
                                                    payload will be filled with data according to this polynomial. Otherwise,
                                                    the payload data is “don’t care”.
                                                    When the stream channel is a receiver, this bit doesn’t have any effect and
                                                    always reads as zero.
   1        do_not_fragment (D)                     For GVSP transmitters, this bit is copied into the “don’t fragment” bit of IP
                                                    header of each stream packet. It can be used by the application to prevent IP
                                                    fragmentation of packets on the stream channel.
                                                    When the stream channel is a receiver, this bit doesn’t have any effect and
                                                    always reads as zero.
   2        pixel_endianess (P)                     Endianess of multi-byte pixel data for this stream.
                                                    0: little-endian
                                                    1: big-endian
                                                    This is an optional feature. A GVSP transmitter or receiver that does not
                                                    support this feature must support little-endian and always leave that bit
                                                    clear.
3 – 15         reserved                            Always 0. Ignore when reading.
16 – 31        packet_size                         For GVSP transmitters, this field represents the stream packet size to send
                                                    on this channel, except for data leader and data trailer; and the last data
                                                    packet which might be of smaller size (since packet size is not necessarily a
                                                    multiple of block size for stream channel). The value is in bytes. For a
                                                    camera, it can be accessed through the GevSCPSPacketSize mandatory
                                                    feature.
                                                    If a GVSP transmitter cannot support the requested packet_size, then it
                                                    must not fire a test packet when requested to do so.
                                                    For GVSP receivers, this field represents the maximum GVSP packet size
                                                    supported by the receiver. It is read-only.
*/
#define GVR_STREAM_CHANNEL_PACK_SIZE        0x0D04

/*
Stream Channel Packet Delay Registers (SCPDx)
These registers indicate the minimal delay (in timestamp counter unit) to insert between each packet
transmitted on a given physical link for the stream channel. This can be used as a crude flow-control
mechanism if the GVSP receiver cannot keep up with the packets coming from the device.
Note: The above essentially means that each network interface must have a separate timer per stream to
measure the inter-packet delay.
As an addition to the inter-packet delay, a device might elect to support the PAUSE option of IEEE 802.3.
[CR-491cd] If a device supports stream channels, it MUST implement the Stream Channel Packet
Delay register for all of its supported stream channels.
This delay normally uses the same granularity as the timestamp counter to ensure a very high precision in
the packet delay. This counter is typically of very high frequency. Inter-packet delay is typically very small.
If the timestamp is not supported, then this register has no effect.

Address 0x0D08 + 0x40 * x
with 0 ≤ x < 512.
[optional for 0 < x < 512 for GVSP transmitters]
[Not applicable for GVSP receivers and peripherals]
Length :4 bytes
Access type :Read/Write
Bits        Name        Description
0 – 31    delay        Inter-packet delay in timestamp ticks.
*/
#define GVR_STREAM_CHANNEL_PACKET_DELAY     0x0D08

/*
Stream Channel Destination Address Registers (SCDAx)
For GVSP transmitters, these registers indicate the destination IP address for the given stream channel. For
GVSP receivers, these registers indicate the destination IP address from which the given receiver may
receive data stream from.
[R-492cd] If a device supports stream channels, it MUST implement the Stream Channel
Destination Address register for all of its supported Stream Channels.
For GVSP transmitters, write access to this register is not possible while streaming is active on this channel.

Address :0x0D18 + 0x40 * x
with 0 ≤ x < 512.
[optional for 0 < x < 512 for GVSP transmitters and receivers]
[Not applicable for peripherals]
Length :4 bytes
Access type: Read/Write
Factory Default 0 if transmitter (SCPx_direction field is set to 0)
Device-specific if receiver (SCPx_direction field is set to 1)

  Bits         Name                         Description
0 – 31   channel_destination_IP        Stream channel destination IPv4 address. The destination address can be a
                                        multicast or a unicast address.
*/
#define GVR_STREAM_CHANNEL_DEST_ADDR        0x0D18

/*
Stream Channel Source Port Registers (SCSPx)
These optional registers indicate the source UDP port for the given GVSP transmitter stream channel. The
purpose of this information is to allow the host application to take measures to ensure streaming traffic from
the device is allowed back to the GVSP receiver. The expected usage of this is to send dummy UDP packets
from the GVSP receiver stream reception port to that GVSP transmitter source port, thus making the
streaming traffic appear to be requested by the GVSP receiver instead of unsolicited.
[CO-493cd] If a device supports stream channels, it SHOULD implement the Stream Channel
Source Port register for all of its supported stream channels.

Address:0x0D1C + 0x40 * x
with 0 ≤ x < 512.
[optional for 0 ≤ x < 512 for GVSP transmitters]
[Not applicable for GVSP receivers and peripherals]
Length :4 bytes
Access type :Read-Only
Factory Default :0 if unavailable

  Bits      Name                    Description
0 - 15    reserved Always 0.        Ignore when reading.
16 - 31   source_port Indicates     the UDP source port GVSP traffic will be generated from while the
                                    streaming channel is open.

*/
#define GVR_STREAM_CHANNEL_SRC_PORT         0x0D1C

/*
Stream Channel Capability Registers (SCCx)
These optional registers provide a list of capabilities specific to the given stream channel.
[CO-494cd] If a device supports stream channels, it SHOULD implement the Stream Channel
Capability register for all of its supported stream channels.

Address 0x0D20 + 0x40 * x
with 0 ≤ x < 512.
[optional for 0 ≤ x < 512 for GVSP transmitters and receivers]
[Not applicable for peripherals]
Length 4 bytes
Access type Read
Factory Default Device-specific

 Bits       Name                            Description
  0 big_and_little_endian_supported (BE)    Indicates this stream channel supports both big and little-endian. Note that all
                                            stream channels must support little-endian.
  1  IP_reassembly_supported (R)            For GVSP receivers, indicates this stream channel supports the reassembly of
                                            fragmented IP packets. Otherwise, it reads as zero.
 2-26     reserved                          Always 0. Ignore when reading.
  27   multi_zone_supported(MZ)             Set to 1 to indicate this stream channel supports the SCZx and SCZDx
                                            registers.
  28 packet_resend_destination_option (PRD) Indicates if an alternate destination exists for Packet resend request. This is only
                                            valid for GVSP transmitter.
                                            For GVSP transmitter:
                                            0: No alternative, always use the destination specified by SCDAx.
                                            1: Use the destination indicated by the PRD flag of the SCCFGx register.
                                            For GVSP receiver:
                                            must be 0.
  29   all_in_transmission_supported (AIT)  For GVSP transmitters, indicates that the stream channel supports the All-in
                                            Transmission mode. Otherwise, it reads as zero.
  30 unconditional_streaming_supported (US) For GVSP transmitters, indicates that the stream channel supports unconditional
                                            streaming capabilities. Otherwise, it reads as zero.
  31  extended_chunk_data_supported (EC)    Indicates that the stream channel supports the deprecated extended chunk data
                                            payload type. Otherwise, it reads as zero.
*/ // 0x00000000
#define GVR_STREAM_CHANNEL_CAPABILITY       0x0D20

/*
Stream Channel Configuration Registers (SCCFGx)
These optional registers provide additional control over optional features specific to the given stream
channel. These optional features must be indicated by the Stream Channel Capability register of the given
stream channel.
For instance, they can be used to enable GVSP transmitters to use the All-in Transmission mode.
[CO-495cd] If a device supports stream channels, it SHOULD implement the Stream Channel
Configuration register for all of its supported stream channels.

Address 0x0D24 + 0x40 * x
with 0 ≤ x < 512.
[optional for 0 ≤ x < 512 for GVSP transmitters and receivers]
[Not applicable for peripherals]
Length 4 bytes
Access type Read/Write
Factory Default 0

  Bits              Name                                  Description
0 – 27           reserved                              Always 0. Ignore when reading.
  28     packet_resend_destination (PRD)                Enable the alternate IP destination for stream packet resent due to a Packet
                                                        Resend request. This is only valid for GVSP transmitter.
                                                        For GVSP transmitter:
                                                        0: Use SCDAx
                                                        1: Use the source IP address provided in the PACKETRESEND_CMD packet
                                                        For GVSP receiver:
                                                        don’t care.
  29     all_in_transmission_enabled (AIT)              Enable GVSP transmitter (direction field of SCPx set to 0) to use the single
                                                        packet per data block All-in Transmission mode.
  30    unconditional_streaming_enabled (US)            Enable GVSP transmitters (direction field of SCPx set to 0) to continue to
                                                        stream if the control channel of its associated device is closed or regardless of
                                                        any ICMP messages, such as the destination unreachable messages, received by
                                                        the device associated to them. This bit has no effect for GVSP receivers
                                                        (direction field of SCPx set to 1). In the latter case, it always reads back as
                                                        zero.
  31    extended_chunk_data_enable (EC)                 Enable GVSP transmitters (direction field of SCPx set to 0) to use the
                                                        deprecated extended chunk data payload type. This bit has no effect for GVSP
                                                        receivers (direction field of SCPx set to 1). In the latter case, it always reads
                                                        back as zero.
*/
#define GVR_STREAM_CHANNEL_CONFIG           0x0D24

/*
Stream Channel Zone Registers (SCZx)
These optional registers indicate the number of zones in a Multi-zone Image data block transmitted on the
corresponding GVSP transmitter stream channels.
This register is available when bit 27 of the corresponding SCCx register is set.
[CR-496cd] If a device supporting stream channels also supports the Multi-zone Image payload type,
then it MUST implement the Stream Channel Zone register for each stream channel
supporting the Multi-zone Image payload type.

Address 0x0D28 + 0x40 * x
with 0 ≤ x < 512.
[optional for 0 ≤ x < 512 for GVSP transmitters]
[Not applicable for GVSP receivers and peripherals]
Length 4 bytes
Access type Read-Only
Factory Default Device-specific

  Bits              Name                            Description
0 – 26         reserved                            Always 0. Ignore when reading.
27 – 31    additional_zones                        Reports the number of additional zones in a block transmitted on the
                                                    corresponding stream channel. The number of zones is equal to the value of this
                                                    field plus one.
*/
#define GVR_STREAM_CHANNEL_ZONE             0x0D28

/*
Stream Channel Zone Direction Registers (SCZDx)
These optional registers indicate the transmission direction of each zone in a Multi-zone Image data block
transmitted on the corresponding GVSP transmitter stream channels.
This register is available when bit 27 of the corresponding SCCx register is set.
[CR-497cd] If a device supporting stream channels supports the Multi-zone Image payload type, the it
MUST implement the Stream Channel Zone Direction register for each stream
channel supporting the Multi-zone Image payload type.

Address 0x0D2C + 0x40 * x
with 0 ≤ x < 512.
[optional for 0 ≤ x < 512 for GVSP transmitters]
[Not applicable for GVSP receivers and peripherals]
Length 4 bytes
Access type Read-Only
Factory Default Device-specific

  Bits              Name                                Description
0 – 31 zone_transmission_direction (Zx)        Reports the transmission direction of each zone.
                                                bit 0 (msb) - Direction of zone 0. Reports the transmission direction of zone ID
                                                0. When set to zero, the zone is transmitted top-down. Otherwise, it is
                                                transmitted bottom up.
                                                bit 1 - Direction of zone 1. Reports the transmission direction of zone ID 1.
                                                When set to zero, the zone is transmitted top-down. Otherwise, it is transmitted
                                                bottom up.
                                                …
                                                bit 31 (lsb) - Direction of zone 31. Reports the transmission direction of zone
                                                ID 31. When set to zero, the zone is transmitted top-down. Otherwise, it is
                                                transmitted bottom up.

*/
#define GVR_STREAM_CHANNEL_ZONE_DIRECTION  0x0D2C

/*
Manifest Table
The optional Manifest Table starts with a header indicating the number of entries in the table followed by a
list of entries, each describing one document. This table is only present if the manifest_table capability bit
(bit 5) is set in the GVCP Capability register (at address 0x0934).
The Manifest Table is optional for a GigE Vision device. Even if this Manifest table is present, the device
must provide valid entries in the First URL and Second URL registers.
[O-498cd] A device SHOULD implement the Manifest Table.
[R-499ca] An application MUST support Manifest Tables.
For 8-byte entries in the Manifest Table, the application needs to access the high part (bit 0 to 31, lowest
address) before accessing the low part (bit 32 to 63, highest address).

Address 0x9000 [optional]
Length 512 bytes
Access type Read
Factory Default Device-specific

Address         Name           Support   Type  Length(bytes)           Description
0x9000     ManifestHeader        M*       R      8           Header of the manifest table0x9000+8 ManifestEntry_1 M* R 8 First
                                                             entry in the manifest table
0x9000+8    ManifestEntry_1      M*       R      8           First entry in the manifest table
0x9000+16   ManifestEntry_2      O        R      8           Second entry in the manifest table
…               …              …       …     …             …
0x9000+8*N  ManifestEntry_N      O        R      8           Entry N in the manifest table
…               …               …      …     …             ...
0x9000+8*63 ManifestEntry_63     O        R      8           Last entry in the manifest table

More info about Manifest,see Gige Vision Specification 2.0 pdf [28.60 Manifest Table]
*/
#define GVR_MANIFEST_TABLE_ADDR             0x9000



/* gige vision XML Description File Mandatory Features */
//这部分的寄存器是gige vision中规定必须要在XML中支持的，地址由用户自定义，与XML中的地址保持一致

/*
Feature Name 			Interface 		   Access 		Units 					Description
“Width” 				IInteger 			R/(W) 		pixels 			Width of the image output by the camera on the stream channel.

“Height” 				IInteger 			R/(W) 		pixels 			Height of the image output by the camera on the stream channel.
                                                                        For linescan sources, this represents the height of the frame created
                                                                        by combining consecutive lines together.

“PixelFormat” 		IEnumeration 		R/(W) 		N/A 			Format of the pixel output by the camera on the stream channel. Refer
                                                                        to the Pixel format section (p.238)

“PayloadSize” 		IInteger 			R 			Bytes 			Maximum number of bytes transferred for eachimage on the stream channel,
                                                                        including any end-ofline,end-of-frame statistics or other stamp data. This
                                                                        is the maximum total size of data payload for a	block. UDP and GVSP headers
                                                                        are not considered.
                                                                        Data leader and data trailer are not included.
                                                                        This is mainly used by the application software to determine size of image
                                                                        buffers to allocate (largest buffer possible for current mode of operation).
                                                                        For example, an image with no statistics or stamp data as PayloadSize equals
                                                                        to (width x height x pixel size) in bytes. It is strongly recommended to
                                                                        retrieve PayloadSize from the camera instead of relying on the above formula.

“GevSCPSPacketSize” 	IInteger 			R/W 		bytes 			Size of the GVSP packets to transmit during acquisition. This reflects the
                                                                        packet_size field of the SCPS register associated to this stream channel.
                                                                        This feature MUST explicitly state the minimum,maximum and increment value
                                                                        supported by the camera for GevSCPSPacketSize.

“AcquisitionMode” 	IEnumeration 		R/(W) 		N/A         Used by application software to set the mode of acquisition. This feature indicates
                                                                        how image are sequenced out of the camera (continuously, single shot, multi-shot, …)
                                                                        Only a single value is mandatory for GigE Vision.{“Continuous”}
                                                                        Continuous: Configures the camera to stream an infinite sequence of images that must
                                                                        be stopped explicitly by the application.
                                                                        Note that the AcquistionMode can have more than this single entry. However, only this
                                                                        one is mandatory.
                                                                        The “AcquisitionStart” and “AcquisitionStop”features are used to control the
                                                                        acquisition state.

“AcquisitionStart” 	ICommand 			(R)/W 		N/A         Start image acquisition using the specified acquisition mode.

“AcquisitionStop” 	ICommand 			(R)/W 		N/A         Stop image acquisition using the specified acquisition mode.

*/

//O R/W 4 First action signal group key
#define GVR_REG_ActionGroupKey0                                  0x9800
//O R/W 4 First action signal group mask
#define GVR_REG_ActionGroupMask0                                 0x9804
//0x9808 Reserved - - -//O R/W 4 Second action signal group key
#define GVR_REG_ActionGroupKey1                                  0x9810
//O R/W 4 Second action signal group mask
#define GVR_REG_ActionGroupMask1                                 0x9814

#define GVR_REG_ActionGroupKey(n)                                (0x9800+(n)*0x10)
#define GVR_REG_ActionGroupMask(n)                               (0x9804+(n)*0x10)

//0x9818 Reserved - - -//O R/W - Each action signal is allocated a section of 16 bytes(0x10). Only supported action signals are available.
#define GVR_REG_OtherActionSignalsRegisters                      0x9820
//O R/W 4 128th action signal group key
#define GVR_REG_ActionGroupKey127                                0x9FF0
//O R/W 4 128th action signal group mask
#define GVR_REG_ActionGroupMask127                               0x9FF4


#define GVR_GEV_SCPS_PACKET_SIZE								GVR_STREAM_CHANNEL_PACK_SIZE



/* 自定义的地址从 0x1000000开始 */
#define GVR_IMG_WIDTH                   0x1000000
#define GVR_IMG_HEIGHT                  0x1000004
#define GVR_PIXEL_FORMAT                0x1000008
#define GVR_PAYLOAD_SIZE                0x100000C
#define GVR_ACQUISITION_MODE            0x1000010
#define GVR_ACQUISITION_START           0x1000014
#define GVR_ACQUISITION_STOP            0x1000018
#define GVR_OFFSET_X                    0x100001c
#define GVR_OFFSET_Y                    0x1000020
#define GVR_REVERSE_X                   0x1000024
#define GVR_REVERSE_Y                   0x1000028
#define GVR_PIXEL_RATE                  0x100002c
#define GVR_RESOLUTION_MODE             0x1000030
#define GVR_RESOLUTION_SEL              0x1000034
#define GVR_DROP_CASE                   0x1000040
#define GVR_ACQUISITION_FRAMES          0x1000050
#define GVR_ACQUISITION_COUNT           0x1000054
#define GVR_SENSOR_WIDTH                0x1000060
#define GVR_SENSOR_HEIGHT               0x1000064
#define GVR_IMG_WIDTH_MAX               0x1000070
#define GVR_IMG_HEIGHT_MAX              0x1000074
#define GVR_IMG_WIDTH_STEP              0x1000078
#define GVR_IMG_HEIGHT_STEP             0x100007C
#define GVR_TEST_IMAGE_SELECTOR         0x1000080


/* 曝光和增益 */
#define GVR_EXPOSURE_MODE               0x10000FC
#define GVR_EXPOSURE_TIME               0x1000100
#define GVR_ANTIFLICK                   0x100011c
#define GVR_ANALOG_GAIN                 0x1000120
#define GVR_GAIN_AUTO                   0x1000124   // GainAuto设置, 0:OFF, 1:ONCE, 2:Continuous
#define GVR_EXPOSURE_AUTO               0x1000130   // ExposureAuto设置, 0:OFF, 1:ONCE, 2:Continuous
#define GVR_AE_PRI                      0x1000134   // 0:曝光时间优先，1:增益优先
#define GVR_AE_TARGET                   0x1000138   // 亮度目标值
#define GVR_AE_EXPOSURE_MIN             0x1000140
#define GVR_AE_EXPOSURE_MAX             0x1000144
#define GVR_AE_GAIN_MIN                 0x1000148
#define GVR_AE_GAIN_MAX                 0x100014C
#define GVR_ANALOG_GAIN_MIN             0x1000150
#define GVR_ANALOG_GAIN_MAX             0x1000154
#define GVR_ANALOG_GAIN_STEP            0x1000158
#define GVR_EXPOSURE_MIN                0x1000160
#define GVR_EXPOSURE_MAX                0x1000164
#define GVR_EXPOSURE_STEP               0x1000168
#define GVR_AE_ROI_X                    0x1000170
#define GVR_AE_ROI_Y                    0x1000174
#define GVR_AE_ROI_W                    0x1000178
#define GVR_AE_ROI_H                    0x100017C


/* 触发功能用户接口 */
#define GVR_TRIG_MODE                   0x1000180


/* 图像处理 */
#define GVR_BLACK_LEVEL                 0x1000200
#define GVR_GAMMA                       0x1000204
#define GVR_CONTRAST                    0x1000208
#define GVR_USER_LUT                    0x1000210 // User LUT, 还未实现
//#define GVR_USER_LUT_VALUE_LENGTH       0x200
//#define GVR_USER_LUT_VALUE_BUF          0x2200000 /* 此地址与物理地址无关 可以修改 */
//#define GVR_USER_LUT_VALUE_BUF_END      (GVR_USER_LUT_VALUE_BUF+GVR_USER_LUT_VALUE_LENGTH)

/* User Lut */
#define USER_LUT_SIZE                   0x100 // 256条记录

/* 使用GVR寄存器方式操作的地址空间， 1KB大小 */
#define GVR_USER_LUT_START              0x10F0000 
#define GVR_USER_LUT_LAST               (GVR_USER_LUT_START+(USER_LUT_SIZE-1)*4)

/* 使用GVR MEM方式操作的地址空间， 512B大小 */
#define GVR_USER_LUT_BUF_START          0x10F8000
#define GVR_USER_LUT_BUF_END            (GVR_USER_LUT_BUF_START+(USER_LUT_SIZE-1)*2)


#define GVR_SATURATION                  0x1000230 // 饱和度
#define GVR_GAIN_R                      0x1000234 // 数字增益
#define GVR_GAIN_G                      0x1000238
#define GVR_GAIN_B                      0x100023C
#define GVR_COL_TEMP                    0x1000240

#define GVR_NOISE_REDUCT                0x1000250 // 降噪
#define GVR_NOISE_REDUCT_3D             0x1000258 // 3D降噪
#define GVR_SHARPNESS                   0x1000260 // 锐度

#define GVR_AWB_OP						0x1000280 // AWB操作控制
#define GVR_AWB_ROI_X					0x1000290 // AWB统计的ROI
#define GVR_AWB_ROI_Y					0x1000294
#define GVR_AWB_ROI_W					0x1000298
#define GVR_AWB_ROI_H					0x100029c

#define GVR_CM_CTRL						0x10002a0

#define GVR_CM_GAINRR					0x10002b0
#define GVR_CM_GAINRG					0x10002b4
#define GVR_CM_GAINRB					0x10002b8

#define GVR_CM_GAINGR					0x10002bc
#define GVR_CM_GAINGG					0x10002c0
#define GVR_CM_GAINGB					0x10002c4

#define GVR_CM_GAINBR					0x10002c8
#define GVR_CM_GAINBG					0x10002cc
#define GVR_CM_GAINBB					0x10002d0



#define GVR_CM_R_OFFSET					0x10002d4
#define GVR_CM_G_OFFSET					0x10002d8
#define GVR_CM_B_OFFSET					0x10002dc


/* 存档相关 */
/* 保存参数，使用位定义，可以指定部分或全部存档 */
#define GVR_PARAM_SAVE                  0x1000300
#define GVR_PARAM_USERSET_SEL           0x1000310
#define GVR_PARAM_USERSET_LOAD          0x1000314
#define GVR_PARAM_USERSET_SAVE          0x1000318
#define GVR_PARAM_USERSET_DEFAULT       0x100031c

/* 相机有的处理能力 和 开关 */
#define GVR_CAMERA_CAPABILITY           0x1000340
#define GVR_CAMERA_CAPABILITY_CONTROL   0x1000344

/* GigEVision 版本 */
#define GVR_GEV_VERSION_MAJOR           0x1000350
#define GVR_GEV_VERSION_MINOR           0x1000350


/* Action CMD 使用临时的时间 */
#define GVR_TEMP_TIMER_HIGH             0x1000354
#define GVR_TEMP_TIMER_LOW              0x1000358
/* 时间同步服务器IP */
#define GVR_SYNC_TIMER_IP               0x100035C
#define GVR_SYNC_TIMER                  0x1000360
/* DeviceFirmwareVersion 有些软件需要 */
#define GVR_GEV_FIRMWARE_VERSION        0x1000370
/* PHY初始化花费的时间 和 开机启动的时间 单位 1/1280 秒 */
#define GVR_PHY_INIT_TIME               0x1000374
#define GVR_STARTUP_TIME                0x1000378

/* 调试信息输出到的IP */
#define GVR_DBG_INFO_IP                 0x1000380
#define GVR_DBG_INFO_PORT               0x1000384

/* CRITICAL调试信息 */
#define GVR_CRITICAL_DEBUG_CTRL         0x1000388
#define GVR_CRITICAL_DEBUG_ID           0x100038C

/* 单片机记录当前时间 */
#define GVR_CURRENT_TIME_HIGH           0x1000390
#define GVR_CURRENT_TIME_LOW            0x1000394

/* 控制PHY芯片复位 */
#define GVR_RESET_PHY                   0x1000398

/* DO3THINK驱动程序使用的变量 */
#define GVR_DSDEF_VALUE1                0x1000400
#define GVR_DSDEF_VALUE2                0x1000404
#define GVR_DSDEF_VALUE3                0x1000408
#define GVR_DSDEF_VALUE4                0x100040c

#define GVR_USERDEF_VALUE1              0x1000440
#define GVR_USERDEF_VALUE2              0x1000444
#define GVR_USERDEF_VALUE3              0x1000448
#define GVR_USERDEF_VALUE4              0x100044C

/* labview 2010~2012 visionPro 需要 */
#define GVR_USERDEF_SELECTOR            0x1000450
#define GVR_USERDEF_VALUE               0x1000454

/* AcionCmd 选择 */
#define GVR_REG_ACTIONSELECTOR          0x10004C0

/* 设备复位 */
#define GVR_DEVICE_RESET                0x1001000

/* Bootloader版本 */
#define GVR_BOOTLOADER_VER              0x1001004

/* 固件版本 */
#define GVR_FIRMWARE_VER                0x1001008

/* FPGA版本 */
#define GVR_FPGA_VER                    0x100100C

/* 工厂模式使能，工厂模式允许访问EEPROM所有空间 */
#define GVR_FACTORY_MODE                0x1001020

/* 基本模式使能，这个模式下不使用相机驱动，sensor控制完全由APP端实现 */
#define GVR_BASE_MODE                   0x1001024

/* MCU Flash ROM 状态 */
#define GVR_MCU_FLASH_STATUS            0x1001030

/* MCU Flash ROM 加锁/解锁 */
#define GVR_MCU_FLASH_LOCK              0x1001034

/* FPGA 配置芯片状态 */
#define GVR_FPGA_FLASH_STATUS           0x1001040

/* FPGA 配置芯片加锁/解锁 */
#define GVR_FPGA_FLASH_LOCK             0x1001044

/* FPGA2 配置芯片状态 */
#define GVR_FPGA2_FLASH_STATUS          0x1001048

/* FPGA2 配置芯片加锁/解锁 */
#define GVR_FPGA2_FLASH_LOCK            0x100104c

/* 物理层错误计数统计 */
#define GVR_PHY_LINK_ERROR              0x1001050

/* MDIO错误计数读取全0或者全1统计 */
#define GVR_PHY_MDIO_ERROR              0x1001054

/* 收到的GVCP重复命令计数 */
#define GVR_CMD_RESEND_CNT              0x1001058

/* 收到的网络(FIFO)包计数 */
#define GVR_CMD_MAC_FIFO_CNT            0x100105C

/* IP地址分配信息 */
#define GVR_IP_CONFIG_STATUS            0x1001060

/* MGS_PLUS新增SPI接收校验错误 */
#define GVR_SPI_RECV_ERROR              0x1001080

/* 获取当前设备授权状态 */
#define GVR_PERMISSION_STATUS           0x1001084


/* SENSOR总线选择，I2C/SPI总线 */
#define GVR_SENSOR_BUS                  0x1001100

/* SENSOR的器件地址，I2C总线 */
#define GVR_SENSOR_I2C_CONFIG           0x1001114

/* SENSOR的SPI总线配置 */
#define GVR_SENSOR_SPI_CONFIG           0x1001118

/* 与SENSOR相连的管脚控制，RST、PWDN等 */
#define GVR_SENSOR_CTRL_PIN             0x100111c

/* sensor调试，对sensor驱动发命令 */
#define GVR_SENSOR_DEBUG                0x1001120


/* sensor的MCLK，单位：Hz */
#define GVR_SENSOR_MCLK                 0x1001130

/* 数据流的流控制使能，PRE-SEND设置 */
#define GVR_STREAM_FLOW_CTRL            0x1001180

/* 数据流流控制的应答，请求的下一个Block ID和Pack ID */
#define GVR_STREAM_FC_ACK               0x1001184

/* 消点功能使能，使能后伴随着初始化(重新加载EEPROM) */
#define GVR_DEFECT_FIX                  0x10011d0


/* 消彩点数据更新 */
#define GVR_DEFECT_HOT_FIX_UPDATE       0x10011d4

/* 消死点数据更新 */
#define GVR_DEFECT_DEAD_FIX_UPDATE      0x10011d8


/* 平场功能使能，使能后伴随着初始化(重新加载EEPROM) */
#define GVR_FLAT_FIELD                  0x10011e0


/* 平场数据写入完毕 */
#define GVR_FLAT_FIELD_UPDATE			0x10011e4

/* 触发功能相关 */
#define GVR_TRIG_ACTIVATION             0x10011f4
#define GVR_TRIG_SOURCE                 0x10011f8
#define GVR_TRIG_SELECTOR               0x10011fc
#define GVR_TRIG_CTRL                   0x1001200
#define GVR_TRIG_IN_TYPE                0x1001204
#define GVR_TRIG_OUT_TYPE               0x1001208
#define GVR_STROBE_IN_TYPE              0x100120c
#define GVR_STROBE_OUT_TYPE             0x1001210
#define GVR_JITTER_FILTER               0x1001220
#define GVR_TRIG_DELAY                  0x1001224
#define GVR_TRIG_DURATION               0x1001228
#define GVR_TRIG_INTERVAL               0x100122c
#define GVR_STROBE_DRIVER               0x1001230
#define GVR_STROBE_DELAY                0x1001234
#define GVR_STROBE_DURATION             0x1001238
#define GVR_TRIG_CHECK_TIMEOUT          0x100123c
#define GVR_FRAMES_PER_TRIG             0x1001240
#define GVR_SOFT_TRIG_LOOP_CTRL         0x1001244
#define GVR_SOFT_TRIG_LOOP_PERIOD       0x1001248
#define GVR_SOFT_TRIG_FIRE              0x100124c
#define GVR_SNAP_DELAY                  0x1001250
#define GVR_TRIG_ID						0x1001254
#define GVR_EXT_TRIG_MODE               0x1001260



/* GPIO相关，暂时只有OUT1, IN1 */
#define GVR_GPIO_OUT1_PIN               0x1001280
#define GVR_GPIO_OUT1_CTRL              0x1001284
#define GVR_GPIO_OUT2_PIN               0x1001288
#define GVR_GPIO_OUT2_CTRL              0x100128c
#define GVR_GPIO_OUT3_PIN               0x1001290
#define GVR_GPIO_OUT3_CTRL              0x1001294
#define GVR_GPIO_OUT4_PIN               0x1001298
#define GVR_GPIO_OUT4_CTRL              0x100129c

#define GVR_GPIO_IN1_PIN                0x10012c0
#define GVR_GPIO_IN1_CTRL               0x10012c4
#define GVR_GPIO_IN2_PIN                0x10012c8
#define GVR_GPIO_IN2_CTRL               0x10012cc
#define GVR_GPIO_IN3_PIN                0x10012d0
#define GVR_GPIO_IN3_CTRL               0x10012d4
#define GVR_GPIO_IN4_PIN                0x10012d8
#define GVR_GPIO_IN4_CTRL               0x10012dc

#define GVR_LINE_SELECTOR               0x1001300
#define GVR_LINE_MODE                   0x1001304
#define GVR_LINE_INVERTER               0x1001308
#define GVR_LINE_STATUS                 0x100130c
#define GVR_LINE_STATUS_ALL             0x1001310
#define GVR_LINE_SOURCE                 0x1001314
#define GVR_USER_OUTPUT_SELECTOR        0x1001318
#define GVR_USER_OUTPUT_VALUE           0x100131c
#define GVR_USER_OUTPUT_VALUE_ALL       0x1001320

/* timer 相关 可用来实现PWM功能 */
#define GVR_TIMER_DURATION              0x1001380
#define GVR_TIMER_DEALY                 0x1001384
#define GVR_TIMER_TRIGGER_SOURCE        0x1001388

//#define GVR_TIMER_LOOP                  0x100133C
/* PWM 控制功能 */
#define GVR_PWM_CYCLE                   0x10013C0
#define GVR_PWN_DUTY                    0x10013C4


/* 温度计 */
/* 设备温度 */
#define GVR_DEV_TEMP                    0x1001400
/* sensor温度 */
#define GVR_SENSOR_TEMP                 0x1001404
/* chip1温度，一般是CPU */
#define GVR_CHIP1_TEMP                  0x1001408
/* chip2温度，一般是FPGA */
#define GVR_CHIP2_TEMP                  0x100140c
/* 降温控制 */
#define GVR_COOLER_CONTROL              0x1001410

#define GVR_SENSOR_REG                  0x1001500
#define GVR_SENSOR_REG_VALUE            0x1001504
#define GVR_SENSOR_REG_CONTROL          0x1001508

#define GVR_FPGA_REG                    0x1001510
#define GVR_FPGA_REG_VALUE              0x1001514
#define GVR_FPGA_REG_CONTROL            0x1001518

#define GVR_ADDR_BUF_INFIFO_ERR_CNT     0x1001520
#define GVR_ADDR_BUF_HDR_ERR_CNT        0x1001524
#define GVR_ADDR_BUF_ID_ERR_CNT         0x1001528
#define GVR_ADDR_BUF_CHKSUM_ERR_CNT     0x100152C
#define GVR_ADDR_FRAME_DROP_CNT         0x1001530
#define GVR_ADDR_DROP_CASE              0x1001534
#define GVR_ADDR_PACK_NUM               0x1001564
#define GVR_ADDR_BUFED_SIZE             0x1001538
#define GVR_ADDR_ORG_H_SIZE             0x100153C
#define GVR_ADDR_ORG_V_SIZE             0x1001540
#define GVR_ADDR_MAC_HDR_ERR_CNT        0x1001544
#define GVR_ADDR_MAC_CRC_ERR_CNT        0x1001548
#define GVR_ADDR_MAC_RX_CNT             0x100154C
#define GVR_ADDR_MAC_RX_PAUSE_CNT       0x1001550
#define GVR_ADDR_MAC_RX_DROP_CNT        0x1001554
#define GVR_ADDR_MAC_TX_CNT             0x1001558
#define GVR_ADDR_MAC_TX_PAUSE_CNT       0x100155C
#define GVR_ADDR_REFRESH_INFO           0x1001560
#define GVR_ADDR_RESEND_CNT             0x1001568

/* 产品信息, 映射256字节的MEM空间 */
#define    GVR_PRODUCT_INFO             0x1001800

/* 编译生成日期 */
#define GVR_BUILD_DATE                  0x1001b00

/* 保护信息，映射128直接的MEM空间 */
#define GVR_PROTECTED_INFO              0x1001c00


/* XML FILE 最大500K  0x1002000 */
#define GVR_LOCAL_XML_FILE_START        0x1002000
#define GVR_LOCAL_XML_FILE_END          0x107EFFF

/* EEPROM存储空间  最大32K */
#define GVR_EEPROM_ADDR_START           0x107F000
#define GVR_EEPROM_ADDR_END             (GVR_EEPROM_ADDR_START+0x7fff)

/* EEPROM的用户存储空间，EEPROM的最末4K */
#define GVR_EEPROM_USER_START           (GVR_EEPROM_ADDR_START+0x7000)
#define GVR_EEPROM_USER_END             GVR_EEPROM_ADDR_END

/* FPGA寄存器映射，256K空间 */
#define GVR_FPGA_MAPP_START             0x1100000
#define GVR_FPGA_MAPP_END               0x113ffff

/* 加密芯片寄存器映射，1K空间 */
#define GVR_APLU_MAPP_START             0x1200000
#define GVR_APLU_MAPP_END               0x12003ff

/* 解扰模块缓存，2K空间 */
#define GVR_DESCRAMBLE_PRE              0x1200800
#define GVR_DESCRAMBLE_START            0x1200c00
#define GVR_DESCRAMBLE_END              0x1200fff

/* SENSOR寄存器映射，256K空间 */
#define GVR_SENSOR_MAPP_START           0x1300000
#define GVR_SENSOR_MAPP_END             0x133ffff

/* FLASH芯片访问相关的地址范围段 */
#define GVR_FLASH_MAPP_START       		0x1400000
#define GVR_FLASH_MAPP_END         		0x15000ff

/* MCU Flash 操作前的预处理，只写 */
#define GVR_MCU_FLASH_OP_PRE            0x1400000

/* MCU Flash ROM 状态， 只读 */
#define GVR_MCU_FLASH_STATUS_2          0x1400004

/* MCU Flash ROM 加锁/解锁 ， 只写*/
#define GVR_MCU_FLASH_LOCK_2            0x1400008

/* MCU Flash ROM 擦除， 只写*/
#define GVR_MCU_FLASH_ERASE             0x140000C

/* MCU Flash 操作完成后的处理， 只写*/
#define GVR_MCU_FLASH_OP_POST           0x1400010

/* MCU Flash 存储空间映射，512k， 工厂模式下可读 写*/
#define GVR_MCU_FLASH_MEM_START       	0x1400020
#define GVR_MCU_FLASH_MEM_END         	0x1480020

/* FPGA 配置芯片操作前的处理， 只写 */
#define GVR_FPGA_FLASH_OP_PRE           0x1480030

/* FPGA 配置芯片状态， 只读 */
#define GVR_FPGA_FLASH_STATUS_2         0x1480034

/* FPGA 配置芯片加锁/解锁,只写 */
#define GVR_FPGA_FLASH_LOCK_2           0x1480038

/* FPGA 配置芯片擦除， 只写*/
#define GVR_FPGA_FLASH_ERASE            0x148003C

/* FPGA 配置芯片 操作完成后的处理， 只写*/
#define GVR_FPGA_FLASH_OP_POST          0x1480040

/* FPGA 配置芯片 存储空间映射，512k， 工厂模式下可读 写*/
#define GVR_FPGA_FLASH_MEM_START      	0x1480050


/* 新增文件传输的寄存器 */
#define GVR_FILE_TRANSFER_SEL           0x1600000
#define GVR_FILE_OP_MODE                0x1600004
#define GVR_FILE_OP_CODE                0x1600008
#define GVR_FILE_OP_EXECUTE             0x160000C
#define GVR_FILE_ACTUAL_SIZE            0x1600010
#define GVR_FILE_OP_RESULT              0x1600014
#define GVR_FILE_OP_RESULT_SIZE         0x1600018
#define GVR_FILE_ACCESS_LEGNTH          0x160001C
#define GVR_FILE_ACCESS_OFFSET          0x1600020
#define GVR_FILE_ACCESS_MAXLEN          0x100000
#define GVR_FILE_ACCESS_BUFFER          0x8100000
#define GVR_FILE_ACCESS_BUFFER_END      (GVR_FILE_ACCESS_BUFFER+GVR_FILE_ACCESS_MAXLEN)


#define GVR_DEFECT_HOT_FIX_MAXLEN       0x10000
#define GVR_DEFECT_HOT_FIX_START        0x2000000
#define GVR_DEFECT_HOT_FIX_END          (GVR_DEFECT_HOT_FIX_START+GVR_DEFECT_HOT_FIX_MAXLEN)

/* GVR 消死点EEPROM存档映射 4KB */
#define GVR_DEFECT_DEAD_FIX_MAXLEN      0x1000
#define GVR_DEFECT_DEAD_FIX_START       0x2010000
#define GVR_DEFECT_DEAD_FIX_END         (GVR_DEFECT_DEAD_FIX_START+GVR_DEFECT_DEAD_FIX_MAXLEN)

/* GVR 平场EEPROM存档映射 60KB */
#define GVR_FLAT_FIELD_MAXLEN           0xF000
#define GVR_FLAT_FIELD_START            0x2080000
#define GVR_FLAT_FIELD_END				(GVR_FLAT_FIELD_START + GVR_FLAT_FIELD_MAXLEN)


/* FPGA 配置芯片操作前的处理， 只写 */
#define GVR_FPGA2_FLASH_OP_PRE           0x2180030

/* FPGA 配置芯片状态， 只读 */
#define GVR_FPGA2_FLASH_STATUS_2         0x2180034

/* FPGA 配置芯片加锁/解锁,只写 */
#define GVR_FPGA2_FLASH_LOCK_2           0x2180038

/* FPGA 配置芯片擦除， 只写*/
#define GVR_FPGA2_FLASH_ERASE            0x218003C

/* FPGA 配置芯片 操作完成后的处理， 只写*/
#define GVR_FPGA2_FLASH_OP_POST          0x2180040

/* FPGA 配置芯片 存储空间映射，512k， 工厂模式下可读 写*/
#define GVR_FPGA2_FLASH_MEM_START      	 0x2180050

/* #define GVR_ */
#define PIX_OCCUPY(fmt)     8  /* u3v使用的宏，gige没有使用，防止出错 */

#endif /* __GIGE_H__ */
