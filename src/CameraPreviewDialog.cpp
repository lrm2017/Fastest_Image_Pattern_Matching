#include "CameraPreviewDialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>
#include <QPainter>
#include <QTime>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QFileDialog>
#include <QFileInfo>
#include <QDateTime>
#include <QSettings>
#include <opencv2/opencv.hpp>



// 图像采集线程类已在头文件中声明

// RGB转BGR和BGR转RGB是一样的方法
static bool BGR2RGB(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
{
    if (NULL == pRgbData)
    {
        return false;
    }

    for (unsigned int j = 0; j < nHeight; j++)
    {
        for (unsigned int i = 0; i < nWidth; i++)
        {
            unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
            pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
            pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
        }
    }
    return true;
}

QImageAcquisition::QImageAcquisition(quint32 &handle, QObject *parent):
    QObject(parent)
{
    m_pThread = new QThread();
    this->moveToThread(m_pThread);
    // 启动采集线程
    m_pThread->start();

    // 初始化成员变量
    m_handle = handle;
    pBuffer = NULL;

    m_timer = new QTimer(this);

    //将定时器超时信号与槽(功能函数)联系起来
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotGrabFrames()));

    //定时器每30毫秒采集一次，也就是显示帧率大概维持在30帧每秒
    m_timer->start(30);
    qDebug() << __FUNCTION__ << " id = " << QThread::currentThreadId();
}

QImageAcquisition::~QImageAcquisition()
{
    //等待采集线程结束
    m_pThread->wait();

    //删除线程对象
    if (m_pThread != NULL)
    {
        delete m_pThread;
        m_pThread = NULL;
    }
}

bool QImageAcquisition::IsValidHandle(quint32 handle)
{
    bool bValidHandle = false;
    dvpIsValid(handle, &bValidHandle);
    return bValidHandle;
}

void QImageAcquisition::slotGrabFrames()
{
    dvpStatus status;
    status = dvpGetFrame(m_handle, &m_pFrame, &pBuffer, 3000); // 3秒超时

    if (status == DVP_STATUS_OK)
    {
        //这里将采集图像、图像转换放置在工作线程中实现，解决主界面在高帧率显示时卡顿问题
        QPixmap tempImage;
        
        if(m_pFrame.format == FORMAT_BGR24)
        {
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
            // Qt5.14版本新增QImage::Format_BGR888类型
            tempImage = QPixmap::fromImage(QImage((uchar*)pBuffer, m_pFrame.iWidth, m_pFrame.iHeight, 
                                                   m_pFrame.iWidth*3, QImage::Format_BGR888, 0, 0));
#else
            //其他版本先把BGR数据转成RGB数据，再用RGB数据转QImage
            BGR2RGB((uchar*)pBuffer, m_pFrame.iWidth, m_pFrame.iHeight);
            tempImage = QPixmap::fromImage(QImage((uchar*)pBuffer, m_pFrame.iWidth, m_pFrame.iHeight, 
                                                   m_pFrame.iWidth*3, QImage::Format_RGB888, 0, 0));
#endif
        }
        else if(m_pFrame.format == FORMAT_RGB24)
        {
            tempImage = QPixmap::fromImage(QImage((uchar*)pBuffer, m_pFrame.iWidth, m_pFrame.iHeight, 
                                                   m_pFrame.iWidth*3, QImage::Format_RGB888, 0, 0));
        }
        else if(m_pFrame.format == FORMAT_MONO)
        {
            tempImage = QPixmap::fromImage(QImage((uchar*)pBuffer, m_pFrame.iWidth, m_pFrame.iHeight, 
                                                   m_pFrame.iWidth, QImage::Format_Grayscale8, 0, 0));
        }
        
        // 只在有图像数据时加锁更新
        if (!tempImage.isNull()) {
            if (m_threadMutex.tryLock(100)) { // 100ms 超时
                m_ShowImage = tempImage;
                m_threadMutex.unlock();
                emit signalDisplay();
            }
        }
    }
    else
    {
        m_pThread->usleep(50);
    }
}

