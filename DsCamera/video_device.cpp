#include "video_device.h"
#include "DVPCamera.h"
#include <QDebug>
#include <QPainter>
#include <QThread>

dvpInt32 CameraDriver::OnGetFrame()
{
//    printf("enter OnGetFrame\n");
    dvpStatus status;
    void *p;
    dvpFrame frame;

    status = dvpTriggerFire(h);
    if (status != DVP_STATUS_OK)
    {
        printf("dvpTriggerFire Failed [status = %d]\r\n", status);
    }

//    printf("dvpTriggerFire success [status = %d]\r\n", status);

    status = dvpGetFrame(h, &frame, &p, 3000);
    if (status != DVP_STATUS_OK)
    {
//        if (trigMode)
//            continue;
//        else
//            break;
        printf("DvpGetFrame Failed [status = %d]\r\n", status);
    }

//    printf("DvpGetFrame success [status = %d]\r\n", status);

    dvpFrameCount framecount;
    status = dvpGetFrameCount(h, &framecount);
    if(status != DVP_STATUS_OK)
    {
        printf("get framecount failed\n");
    }

//    printf("dvpGetFrameCount success [status = %d]\r\n", status);



//    printf("handle:%d  uFrameID:%lld, uFrameDrop:%u  uFrameOK:%u  uFrameError:%u \r\n",
//        handle,pFrame->uFrameID, framecount.uFrameDrop, framecount.uFrameOK, framecount.uFrameError);
//    printf("framecount: %d, framerate: %f\n", framecount.uFrameCount, framecount.fFrameRate);
    QImage img((uint8_t *)p, frame.iWidth, frame.iHeight, QImage::Format_Grayscale8);
    if(!img.isNull())
    {
//        qDebug() << "emit send_updateFrame";
//        img.save("/userdata/ori.jpg");
        emit send_updateFrame(img, framecount.fFrameRate);
    }
    else
    {
        printf("img is NULL \n");
    }

    return 0;
}

dvpInt32 CameraDriver::OnEventConnected(dvpHandle handle, dvpEvent event, void *pContext, dvpInt32 param, dvpVariant *pVariant)
{
    /* 当发生对应事件时可添加自定义策略 */
    qDebug() << "OnEventConnected";
    return 0;
}

dvpInt32 CameraDriver::OnEventDisConnected(dvpHandle handle, dvpEvent event, void *pContext, dvpInt32 param, dvpVariant *pVariant)
{
    qDebug() << "OnEventDisConnected";
    return 0;
}

dvpInt32 CameraDriver::OnEventStreamStarted(dvpHandle handle, dvpEvent event, void *pContext, dvpInt32 param, dvpVariant *pVariant)
{
    /* 当发生对应事件时可添加自定义策略 */
    qDebug() << "OnEventStreamStarted";
    return 0;
}

dvpInt32 CameraDriver::OnEventStreamStoped(dvpHandle handle, dvpEvent event, void *pContext, dvpInt32 param, dvpVariant *pVariant)
{
    /* 当发生对应事件时可添加自定义策略 */
    qDebug() << "OnEventStreamStoped";
    return 0;
}

dvpInt32 CameraDriver::OnEventFrameLost(dvpHandle handle, dvpEvent event, void *pContext, dvpInt32 param, dvpVariant *pVariant)
{
    qDebug() << "OnEventFrameLost";
    return 0;
}

dvpInt32 CameraDriver::OnEventFrameTimeout(dvpHandle handle, dvpEvent event, void *pContext, dvpInt32 param, dvpVariant *pVariant)
{
    qDebug() << "OnEventFrameTimeout";
    return 0;
}

dvpInt32 CameraDriver::OnEventLostConnect(dvpHandle handle, dvpEvent event, void *pContext, dvpInt32 param, dvpVariant *pVariant)
{
    qDebug() << "OnEventLostConnect";
    return 0;
}

dvpInt32 CameraDriver::OnEventReconnected(dvpHandle handle, dvpEvent event, void *pContext, dvpInt32 param, dvpVariant *pVariant)
{
    qDebug() << "OnEventReconnected";
    return 0;
}

dvpInt32 CameraDriver::OnEventFrameStart(dvpHandle handle, dvpEvent event, void *pContext, dvpInt32 param, dvpVariant *pVariant)
{
    qDebug() << "OnEventFrameStart";
    return 0;
}

