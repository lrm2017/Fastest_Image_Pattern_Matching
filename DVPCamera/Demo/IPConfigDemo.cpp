/*****************************************************************************
* @FileName:IPConfigDemo.cpp
* @CreatTime: 2023/02/10
* @Descriptions: 简单的IP配置Demo
* @Version: ver 1.0
* @Copyright(c) 2020 Do3Think All Rights Reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "DVPCamera.h"		/* DVP API 依赖 */
#include <thread>


#define ENUM_DEV_MAX 32


// 用于IP配置的Text形式,文本格式一定要注意
typedef struct DsGlanIpConfigText_s
{
	char    	sn[64];					// 相机序列号(SN1)

	char		DeviceMac[18];			// 相机Mac地址"B4-61-D3-08-40-39"

	char		DeviceIP[16];			// 相机IP地址"192.168.10.242"

	char		DeviceNetMask[16];		// 相机子网掩码"255.255.255.0"

	char		DeviceGateway[16];		// 相机网关"192.168.10.1"

	char		Mode[16];				// persistent/dhcp/lla

	char		EthIP[16];				// 网卡IP"192.168.10.1"

	char		EthMac[18];				// 网卡MAC地址 
}DsGlanIpConfigText_t;


int32_t main(int32_t argc, char *argv[])
{
	int32_t i;
	int32_t t;
	int32_t iIpProblem;
	printf("start...\r\n");

	dvpUint32 count = 0;
	dvpCameraInfo info[ENUM_DEV_MAX];
	dvpStatus retStatus;

	/********************
	1、程序启动太快，库内部可能还没有枚举到设备，小睡一会儿；
	2、如果使用系统安装了过滤驱动，则可以不用小睡；
	********************/
	std::this_thread::sleep_for(std::chrono::seconds(3));


	/* 刷新在线设备 */
	dvpRefresh(&count);
	if (count > ENUM_DEV_MAX)
		count = ENUM_DEV_MAX;

	iIpProblem = 0;

	for (i = 0; i < count; i++)
	{   
		/* 按序号获取在线设备信息 */
		if (DVP_STATUS_OK == dvpEnum(i, &info[i]))
		{        
			printf("%s->%d:[%d]-Camera FriendlyName:%s, SN:%s\r\n", __FILE__, __LINE__, i, info[i].FriendlyName, info[i].OriginalSerialNumber);	

			/*  这里演示修改序设备列号为“DSGD13000000001”的IP */
			if(strcmp(info[i].OriginalSerialNumber ,"DSGP13000000006"))
			{
				continue;
			}

			DsGlanIpConfigText_t SetIP;
			DsGlanIpConfigText_t GetIP;
					
			memset(&GetIP, 0, sizeof(GetIP));

			/* 通过序列号指定获取哪台设备的网卡信息 */
			snprintf(GetIP.sn, sizeof(GetIP.sn), "%s", info[i].OriginalSerialNumber);			


			/* 获取设备网卡信息 */
			retStatus = dvpConfigEx(i, 0x1100, NULL, (void *)&GetIP);// 0x1100是获取相机网络信息的命令码
			if (DVP_STATUS_OK == retStatus)
			{
				printf("%s->%d:[%s] DeviceMac:%s, DeviceIP:%s, DeviceNetMask:%s, DeviceGateway:%s\n", 
					__FILE__, __LINE__, info[i].FriendlyName, GetIP.DeviceMac, GetIP.DeviceIP, GetIP.DeviceNetMask, GetIP.DeviceGateway);

				memcpy(&SetIP, &GetIP, sizeof(GetIP));
				SetIP = GetIP;

				/* 指定配置IP模式，包含lla, 静态IP，动态IP（dhcp）*/
				//strcpy(SetIP.Mode, "IP_LLA");	/* IP LLA Mode */
				snprintf(SetIP.Mode, sizeof(SetIP.Mode), "PERSISTENT");	/* 静态IP */
				//strcpy(SetIP.Mode, "");	/* 动态IP */

				/* 指定设备新的IP */
				snprintf(SetIP.DeviceIP, sizeof(SetIP.DeviceIP), "192.168.10.12");

				/* 获取设备网卡信息 */
				retStatus = dvpConfigEx(i, 0x1000, NULL, &SetIP); // 0x1000是设置相机网络信息的命令码
				if (DVP_STATUS_OK != retStatus)
				{
					printf("%s->%d:[%s] set ip config fail, retStatus:%d\r\n", __FILE__, __LINE__, info[i].FriendlyName, retStatus);
				}
				else
				{
					printf("%s->%d:[%s] set ip config success!\r\n", __FILE__, __LINE__, info[i].FriendlyName);
				}
			}
			else
			{
				printf("%s->%d:[%s] get ip config fail, retStatus:%d\r\n", __FILE__, __LINE__, info[i].FriendlyName, retStatus);
			}
		}
	}		


	return 0;
}

