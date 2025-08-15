#ifndef CAMERAPREVIEWDIALOG_H
#define CAMERAPREVIEWDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QPixmap>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

// 包含DVP摄像头API头文件
extern "C" {
#include "DVPCamera.h"
}

// 前向声明
class QGraphicsPixmapItem;

// 图像采集线程类，参考官方例程
class QImageAcquisition : public QObject
{
    Q_OBJECT
public:
    QImageAcquisition(quint32 &handle, QObject *parent = nullptr);
    ~QImageAcquisition();
    
    void StopThread();
    bool IsValidHandle(quint32 handle);
    
    quint32      m_handle;
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

class CameraPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraPreviewDialog(QWidget *parent = nullptr);
    ~CameraPreviewDialog();

protected:
    void resizeEvent(QResizeEvent* event) override;

signals:
    // 新增：发送捕获的图像信号
    void imageCaptured(const QImage& image);

private slots:
    void onScanCameraButtonClicked();
    void onOpenCameraButtonClicked();
    void onCloseCameraButtonClicked();
    void onRefreshTimer();
    void onTriggerCheckBoxClicked();
    void onTriggerButtonClicked();
    void onExposureValueChanged(double value);
    void onGainValueChanged(double value);
    void slotDispImage();
    // 新增：捕获图像槽函数
    void onCaptureButtonClicked();
    // 新增：保存图像槽函数
    void onSaveButtonClicked();
    
    // 设置保存和加载函数
    void saveCameraSettings();
    void loadCameraSettings();

private:
    void setupUI();
    void setupConnections();
    void initializeCamera();
    void scanCameras();
    bool openCamera(const QString& cameraName);
    void closeCamera();
    void startStream();
    void stopStream();
    void initCameraParameters();
    void updateDisplay();
    void updateStatus(const QString& status);
    void enableControls(bool enable);
    bool IsValidHandle(quint32 handle);

private:
    // UI组件
    QComboBox* m_cameraComboBox;
    QPushButton* m_scanButton;
    QPushButton* m_openButton;
    QPushButton* m_closeButton;
    QPushButton* m_captureButton;  // 新增：捕获按钮
    QPushButton* m_saveButton;     // 新增：保存按钮
    QCheckBox* m_triggerCheckBox;
    QPushButton* m_triggerButton;
    QDoubleSpinBox* m_exposureSpinBox;
    QDoubleSpinBox* m_gainSpinBox;
    QGraphicsScene* m_imageScene;
    QGraphicsView* m_imageView;
    QLabel* m_statusLabel;
    
    // 摄像头相关
    quint32 m_cameraHandle;  // DVP摄像头句柄
    bool m_cameraConnected;
    bool m_streaming;
    bool SoftTriggerFlag;    // 软触发标志
    
    // 图像显示
    QTimer* m_refreshTimer;
    QGraphicsPixmapItem* m_imageItem;
    QPixmap m_currentFrame;
    void* m_frameBuffer;  // 帧数据缓冲区
    
    // 图像采集线程
    QImageAcquisition* m_AcquireImage;
};

#endif // CAMERAPREVIEWDIALOG_H 