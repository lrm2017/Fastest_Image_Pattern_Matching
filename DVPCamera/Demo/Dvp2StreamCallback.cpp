/*****************************************************************************
* @FileName:dvp2getframe.cpp
* @CreatTime: 2020/9/1 10:58
* @Descriptions: 回调方式获取�?
* @Version: ver 1.0
* @Copyright(c) 2020 Do3Think All Rights Reserved.
*****************************************************************************/

#include <iostream>
#include <thread>
using namespace std;

#include <stdio.h> 
#include <stdint.h> 
#include <pthread.h> 
#include <iostream> 
#include "DVPCamera.h"


/* 自定义回调函�?*/
static dvpInt32 OnGetFrame(dvpHandle handle, dvpStreamEvent event, void* pContext, dvpFrame* pFrame, void* pBuffer)
{
	char PicName[64];
	dvpStatus status;

	/* 显示帧数和帧�?*/
	dvpFrameCount framecount;
	status = dvpGetFrameCount(handle, &framecount);
	if (status != DVP_STATUS_OK)
	{
		printf("get framecount failed\n");
	}

	/* 显示帧信�?*/
	printf("handle:%d  uFrameID:%lld, uFrameDrop:%u  uFrameOK:%u  uFrameError:%u \r\n",
		handle,pFrame->uFrameID, framecount.uFrameDrop, framecount.uFrameOK, framecount.uFrameError);

	/* 需要创建pic目录保存图片 */
	/*sprintf(PicName, "pic/test_pic_framecount_%d.jpg", framecount.uFrameCount);
	status = dvpSavePicture(pFrame, pBuffer, PicName, 90);
	if (status != DVP_STATUS_OK) {
		printf("Save failed,error:%d\n", status);
	}*/


	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	/* 通常返回0即可，没有特殊含�?*/
	return 0;
}


static dvpInt32 OnDVPEvent(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, struct dvpVariant* pVariant)
{
	/* 当发生对应事件时可添加自定义策略 */
	
	return 0;
}

void test(void* p)
{
	dvpStatus status;
	dvpHandle h;
	char* name = (char*)p;

	printf("Test start,camera is %s\r\n", name);
	do
	{
		/* 打开设备 */
		status = dvpOpenByName(name, OPEN_NORMAL, &h);
		if (status != DVP_STATUS_OK)
		{
			printf("dvpOpenByName failed with err:%d\r\n", status);
			break;
		}

                status = dvpSetTriggerState(h, true);
                if (status == DVP_STATUS_OK)
                {
                    dvpSetTriggerSource(h, TRIGGER_SOURCE_LINE1);
                }
		

		dvpRegion region;
		double exp;
		float gain;

                /*status = dvpSetTriggerState(h, false);
		if (status != DVP_STATUS_OK)
		{
			printf("dvpSetTriggerState failed with err:%d\r\n", status);
			break;
                }*/

		/* 打印ROI信息 */
		status = dvpGetRoi(h, &region);
		if (status != DVP_STATUS_OK)
		{
			printf("dvpGetRoi failed with err:%d\r\n", status);
			break;
		}
		printf("%s, region: x:%d, y:%d, w:%d, h:%d\r\n", name, region.X, region.Y, region.W, region.H);

		/* 打印曝光增益信息 */
		status = dvpGetExposure(h, &exp);
		if (status != DVP_STATUS_OK)
		{
			printf("dvpGetExposure failed with err:%d\r\n", status);
			break;
		}

		status = dvpGetAnalogGain(h, &gain);
		if (status != DVP_STATUS_OK)
		{
			printf("dvpGetAnalogGain failed with err:%d\r\n", status);
			break;
		}

		printf("%s, exposure: %lf, gain: %f\r\n", name, exp, gain);

		/* 在打开相机之后，开启视频流之前注册事件回调函数，启动一个专门的线程以dvpGetFrame（同步采集）的方式获取图�?*/
		status = dvpRegisterStreamCallback(h, OnGetFrame, STREAM_EVENT_FRAME_THREAD, NULL);
		if (status != DVP_STATUS_OK)
		{
			printf("dvpRegisterStreamCallback failed with err:%d\r\n", status);
			break;
		}

		/* 开始视频流 */
		status = dvpStart(h);
		if (status != DVP_STATUS_OK)
		{
			break;
		}

		/* 主线程等�?*/
		int key;
		//printf("\nPress ESC to exit!\n");
		while (1)
		{
			//getch会阻塞主线程
			key = getchar();
			if (key == 27)
			{
				/* 关闭视频�?*/
				status = dvpStop(h);
				if (status != DVP_STATUS_OK)
				{
					break;
				}
				break;
			}
		}
	} while (0);

	/* 关闭相机 */
	status = dvpClose(h);

	printf("\ntest quit, %s, status:%d\n", name, status);
}

int main()
{
	printf("start...\r\n");

	dvpUint32 count = 0, num = -1;
	dvpCameraInfo info[8];

	/* 枚举设备 */
	dvpRefresh(&count);
	if (count > 8)
		count = 8;

	for (int i = 0; i < (int)count; i++)
	{
		if (dvpEnum(i, &info[i]) == DVP_STATUS_OK)
		{
			printf("[%d]-Camera FriendlyName : %s\r\n", i, info[i].FriendlyName);
		}
	}

	/* 没发现设�?*/
	if (count == 0)
	{
		printf("No device found!\n");
		return 0;
	}

        /*while (num < 0 || num >= count)
	{
		printf("Please enter the number of the camera you want to open: \r\n");
		scanf("%d", &num);
        }*/

        thread task1(test, (void*)info[0].FriendlyName);
        thread task2(test, (void*)info[1].FriendlyName);
        task1.join();
        task2.join();

	system("pause");
	return 0;
}