void QImageAcquisition::StopThread()
{
    if(m_pThread->isRunning())
    {
        m_pThread->exit();
    }
}

// 全局变量用于存储摄像头信息
static QList<dvpCameraInfo> g_cameraInfoList;
static QMutex g_cameraMutex;

CameraPreviewDialog::CameraPreviewDialog(QWidget *parent)
    : QDialog(parent)
    , m_cameraHandle(0)
    , m_cameraConnected(false)
    , m_streaming(false)
    , m_refreshTimer(nullptr)
    , m_imageItem(nullptr)
    , m_AcquireImage(nullptr)
    , SoftTriggerFlag(false)
{
    setWindowTitle("摄像头预览");
    setMinimumSize(800, 600);
    
    setupUI();
    setupConnections();
    loadCameraSettings();  // 加载摄像头设置
    initializeCamera();
}

CameraPreviewDialog::~CameraPreviewDialog()
{
    saveCameraSettings();  // 保存摄像头设置
    
    if (m_cameraConnected) {
        closeCamera();
    }
    if (m_refreshTimer) {
        m_refreshTimer->stop();
        delete m_refreshTimer;
    }
}

void CameraPreviewDialog::setupUI()
{
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 控制面板
    QHBoxLayout* controlLayout = new QHBoxLayout();
    
    // 摄像头选择
    QLabel* cameraLabel = new QLabel("选择摄像头:");
    m_cameraComboBox = new QComboBox();
    m_cameraComboBox->setMinimumWidth(200);
    
    // 按钮
    m_scanButton = new QPushButton("扫描摄像头");
    m_openButton = new QPushButton("开启摄像头");
    m_closeButton = new QPushButton("关闭摄像头");
    m_closeButton->setEnabled(false);
    
    // 新增：捕获按钮
    m_captureButton = new QPushButton("捕获图像");
    m_captureButton->setEnabled(false);
    
    // 新增：保存按钮
    m_saveButton = new QPushButton("保存图像");
    m_saveButton->setEnabled(false);
    
    // 触发控制
    m_triggerCheckBox = new QCheckBox("软触发模式");
    m_triggerButton = new QPushButton("触发");
    m_triggerButton->setEnabled(false);
    
    // 曝光和增益控制
    QLabel* expoLabel = new QLabel("曝光时间(μs):");
    m_exposureSpinBox = new QDoubleSpinBox();
    m_exposureSpinBox->setRange(1.0, 1000000.0);
    m_exposureSpinBox->setValue(10000.0);
    m_exposureSpinBox->setEnabled(false);
    
    QLabel* gainLabel = new QLabel("增益:");
    m_gainSpinBox = new QDoubleSpinBox();
    m_gainSpinBox->setRange(1.0, 16.0);
    m_gainSpinBox->setValue(1.0);
    m_gainSpinBox->setEnabled(false);
    
    controlLayout->addWidget(cameraLabel);
    controlLayout->addWidget(m_cameraComboBox);
    controlLayout->addWidget(m_scanButton);
    controlLayout->addWidget(m_openButton);
    controlLayout->addWidget(m_closeButton);
    controlLayout->addWidget(m_captureButton);
    controlLayout->addWidget(m_saveButton);
    controlLayout->addWidget(m_triggerCheckBox);
    controlLayout->addWidget(m_triggerButton);
    controlLayout->addWidget(expoLabel);
    controlLayout->addWidget(m_exposureSpinBox);
    controlLayout->addWidget(gainLabel);
    controlLayout->addWidget(m_gainSpinBox);
    controlLayout->addStretch();
    
    mainLayout->addLayout(controlLayout);
    
    // 图像显示区域
    m_imageScene = new QGraphicsScene(this);
    m_imageView = new QGraphicsView(m_imageScene);
    m_imageView->setRenderHint(QPainter::Antialiasing);
    m_imageView->setRenderHint(QPainter::SmoothPixmapTransform);
    m_imageView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_imageView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    mainLayout->addWidget(m_imageView);
    
    // 状态栏
    m_statusLabel = new QLabel("状态: 未连接");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);
    
    // 设置样式
    setStyleSheet("QDialog { background-color: #f0f0f0; }");
}