dvpInt32 CameraDriver::OnEventFrameEnd(dvpHandle handle, dvpEvent event, void *pContext, dvpInt32 param, dvpVariant *pVariant)
{
    qDebug() << "OnEventFrameEnd";
    return 0;
}

void CameraDriver::init_event()
{
/* 连接成功 */
#if EVENT_STATE_CONNECTED
    status = dvpRegisterEventCallback(h, OnEventConnected, EVENT_CONNECTED, NULL);
#endif
/* 断开连接 */
#if EVENT_STATE_DISCONNECTED
    status = dvpRegisterEventCallback(h, OnEventDisConnected, EVENT_DISCONNECTED, NULL);
#endif
/* 开启视频流 */
#if EVENT_STATE_STREAMSTARTED
    status = dvpRegisterEventCallback(h, OnEventStreamStarted, EVENT_STREAM_STARTRD, NULL);
#endif
/* 停止视频流 */
#if EVENT_STATE_STREAMSTOPED
    status = dvpRegisterEventCallback(h, OnEventStreamStoped, EVENT_STREAM_STOPPED, NULL);
#endif
/* 帧丢失 */
#if EVENT_STATE_FRAMELOST
    status = dvpRegisterEventCallback(h, OnEventFrameLost, EVENT_FRAME_LOST, NULL);
#endif
/* 帧超时 */
#if EVENT_STATE_FRAMETIMEOUT
    status = dvpRegisterEventCallback(h, OnEventFrameTimeout, EVENT_FRAME_TIMEOUT, NULL);
#endif
/* 丢失连接 */
#if EVENT_STATE_LOSTCONNECT
    status = dvpRegisterEventCallback(h, OnEventLostConnect, EVENT_LOST_CONNECTION, NULL);
#endif
/* 重新连接 */
#if EVENT_STATE_RECONNECT
    status = dvpRegisterEventCallback(h, OnEventReconnected, EVENT_RECONNECTED, NULL);
#endif
/* 帧开始传输 */
#if EVENT_STATE_FRAMESTART
    status = dvpRegisterEventCallback(h, OnEventFrameStart, EVENT_FRAME_START, NULL);
#endif
/* 帧结束传输 */
#if EVENT_STATE_FRAMEEND
    status = dvpRegisterEventCallback(h, OnEventFrameEnd, EVENT_FRAME_END, NULL);
#endif
}

CameraDriver::CameraDriver(int loopTime, QObject *parent) : QObject(parent), trigTime(loopTime)
{
    h = NULL;

//    start = new QTimer(this);
//    connect(start, &QTimer::timeout, this, [=](){
//        status = dvpTriggerFire(h);
//        if (status != DVP_STATUS_OK)
//        {

//        }
//    });
}

CameraDriver::~CameraDriver()
{

}

void CameraDriver::scan_camera()
{
    dvpUint32 count = 0, num = -1;
    status = DVP_STATUS_FAILED;

    /* 枚举设备 */
    dvpRefresh(&count);
    qDebug() << "device num:" << count;
    if (count > 4)
        count = 4;
     QVector<QString> nCam;
    /* 打印设备信息 */
    for (int i = 0; i < (int)count; i++)
    {
        if (dvpEnum(i, &info[i]) == DVP_STATUS_OK)
        {
            printf("[%d]-Camera FriendlyName : %s\r\n", i, info[i].FriendlyName);
            if(info[i].FriendlyName[0] == 'M'){
                status = DVP_STATUS_OK;
            }
            nCam.append(QString::fromUtf8(info[i].FriendlyName));
        }
    }

    /* 没发现设备 */
//    if (count == 0)
//    {
//        return;
//    }
    emit send_scan_results(nCam);
}

void CameraDriver::open_camera(QString canName)
{
    char* name = canName.toUtf8().data();

    printf("Test start,camera is %s\r\n", name);
    /* 打开相机 */
    status = dvpOpenByName(name, OPEN_NORMAL, &h);
    if (status != DVP_STATUS_OK)
    {
        printf("dvpOpenByName failed with err:%d\r\n", status);
        return;
    }
//    init_event();
}

void CameraDriver::close_camera()
{
    /* 在打开相机之后，开启视频流之前注册事件回调函数，启动一个专门的线程以dvpGetFrame（同步采集）的方式获取图�?*/
//    status = dvpUnregisterStreamCallback(h, OnGetFrame, STREAM_EVENT_FRAME_THREAD, this);
//    if (status != DVP_STATUS_OK)
//    {
//        printf("dvpUnregisterStreamCallback failed with err:%d\r\n", status);
//    }
    dvpClose(h);
}

