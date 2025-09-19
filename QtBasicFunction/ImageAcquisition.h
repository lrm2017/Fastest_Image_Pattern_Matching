//============================================================================================
//用QT 4.8之后推荐的方式创建线程，只需要将一个继承于QObject的类通过moveToThread移到QThread的一个对象中
//需要注意的是：
//只有在槽中执行的操作才是在线程中执行的，所以需要通过连接信号槽的方式来实现
//如果object对象存在父对象，不能将其移到子线程中执行。
//============================================================================================

#ifndef IMAGEACQUISITION_H
#define IMAGEACQUISITION_H

#include <QWaitCondition>
#include <QMessageBox>
#include <QThread>
#include <QMutex>
#include <QObject>
#include <QDebug>
#include <QTimer>
#include "DVPCamera.h"

#define m_uGrabTimeout   3000                // 图像获取的超时时间 ms

class QImageAcquisition : public QObject
{
    Q_OBJECT
public:
     QImageAcquisition(dvpHandle &handle,QObject *parent = nullptr);
    ~QImageAcquisition();
    void StopThread();
    bool IsValidHandle(dvpHandle handle);    // 判断句柄是否有效
    dvpHandle    m_handle;
    QPixmap      m_ShowImage;                // 显示图像对象
    bool         m_bAcquireImg;              // 采集线程是否结束的标志：true 运行；false 退出
    bool         ThreadSoftTriggerFlag;      // 软触发标志

    dvpFrame     m_pFrame;                   // 采集到的图像的结构体指针
    void *       pBuffer;                    // 采集到的图像的内存首地址


    QMutex       m_threadMutex;              // 互斥量
    QThread      *m_pThread = nullptr;
    QTimer       *m_timer;

private slots:
    void        slotGrabFrames();                // 抓帧函数

signals:
    void         signalDisplay();               // 显示图像信号

};

#endif // IMAGEACQUISITION_H