void CameraPreviewDialog::setupConnections()
{
    // 连接按钮信号
    connect(m_scanButton, &QPushButton::clicked, this, &CameraPreviewDialog::onScanCameraButtonClicked);
    connect(m_openButton, &QPushButton::clicked, this, &CameraPreviewDialog::onOpenCameraButtonClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &CameraPreviewDialog::onCloseCameraButtonClicked);
    connect(m_captureButton, &QPushButton::clicked, this, &CameraPreviewDialog::onCaptureButtonClicked);
    connect(m_saveButton, &QPushButton::clicked, this, &CameraPreviewDialog::onSaveButtonClicked);
    connect(m_triggerCheckBox, &QCheckBox::clicked, this, &CameraPreviewDialog::onTriggerCheckBoxClicked);
    connect(m_triggerButton, &QPushButton::clicked, this, &CameraPreviewDialog::onTriggerButtonClicked);
    connect(m_exposureSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &CameraPreviewDialog::onExposureValueChanged);
    connect(m_gainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &CameraPreviewDialog::onGainValueChanged);
}

void CameraPreviewDialog::initializeCamera()
{
    // 创建刷新定时器，用于更新状态信息
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(1000); // 1秒更新一次状态
    connect(m_refreshTimer, &QTimer::timeout, this, &CameraPreviewDialog::onRefreshTimer);
    
    // 扫描可用摄像头
    scanCameras();
}

bool CameraPreviewDialog::IsValidHandle(dvpHandle handle)
{
    bool bValidHandle = false;
    dvpIsValid(handle, &bValidHandle);
    return bValidHandle;
}

void CameraPreviewDialog::scanCameras()
{
    updateStatus("正在扫描摄像头...");
    
    m_cameraComboBox->clear();
    g_cameraInfoList.clear();
    
    // 使用 tryLock 避免长时间阻塞
    if (!g_cameraMutex.tryLock(1000)) { // 1秒超时
        updateStatus("摄像头忙，请稍后重试");
        qDebug() << "Failed to acquire camera mutex for scan";
        return;
    }
    
    qDebug() << "Scanning for cameras...";
    
    // 使用DVP API扫描摄像头
    dvpUint32 count = 0;
    dvpStatus status = dvpRefresh(&count);
    
    if (status != DVP_STATUS_OK) {
        g_cameraMutex.unlock();
        updateStatus("扫描摄像头失败");
        qDebug() << "dvpRefresh failed with status:" << status;
        return;
    }
    
    qDebug() << "Found" << count << "cameras";
    
    if (count == 0) {
        g_cameraMutex.unlock();
        updateStatus("未发现摄像头");
        return;
    }
    
    // 限制最大摄像头数量
    if (count > 16) {
        count = 16;
    }
    
    // 枚举每个摄像头
    for (dvpUint32 i = 0; i < count; i++) {
        dvpCameraInfo info;
        status = dvpEnum(i, &info);
        if (status == DVP_STATUS_OK) {
            g_cameraInfoList.append(info);
            QString cameraName = QString(info.FriendlyName);
            m_cameraComboBox->addItem(cameraName);
            qDebug() << "Found camera" << i << ":" << info.FriendlyName;
            qDebug() << "  Model:" << info.Model;
            qDebug() << "  Vendor:" << info.Vendor;
            qDebug() << "  SerialNumber:" << info.SerialNumber;
        } else {
            qDebug() << "Failed to enum camera" << i << "with status:" << status;
        }
    }
    
    g_cameraMutex.unlock(); // 立即释放锁
    updateStatus(QString("发现 %1 个摄像头").arg(g_cameraInfoList.size()));
    
    // 尝试选择上次选择的摄像头
    QSettings settings("FastestImagePatternMatching", "CameraPreview");
    QString lastCamera = settings.value("LastSelectedCamera", "").toString();
    if (!lastCamera.isEmpty()) {
        int index = m_cameraComboBox->findText(lastCamera);
        if (index >= 0) {
            m_cameraComboBox->setCurrentIndex(index);
            qDebug() << "已选择上次使用的摄像头:" << lastCamera;
        }
    }
}