void CameraDriver::init_camera()
{
    /* 设置为软触发 */
    status = dvpSetTriggerState(h, true);
    if (status == DVP_STATUS_OK)
    {
        dvpSetTriggerSource(h, TRIGGER_SOURCE_SOFTWARE);
    }

    status = dvpSetExposure(h, 10000);
    if(status != DVP_STATUS_OK)
    {
        qDebug() << "set Exposure error" << status;
    }

    status = dvpSetAnalogGain(h, 1.0);


    dvpRegion region;
    double exp;
    float gain;
    /* 获取ROI信息  */
    status = dvpGetRoi(h, &region);
    if (status != DVP_STATUS_OK)
    {
        printf("dvpGetRoi failed with err:%d\r\n", status);
        return;
    }
    printf("region: x:%d, y:%d, w:%d, h:%d\r\n", region.X, region.Y, region.W, region.H);
    width = region.W;
    height = region.H;

    /* 获取曝光信息 */
    status = dvpGetExposure(h, &exp);
    if (status != DVP_STATUS_OK)
    {
        printf("dvpGetExposure failed with err:%d\r\n", status);
        return;
    }
    /* 获取增益信息 */
    status = dvpGetAnalogGain(h, &gain);
    if (status != DVP_STATUS_OK)
    {
        printf("dvpGetAnalogGain failed with err:%d\r\n", status);
        return;
    }
    printf("exposure: %lf, gain: %f\r\n", exp, gain);

    /* 在打开相机之后，开启视频流之前注册事件回调函数，启动一个专门的线程以dvpGetFrame（同步采集）的方式获取图�?*/
//    status = dvpRegisterStreamCallback(h, OnGetFrame, STREAM_EVENT_FRAME_THREAD, this);
//    if (status != DVP_STATUS_OK)
//    {
//        printf("dvpRegisterStreamCallback failed with err:%d\r\n", status);
//    }
}

void CameraDriver::start_camera()
{
    qDebug() << "start";
    qDebug() << QThread::currentThreadId() << " " << h;
    status = dvpStart(h);
    if (status != DVP_STATUS_OK)
    {
        printf("dvpStart with err:%d\r\n", status);
        return;
    }
    // start->start(trigTime);
}

void CameraDriver::stop_camera()
{
    // start->stop();
    status = dvpStop(h);
    if (status != DVP_STATUS_OK)
    {
        printf("dvpStop with err:%d\r\n", status);
        return;
    }
}

void CameraDriver::hold_camera()
{
    status = dvpHold(h);
    if (status != DVP_STATUS_OK)
    {
        printf("dvpHold with err:%d\r\n", status);
        return;
    }
}

void CameraDriver::restart_camera()
{
    status = dvpRestart(h);
    if (status != DVP_STATUS_OK)
    {
        printf("dvpRestart with err:%d\r\n", status);
        return;
    }
}

void CameraDriver::on_scanCamera()
{
    scan_camera();
}

void CameraDriver::on_openCamera(QString canName)
{
    open_camera(canName);
    if(status == DVP_STATUS_OK){
        init_camera();
//        emit send_status(1);
    }
}

void CameraDriver::on_closeCamera()
{
    stop_camera();
    dvpClose(h);

}

void CameraDriver::on_startCamera()
{
    bool state = false;
    dvpIsValid(h, &state);
    qDebug() << "isvalid:" << state << "status:" << status;
    if(!state || status != DVP_STATUS_OK)
    {
        qDebug() << "start error";
        return;
    }

    start_camera();
}

void CameraDriver::on_stopCamera()
{
    stop_camera();
}

void CameraDriver::on_holdCamera()
{
    hold_camera();
}

void CameraDriver::on_restartCamera()
{
    restart_camera();
}

void CameraDriver::on_setExposure(double time)
{
    status = dvpSetExposure(h, time);
    if(status != DVP_STATUS_OK)
    {
        qDebug() << "set Exposure error" << status;
    }
    qDebug() << "set Exposure" << time;
    /* 获取曝光信息 */
    double exp;
    status = dvpGetExposure(h, &exp);
    if (status != DVP_STATUS_OK)
    {
        printf("dvpGetExposure failed with err:%d\r\n", status);
        return;
    }
    qDebug() << "exposure : " << exp;
}


void CameraDriver::test_timer()
{
    printf("vd2_timer is running \n");
}
