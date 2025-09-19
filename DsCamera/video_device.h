#ifndef CAMERADRIVER_H
#define CAMERADRIVER_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <QVector>
#include "DVPCamera.h"

#define EVENT_STATE_CONNECTED       1  // 启用连接成功回调
#define EVENT_STATE_DISCONNECTED    1  // 启用断开连接回调
#define EVENT_STATE_STREAMSTARTED   1  // 启用视频流开始回调
#define EVENT_STATE_STREAMSTOPED    1  // 启用视频流停止回调
#define EVENT_STATE_FRAMELOST       1  // 启用帧丢失回调
#define EVENT_STATE_FRAMETIMEOUT    1  // 启用帧超时回调
#define EVENT_STATE_LOSTCONNECT     1  // 启用丢失连接回调
#define EVENT_STATE_RECONNECT       1  // 启用重新连接回调
#define EVENT_STATE_FRAMESTART      1  // 启用帧开始传输回调
#define EVENT_STATE_FRAMEEND        1  // 启用帧结束传输回调

class CameraDriver : public QObject
{
    Q_OBJECT
private:
    // 帧数据回调函数
//    dvpInt32 OnGetFrame();
    // 事件回调函数群
    static dvpInt32 OnEventConnected(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, dvpVariant* pVariant);
    static dvpInt32 OnEventDisConnected(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, dvpVariant* pVariant);
    static dvpInt32 OnEventStreamStarted(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, dvpVariant* pVariant);
    static dvpInt32 OnEventStreamStoped(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, dvpVariant* pVariant);
    static dvpInt32 OnEventFrameLost(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, dvpVariant* pVariant);
    static dvpInt32 OnEventFrameTimeout(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, dvpVariant* pVariant);
    static dvpInt32 OnEventLostConnect(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, dvpVariant* pVariant);
    static dvpInt32 OnEventReconnected(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, dvpVariant* pVariant);
    static dvpInt32 OnEventFrameStart(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, dvpVariant* pVariant);
    static dvpInt32 OnEventFrameEnd(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, dvpVariant* pVariant);
    void init_event();

    void scan_camera();
    void open_camera(QString camName);
    void close_camera();
    void init_camera();
    void start_camera();
    void stop_camera();
    void hold_camera();
    void restart_camera();

public:
    explicit CameraDriver(int loopTime = 100, QObject *parent = nullptr);
    ~CameraDriver();
signals:
    void send_updateFrame(QImage frame, float frameRate);
    void send_scan_results(QVector<QString> cameraList);
    void send_triggerFire();

public slots:

    void test_timer();


    dvpInt32 OnGetFrame();

    void on_scanCamera();
    void on_openCamera(QString canName);
    void on_closeCamera();
    void on_startCamera();
    void on_stopCamera();
    void on_holdCamera();
    void on_restartCamera();
    void on_setExposure(double time);

private:
    dvpHandle h;
    dvpStatus status;
    dvpCameraInfo info[4];

    QTimer *start;
    int trigTime;
    int width;
    int height;
};

#endif // CAMERADRIVER_H