bool CameraPreviewDialog::openCamera(const QString& cameraName)
{
    if (m_cameraConnected) {
        return false;
    }
    
    int selectedIndex = m_cameraComboBox->currentIndex();
    if (selectedIndex < 0 || selectedIndex >= g_cameraInfoList.size()) {
        updateStatus("无效的摄像头选择");
        return false;
    }
    
    updateStatus("正在连接摄像头...");
    
    // 使用 tryLock 避免长时间阻塞
    if (!g_cameraMutex.tryLock(1000)) { // 1秒超时
        updateStatus("摄像头忙，请稍后重试");
        qDebug() << "Failed to acquire camera mutex for open";
        return false;
    }
    
    // 使用DVP API打开摄像头
    dvpStatus status = dvpOpenByName(g_cameraInfoList[selectedIndex].FriendlyName, 
                                    OPEN_NORMAL, &m_cameraHandle);
    
    if (status != DVP_STATUS_OK) {
        g_cameraMutex.unlock();
        updateStatus(QString("连接摄像头失败，错误码: %1").arg(status));
        qDebug() << "dvpOpenByName failed with status:" << status;
        return false;
    }
    
    // 检查摄像头是否有效
    if (!IsValidHandle(m_cameraHandle)) {
        g_cameraMutex.unlock();
        updateStatus("摄像头无效");
        dvpClose(m_cameraHandle);
        m_cameraHandle = 0;
        return false;
    }
    
    g_cameraMutex.unlock(); // 立即释放锁
    
    // 初始化摄像头参数
    initCameraParameters();
    
    m_cameraConnected = true;
    updateStatus("摄像头连接成功");
    
    // 开始视频流
    startStream();
    
    return true;
}

void CameraPreviewDialog::initCameraParameters()
{
    if (!IsValidHandle(m_cameraHandle)) {
        return;
    }
    
    dvpStatus status;
    
    // 设置目标格式为BGR24
    status = dvpSetTargetFormat(m_cameraHandle, S_BGR24);
    if (status != DVP_STATUS_OK) {
        qDebug() << "Set target format failed with status:" << status;
    }
    
    // 设置曝光时间
    status = dvpSetExposure(m_cameraHandle, m_exposureSpinBox->value());
    if (status != DVP_STATUS_OK) {
        qDebug() << "Set exposure failed with status:" << status;
    }
    
    // 设置模拟增益
    status = dvpSetAnalogGain(m_cameraHandle, m_gainSpinBox->value());
    if (status != DVP_STATUS_OK) {
        qDebug() << "Set analog gain failed with status:" << status;
    }
    
    // 设置触发模式
    status = dvpSetTriggerState(m_cameraHandle, SoftTriggerFlag);
    if (status != DVP_STATUS_OK) {
        qDebug() << "Set trigger state failed with status:" << status;
    }
    
    if (SoftTriggerFlag) {
        status = dvpSetTriggerSource(m_cameraHandle, TRIGGER_SOURCE_SOFTWARE);
        if (status != DVP_STATUS_OK) {
            qDebug() << "Set trigger source failed with status:" << status;
        }
    }
    
    // 启用控件
    m_exposureSpinBox->setEnabled(true);
    m_gainSpinBox->setEnabled(true);
    m_triggerCheckBox->setEnabled(true);
    m_triggerButton->setEnabled(SoftTriggerFlag);
}

void CameraPreviewDialog::closeCamera()
{
    if (!m_cameraConnected) {
        return;
    }
    
    updateStatus("正在关闭摄像头...");
    
    stopStream();
    
    QMutexLocker locker(&g_cameraMutex);
    
    if (m_cameraHandle != 0) {
        dvpClose(m_cameraHandle);
        m_cameraHandle = 0;
    }
    
    m_cameraConnected = false;
    
    // 禁用控件
    m_exposureSpinBox->setEnabled(false);
    m_gainSpinBox->setEnabled(false);
    m_triggerCheckBox->setEnabled(false);
    m_triggerButton->setEnabled(false);
    
    updateStatus("摄像头已关闭");
}

void CameraPreviewDialog::startStream()
{
    if (m_streaming) {
        return;
    }
    
    qDebug() << "Starting video stream...";
    
    // 使用 tryLock 避免长时间阻塞
    if (!g_cameraMutex.tryLock(1000)) { // 1秒超时
        updateStatus("摄像头忙，请稍后重试");
        qDebug() << "Failed to acquire camera mutex";
        return;
    }
    
    // 启动视频流
    dvpStatus status = dvpStart(m_cameraHandle);
    if (status != DVP_STATUS_OK) {
        g_cameraMutex.unlock();
        updateStatus(QString("启动视频流失败，错误码: %1").arg(status));
        qDebug() << "dvpStart failed with status:" << status;
        return;
    }
    
    g_cameraMutex.unlock(); // 立即释放锁
    
    qDebug() << "Video stream started successfully";
    
    // 创建图像采集线程对象
    m_AcquireImage = new QImageAcquisition(m_cameraHandle);
    // 建立图像显示信号和槽函数的联系
    connect(m_AcquireImage, SIGNAL(signalDisplay()), this, SLOT(slotDispImage()));
    
    m_streaming = true;
    m_refreshTimer->start();
    updateStatus("视频流已开始");
}

void CameraPreviewDialog::stopStream()
{
    if (!m_streaming) {
        return;
    }
    
    // 使用 tryLock 避免长时间阻塞
    if (!g_cameraMutex.tryLock(1000)) { // 1秒超时
        qDebug() << "Failed to acquire camera mutex for stop";
        return;
    }
    
    if (m_cameraHandle != 0) {
        dvpStop(m_cameraHandle);
    }
    
    g_cameraMutex.unlock(); // 立即释放锁
    
    // 停止图像采集线程
    if (m_AcquireImage) {
        m_AcquireImage->StopThread();
        delete m_AcquireImage;
        m_AcquireImage = nullptr;
    }
    
    m_refreshTimer->stop();
    m_streaming = false;
    updateStatus("视频流已停止");
}

void CameraPreviewDialog::slotDispImage()
{
    if (m_AcquireImage != nullptr) {
        if(m_AcquireImage->m_threadMutex.tryLock()) {
            if (!m_AcquireImage->m_ShowImage.isNull()) {
                // 清除场景中的旧图像
                m_imageScene->clear();
                
                // 添加新图像到场景
                m_imageItem = m_imageScene->addPixmap(m_AcquireImage->m_ShowImage);
                m_imageScene->setSceneRect(m_AcquireImage->m_ShowImage.rect());
                
                // 自动调整视图大小
                m_imageView->fitInView(m_imageItem, Qt::KeepAspectRatio);
            }
            m_AcquireImage->m_threadMutex.unlock();
        }
    }
}

void CameraPreviewDialog::updateDisplay()
{
    // 这个方法现在主要用于兼容性，实际显示由slotDispImage处理
}

void CameraPreviewDialog::updateStatus(const QString& status)
{
    m_statusLabel->setText("状态: " + status);
    QApplication::processEvents();
}

void CameraPreviewDialog::enableControls(bool enable)
{
    m_openButton->setEnabled(!enable);
    m_closeButton->setEnabled(enable);
    m_captureButton->setEnabled(enable);  // 摄像头连接后启用捕获按钮
    m_saveButton->setEnabled(enable);     // 摄像头连接后启用保存按钮
    m_cameraComboBox->setEnabled(!enable);
}

void CameraPreviewDialog::onScanCameraButtonClicked()
{
    scanCameras();
}

void CameraPreviewDialog::onOpenCameraButtonClicked()
{
    if (m_cameraComboBox->currentText().isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择摄像头");
        return;
    }
    
    if (openCamera(m_cameraComboBox->currentText())) {
        enableControls(true);
        // 保存当前选择的摄像头
        saveCameraSettings();
    }
}

void CameraPreviewDialog::onCloseCameraButtonClicked()
{
    closeCamera();
    enableControls(false);
}

void CameraPreviewDialog::onTriggerCheckBoxClicked()
{
    if (!IsValidHandle(m_cameraHandle)) {
        return;
    }
    
    SoftTriggerFlag = m_triggerCheckBox->isChecked();
    m_triggerButton->setEnabled(SoftTriggerFlag);
    
    dvpStatus status = dvpSetTriggerState(m_cameraHandle, SoftTriggerFlag);
    if (status != DVP_STATUS_OK) {
        qDebug() << "Set trigger state failed with status:" << status;
        return;
    }
    
    if (SoftTriggerFlag) {
        status = dvpSetTriggerSource(m_cameraHandle, TRIGGER_SOURCE_SOFTWARE);
        if (status != DVP_STATUS_OK) {
            qDebug() << "Set trigger source failed with status:" << status;
        }
    }
    
    // 自动保存设置
    saveCameraSettings();
}

void CameraPreviewDialog::onTriggerButtonClicked()
{
    if (!IsValidHandle(m_cameraHandle) || !SoftTriggerFlag) {
        return;
    }
    
    dvpStatus status = dvpTriggerFire(m_cameraHandle);
    if (status != DVP_STATUS_OK) {
        qDebug() << "Trigger fire failed with status:" << status;
    }
}

void CameraPreviewDialog::onExposureValueChanged(double value)
{
    if (!IsValidHandle(m_cameraHandle)) {
        return;
    }
    
    dvpStatus status = dvpSetExposure(m_cameraHandle, value);
    if (status != DVP_STATUS_OK) {
        qDebug() << "Set exposure failed with status:" << status;
    }
    
    // 自动保存设置
    saveCameraSettings();
}

void CameraPreviewDialog::onGainValueChanged(double value)
{
    if (!IsValidHandle(m_cameraHandle)) {
        return;
    }
    
    dvpStatus status = dvpSetAnalogGain(m_cameraHandle, value);
    if (status != DVP_STATUS_OK) {
        qDebug() << "Set analog gain failed with status:" << status;
    }
    
    // 自动保存设置
    saveCameraSettings();
}

void CameraPreviewDialog::onRefreshTimer()
{
    if (IsValidHandle(m_cameraHandle) && m_streaming) {
        // 更新状态信息
        dvpFrameCount frameCount;
        dvpStatus status = dvpGetFrameCount(m_cameraHandle, &frameCount);
        if (status == DVP_STATUS_OK) {
            QString frameInfo = QString("视频流中 - 帧数: %1, 帧率: %2 fps")
                               .arg(frameCount.uFrameCount)
                               .arg(frameCount.fFrameRate, 0, 'f', 1);
            updateStatus(frameInfo);
        }
    }
}

void CameraPreviewDialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    
    // 当窗口大小改变时，重新调整图像视图
    if (m_imageItem) {
        m_imageView->fitInView(m_imageItem, Qt::KeepAspectRatio);
    }
} 

void CameraPreviewDialog::onCaptureButtonClicked()
{
    if (m_AcquireImage && !m_AcquireImage->m_ShowImage.isNull()) {
        // 将 QPixmap 转换为 QImage
        QImage capturedImage = m_AcquireImage->m_ShowImage.toImage();
        
        // 发送捕获的图像信号
        emit imageCaptured(capturedImage);
        
        updateStatus("图像已捕获并发送");
        qDebug() << "Image captured and sent, size:" << capturedImage.size();
    } else {
        updateStatus("没有可用的图像");
        qDebug() << "No image available for capture";
    }
}

void CameraPreviewDialog::onSaveButtonClicked()
{
    if (m_AcquireImage && !m_AcquireImage->m_ShowImage.isNull()) {
        // 将 QPixmap 转换为 QImage
        QImage imageToSave = m_AcquireImage->m_ShowImage.toImage();
        
        // 弹出文件保存对话框
        QString fileName = QFileDialog::getSaveFileName(
            this,
            "保存图像",
            QString("camera_image_%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
            "PNG图像 (*.png);;JPEG图像 (*.jpg *.jpeg);;BMP图像 (*.bmp);;所有文件 (*.*)"
        );
        
        if (!fileName.isEmpty()) {
            // 根据文件扩展名选择保存格式
            QFileInfo fileInfo(fileName);
            QString suffix = fileInfo.suffix().toLower();
            
            bool saveSuccess = false;
            if (suffix == "png") {
                saveSuccess = imageToSave.save(fileName, "PNG");
            } else if (suffix == "jpg" || suffix == "jpeg") {
                saveSuccess = imageToSave.save(fileName, "JPEG", 95); // 95%质量
            } else if (suffix == "bmp") {
                saveSuccess = imageToSave.save(fileName, "BMP");
            } else {
                // 默认保存为PNG
                if (!fileName.endsWith(".png", Qt::CaseInsensitive)) {
                    fileName += ".png";
                }
                saveSuccess = imageToSave.save(fileName, "PNG");
            }
            
            if (saveSuccess) {
                updateStatus(QString("图像已保存到: %1").arg(fileName));
                qDebug() << "Image saved successfully to:" << fileName;
            } else {
                updateStatus("保存图像失败");
                qDebug() << "Failed to save image to:" << fileName;
            }
        }
    } else {
        updateStatus("没有可用的图像");
        qDebug() << "No image available for saving";
    }
}

void CameraPreviewDialog::saveCameraSettings()
{
    QSettings settings("FastestImagePatternMatching", "CameraPreview");
    
    // 保存摄像头参数设置
    settings.setValue("ExposureTime", m_exposureSpinBox->value());
    settings.setValue("AnalogGain", m_gainSpinBox->value());
    settings.setValue("SoftTriggerMode", m_triggerCheckBox->isChecked());
    
    // 保存最后选择的摄像头
    if (!m_cameraComboBox->currentText().isEmpty()) {
        settings.setValue("LastSelectedCamera", m_cameraComboBox->currentText());
    }
    
    qDebug() << "摄像头设置已保存";
}

void CameraPreviewDialog::loadCameraSettings()
{
    QSettings settings("FastestImagePatternMatching", "CameraPreview");
    
    // 加载摄像头参数设置
    double exposureTime = settings.value("ExposureTime", 10000.0).toDouble();
    double analogGain = settings.value("AnalogGain", 1.0).toDouble();
    bool softTriggerMode = settings.value("SoftTriggerMode", false).toBool();
    
    // 设置UI控件的值
    m_exposureSpinBox->setValue(exposureTime);
    m_gainSpinBox->setValue(analogGain);
    m_triggerCheckBox->setChecked(softTriggerMode);
    
    // 加载最后选择的摄像头
    QString lastCamera = settings.value("LastSelectedCamera", "").toString();
    if (!lastCamera.isEmpty()) {
        // 注意：这里只是设置文本，实际的摄像头列表需要等扫描完成后才能选择
        // 我们将在扫描完成后尝试选择这个摄像头
    }
    
    qDebug() << "摄像头设置已加载";
} 