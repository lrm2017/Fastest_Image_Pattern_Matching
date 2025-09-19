#include "BasicFunction.h"
#include "ImageAcquisition.h"
#include "ui_BasicFunction.h"
#include <QFileDialog>
#include <QDate>
#include "DVPCamera.h"

QBasicFunction::QBasicFunction(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QBasicFunction)
{
    ui->setupUi(this);

    // 成员变量初始化
    m_handle = 0;
    SoftTriggerFlag = false;

    // 预先浏览相机设备
    on_toolButton_Scan_clicked();

    //创建定时器
    m_Timer = new QTimer(this);

    //将定时器超时信号与槽(功能函数)联系起来
    connect( m_Timer, SIGNAL(timeout()), this, SLOT(slotDispRate()));

    //创建显示容器
    scene = new QGraphicsScene();
    ui->graphicsView_PreView->setScene(scene);

}

QBasicFunction::~QBasicFunction()
{
    delete ui;
}

bool QBasicFunction::IsValidHandle(dvpHandle handle)
{
    bool bValidHandle;
    dvpIsValid(handle, &bValidHandle);
    return bValidHandle;
}

void QBasicFunction::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void QBasicFunction::closeEvent(QCloseEvent *)
{
    if (IsValidHandle(m_handle))
    {
        dvpGetStreamState(m_handle,&state);
        if (STATE_STARTED == state)
        {
            on_pushButton_Start_clicked();
        }

        dvpSaveConfig(m_handle, 0);
        dvpClose(m_handle);
        m_handle = 0;
    }
}

void QBasicFunction::UpdateControls()
{
    dvpStatus status;

    if (IsValidHandle(m_handle))
    {
        // 此时已经打开了一个设备,更新基本功能控件
        dvpStreamState state;
        status = dvpGetStreamState(m_handle, &state);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get count fail!");
        }

        // 判断相机是否为彩色相机
        dvpSensorInfo sSensorInfo;
        status = dvpGetSensorInfo(m_handle,&sSensorInfo);
        if (status == DVP_STATUS_OK)
        {
            if (SENSOR_PIXEL_MONO == sSensorInfo.pixel)
                bMono = true;
            else
                bMono = false;
        }
        else if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get sensor information fail!");
        }

        // 更新基本控件
        ui->pushButton_Start->setText(state == STATE_STARTED ? (tr("Stop")) : (tr("Start")));
        ui->pushButton_Open->setText("Close");
        ui->pushButton_Start->setEnabled(true);
        ui->pushButton_iniSave->setEnabled(true);
        ui->pushButton_Property->setEnabled(true);

        // 更新示例相关功能控件
        ui->groupBox_AEOper->setEnabled(true);
        ui->groupBox_trigger->setEnabled(true);
        ui->comboBox_AEMode->setEnabled(true);
        ui->pushButton_OnceWB->setEnabled(true);
        ui->comboBox_ROI->setEnabled(true);
        ui->doubleSpinBox_ExpoTime->setEnabled(false);
        ui->doubleSpinBox_Gain->setEnabled(false);

        if (STATE_STARTED == state)
        {
            ui->pushButton_Load->setEnabled(false);
            ui->pushButton_Save->setEnabled(true);
        }
        else
        {
            ui->pushButton_Load->setEnabled(true);
            ui->pushButton_Save->setEnabled(false);
        }

        if (!bMono)
        {
            ui->groupBox_AWB->setEnabled(true);
            ui->pushButton_OnceWB->setEnabled(true);
        }
        else
        {
            ui->groupBox_AWB->setEnabled(false);
            ui->pushButton_OnceWB->setEnabled(false);
        }

        if (STATE_STARTED == state && false == SoftTriggerFlag)
            ui->groupBox_trigger->setEnabled(false);
        else
            ui->groupBox_trigger->setEnabled(true);


        if (ui->groupBox_trigger->isChecked())
            ui->pushButton_trigger->setEnabled(true);
        else
            ui->pushButton_trigger->setEnabled(false);

        // 更新自动白平衡操作
        if (ui->groupBox_AWB->isChecked())
            ui->pushButton_OnceWB->setEnabled(false);
        else
            ui->pushButton_OnceWB->setEnabled(true);

        // 彩色相机更新白平衡操作
        dvpAwbOperation AwbOper;
        if (!bMono)
        {
            status = dvpGetAwbOperation(m_handle,&AwbOper);
            if (status != DVP_STATUS_OK)
            {
                QMessageBox::about(NULL,"About","Get Awb operation fail!");
            }

            if (AWB_OP_OFF == AwbOper)
               ui->groupBox_AWB->setChecked(false);
            else
               ui->groupBox_AWB->setChecked(true);
        }

        // 更新自动曝光操作控件
        dvpAeOperation AeOp;
        dvpAeMode AeMode;

        status = dvpGetAeOperation(m_handle,&AeOp);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get AE operation fail!");
        }

        status = dvpGetAeMode(m_handle, &AeMode);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get AE mode fail!");
        }

        if (AeOp == AE_OP_OFF)
        {
            ui->spinBox_AETarget->setEnabled(false);
            ui->groupBox_AEOper->setChecked(false);
            ui->doubleSpinBox_ExpoTime->setEnabled(true);
            ui->doubleSpinBox_Gain->setEnabled(true);
        }
        else if (AeOp == AE_OP_CONTINUOUS || AeOp == AE_OP_ONCE)
        {
            ui->spinBox_AETarget->setEnabled(true);
            ui->groupBox_AEOper->setChecked(true);
            ui->doubleSpinBox_ExpoTime->setEnabled(AeMode == AE_MODE_AG_ONLY);
            ui->doubleSpinBox_Gain->setEnabled(AeMode == AE_MODE_AE_ONLY);
        }

        // 获取自动曝光模式并更新自动曝光模式设置控件
        ui->comboBox_AEMode->blockSignals(true);
        ui->comboBox_AEMode->setCurrentIndex(AeMode);
        ui->comboBox_AEMode->blockSignals(false);


        // 更新抗频闪设置控件
        dvpAntiFlick AntiFlick;
        status = dvpGetAntiFlick(m_handle, &AntiFlick);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get anti flick fail!");
        }

        // 更新曝光时间
        double fExpoTime;
        status = dvpGetExposure(m_handle,&fExpoTime);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get exposure time fail!");
        }

        ui->doubleSpinBox_ExpoTime->blockSignals(true);
        ui->doubleSpinBox_ExpoTime->setValue(fExpoTime);
        ui->doubleSpinBox_ExpoTime->blockSignals(false);


        // 更新增益
        float fGain;
        status = dvpGetAnalogGain(m_handle,&fGain);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get analog gain fail!");
        }

        ui->doubleSpinBox_Gain->blockSignals(true);
        ui->doubleSpinBox_Gain->setValue(fGain);
        ui->doubleSpinBox_Gain->blockSignals(false);


        // 更新自动曝光目标亮度
        dvpInt32 iAETargetVal;
        status = dvpGetAeTarget(m_handle,&iAETargetVal);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get AE target fail!");
        }

        ui->spinBox_AETarget->blockSignals(true);
        ui->spinBox_AETarget->setValue(iAETargetVal);
        ui->spinBox_AETarget->blockSignals(false);


        // 更新分辨率
        dvpUint32 QRoiSel;
        status = dvpGetQuickRoiSel(m_handle, &QRoiSel);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get quick roi fail!");
        }

        ui->comboBox_ROI->blockSignals(true);
        ui->comboBox_ROI->setCurrentIndex(QRoiSel);
        ui->comboBox_ROI->blockSignals(false);
    }
    else
    {
        // 此时设备还没有被打开,更新基本功能控件
        ui->pushButton_Open->setText("Open");
        ui->pushButton_Start->setText("Start");
        ui->pushButton_Start->setEnabled(false);
        ui->pushButton_Save->setEnabled(false);
        ui->pushButton_Load->setEnabled(false);
        ui->pushButton_iniSave->setEnabled(false);
        ui->pushButton_Property->setEnabled(false);
        if (ui->comboBox_Devices->count() == 0)
        {
            ui->pushButton_Open->setEnabled(false);
        }
        else
        {
            ui->pushButton_Open->setEnabled(true);
        }

        ui->pushButton_OnceWB->setEnabled(false);
        ui->groupBox_AEOper->setEnabled(false);
        ui->groupBox_trigger->setEnabled(false);
        ui->groupBox_AWB->setEnabled(false);
        ui->comboBox_AEMode->setEnabled(false);
        ui->comboBox_ROI->setEnabled(false);
        ui->doubleSpinBox_ExpoTime->setEnabled(false);
        ui->doubleSpinBox_Gain->setEnabled(false);
        ui->spinBox_AETarget->setEnabled(false);
    }
}

void QBasicFunction::InitAEMode()
{
    ui->comboBox_AEMode->clear();

    if (IsValidHandle(m_handle))
    {
        // 插入自动曝光模式索引
        ui->comboBox_AEMode->blockSignals(true);
        ui->comboBox_AEMode->insertItem(0,tr("AE_MODE_AE_AG"));
        ui->comboBox_AEMode->insertItem(1,tr("AE_MODE_AG_AE"));
        ui->comboBox_AEMode->insertItem(2,tr("AE_MODE_AE_ONLY"));
        ui->comboBox_AEMode->insertItem(3,tr("AE_MODE_AG_ONLY"));

        ui->comboBox_AEMode->setCurrentIndex(0);
        ui->comboBox_AEMode->blockSignals(false);
    }
}

void QBasicFunction::InitAETarget()
{
    dvpStatus status;
    QString strValue;
    dvpInt32  iAETarget;
    dvpIntDescr sAeTargetDescr;

    if (IsValidHandle(m_handle))
    {

        status = dvpGetAeTargetDescr(m_handle, &sAeTargetDescr);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get AE target description fails!");
        }
        else
        {
            ui->spinBox_AETarget->blockSignals(true);
            ui->spinBox_AETarget->setRange(sAeTargetDescr.iMin,sAeTargetDescr.iMax);
            ui->spinBox_AETarget->blockSignals(false);
        }

        status = dvpGetAeTarget(m_handle,&iAETarget);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get AE target fail!");
        }
        else
        {
            ui->spinBox_AETarget->blockSignals(true);
            ui->spinBox_AETarget->setValue(iAETarget);
            ui->spinBox_AETarget->blockSignals(false);
        }
    }
}

void QBasicFunction::InitTrigger()
{
    dvpStatus status;
//    bool IsTriggered;
    if(IsValidHandle(m_handle))
    {
        status = dvpGetTriggerState(m_handle, &SoftTriggerFlag);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get Trigger description fail!");
        }
        if(SoftTriggerFlag)
        {
            ui->groupBox_trigger->blockSignals(true);
            ui->groupBox_trigger->setChecked(true);
            ui->groupBox_trigger->blockSignals(false);
        }
        else
        {
            ui->groupBox_trigger->blockSignals(true);
            ui->groupBox_trigger->setChecked(false);
            ui->groupBox_trigger->blockSignals(false);
        }
    }
}

void QBasicFunction::InitSpinExpoTime()
{
    double fExpoTime;
    dvpDoubleDescr ExpoTimeDescr;
    dvpStatus status;

    if (IsValidHandle(m_handle))
    {
        // 获取曝光时间的描述信息
        status = dvpGetExposureDescr(m_handle, &ExpoTimeDescr);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get exposure time description fail!");
        }
        else
        {
            ui->doubleSpinBox_ExpoTime->blockSignals(true);
            ui->doubleSpinBox_ExpoTime->setRange(ExpoTimeDescr.fMin,ExpoTimeDescr.fMax);
            ui->doubleSpinBox_ExpoTime->blockSignals(false);
        }

        // 获取曝光时间的初值
        status = dvpGetExposure(m_handle, &fExpoTime);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get exposure time fail!");
        }
        else
        {
            // 设置曝光时间拖动条初始值
            ui->doubleSpinBox_ExpoTime->blockSignals(true);
            ui->doubleSpinBox_ExpoTime->setValue(fExpoTime);
            ui->doubleSpinBox_ExpoTime->blockSignals(false);
        }
    }
}

void QBasicFunction::InitSpinGain()
{
    dvpStatus       status;
    float           fAnalogGain;
    dvpFloatDescr   AnalogGainDescr;

    if (IsValidHandle(m_handle))
    {
        // 获取模拟增益的描述信息
        status = dvpGetAnalogGainDescr(m_handle,&AnalogGainDescr);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get analog gain description fail!");
        }
        else
        {
            ui->doubleSpinBox_Gain->blockSignals(true);
            ui->doubleSpinBox_Gain->setRange((double)AnalogGainDescr.fMin,(double)AnalogGainDescr.fMax);
            ui->doubleSpinBox_Gain->blockSignals(false);
        }

        // 获取模拟增益并设置模拟增益的初始值
        status = dvpGetAnalogGain(m_handle, &fAnalogGain);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get analog gain fail!");
        }
        else
        {
            ui->doubleSpinBox_Gain->blockSignals(true);
            ui->doubleSpinBox_Gain->setValue((double)fAnalogGain);
            ui->doubleSpinBox_Gain->blockSignals(false);
        }
    }
}

void QBasicFunction::InitROIMode()
{
    dvpUint32 QuickRoiSel = 0;
    dvpQuickRoi QuickRoiDetail;
    dvpStatus status;
    dvpSelectionDescr QuickRoiDescr;

    if (IsValidHandle(m_handle))
    {
        ui->comboBox_ROI->clear();

        // 获取相机分辨率索引号
        status = dvpGetQuickRoiSelDescr(m_handle, &QuickRoiDescr);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get quick roi sel description fail!");
        }
        else
        {
            for (unsigned int iNum = 0; iNum<QuickRoiDescr.uCount; iNum++)
            {
                status = dvpGetQuickRoiSelDetail(m_handle,iNum, &QuickRoiDetail);
                if (status == DVP_STATUS_OK)
                {
                    ui->comboBox_ROI->blockSignals(true);
                    ui->comboBox_ROI->insertItem(iNum,QuickRoiDetail.selection.string);
                    ui->comboBox_ROI->blockSignals(false);
                }
            }
        }

        // 获取分辨率模式索引
        status = dvpGetResolutionModeSel(m_handle,&QuickRoiSel);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get roi sel fail!");
        }
        else
        {
            ui->comboBox_ROI->blockSignals(true);
            ui->comboBox_ROI->setCurrentIndex(QuickRoiSel);
            ui->comboBox_ROI->blockSignals(false);
        }
    }
}

void QBasicFunction::InitTargetFormat()
{
    dvpStatus status;

    if (IsValidHandle(m_handle))
    {
        status = dvpSetTargetFormat(m_handle,S_BGR24);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Set target format fail!");
        }
    }
}

void QBasicFunction::slotDispRate()
{
    dvpStatus status;

    if (IsValidHandle(m_handle))
    {
        // 更新帧率信息
        status = dvpGetFrameCount(m_handle, &FrameCount);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get frame count fail!");
        }

        strFrameCount = QString::number(FrameCount.uFrameCount);
        strFrameRate = QString::number(FrameCount.fFrameRate);
        strFrameInfo = QString::fromLocal8Bit("采集帧数 ") +strFrameCount + QString::fromLocal8Bit(" 采集帧率 ") + strFrameRate +
                QString::fromLocal8Bit(" fps 显示帧率 ")+QString::number(m_DisplayCount-m_DisplayCountBackUp)+QString::fromLocal8Bit(" fps");
        m_DisplayCountBackUp=m_DisplayCount;
        QWidget::setWindowTitle(strFrameInfo);
    }
}

void QBasicFunction::slotDispImage()
{
    if (m_AcquireImage!= NULL)
    {
        if(m_AcquireImage->m_threadMutex.tryLock())
        {
            if (!m_AcquireImage->m_ShowImage.isNull())
            {
                QList<QGraphicsItem*> items = ui->graphicsView_PreView->items();
                for(int i=0;i<items.count();i++)
                {
                    if(items[i]->type() == QGraphicsPixmapItem::Type)
                    {
                        scene->removeItem(items[i]);
                        delete items[i];
                    }
                }
                QGraphicsPixmapItem* loadedPixmapItem=scene->addPixmap(m_AcquireImage->m_ShowImage);
                loadedPixmapItem->setOffset((50000.0 - m_AcquireImage->m_ShowImage.width()/2), (50000.0 - m_AcquireImage->m_ShowImage.height()/2));
                ui->graphicsView_PreView->fitInView(loadedPixmapItem->boundingRect(), Qt::KeepAspectRatio);
                m_DisplayCount++;
            }

            m_AcquireImage->m_threadMutex.unlock();
        }
    }
}

void QBasicFunction::on_pushButton_Open_clicked()
{
     dvpStatus status;
     QString strName;
     dvpStreamState state;

     if (!IsValidHandle(m_handle))
     {
         strName = ui->comboBox_Devices->currentText();

         if (strName != "")
         {
             // 通过枚举到并选择的FriendlyName打开指定设备
             status = dvpOpenByName(strName.toLatin1().data(),OPEN_NORMAL,&m_handle);
             if (status != DVP_STATUS_OK)
             {
                 QMessageBox::about(NULL,"About","Open the camera fail!");
             }

             m_FriendlyName = strName;

             // 初始化曝光模式，曝光时间，模拟增益，消频闪，分辨率
             InitTrigger();
             InitAEMode();
             InitAETarget();
             InitSpinExpoTime();
             InitSpinGain();
             InitROIMode();
             InitTargetFormat();

             status = dvpGetStreamState(m_handle,&state);
             if (status != DVP_STATUS_OK)
             {
                 QMessageBox::about(NULL,"About","Get the stream state fail!");
             }

             if (state == STATE_STARTED)
             {
                 status = dvpStop(m_handle);
             }
         }
     }
     else
     {
         // 如果图像正在采集，则调用on_pushButton_Start_clicked停止采集
         status = dvpGetStreamState(m_handle,&state);
         if (state == STATE_STARTED)
         {
             on_pushButton_Start_clicked();
         }

         status = dvpSaveConfig(m_handle, 0);
         status = dvpClose(m_handle);
         m_handle = 0;
     }

     UpdateControls();
}

void QBasicFunction::on_pushButton_Start_clicked()
{
    dvpStatus status;
    dvpStreamState state;
    bool bTrigStatus;

    if (IsValidHandle(m_handle))
    {
        status = dvpGetStreamState(m_handle,&state);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get the stream state fail!");
        }

        if (state == STATE_STOPED)
        {
            status = dvpGetTriggerState(m_handle,&bTrigStatus);
            if (status != DVP_STATUS_FUNCTION_INVALID)
            {
                // 在启动视频流之前先设置为触发模式
                status = dvpSetTriggerState(m_handle,SoftTriggerFlag ? true : false);
                if (status != DVP_STATUS_OK)
                {
                     QMessageBox::about(NULL,"About","Set status of trigger fail!");
                }
            }
            else
            {
                ui->groupBox_trigger->setEnabled(false);
            }

            status = dvpStart(m_handle);
            if (status != DVP_STATUS_OK)
            {
                QMessageBox::about(NULL,"About","Start the video stream fail!");
            }
            else
            {
                // 创建线程对象
                m_AcquireImage = new QImageAcquisition(m_handle);
                // 建立图像显示信号和槽函数的联系
                QObject::connect(m_AcquireImage,SIGNAL(signalDisplay()),this,SLOT(slotDispImage()));
                // 开启计算显示帧率的定时器
                m_Timer->start(1000);
            }
        }
        else
        {
            status = dvpStop(m_handle);
//            m_AcquireImage->m_bAcquireImg = false;

            //停止计算显示帧率的定时器
            if ( m_Timer->isActive() )
            {
                m_Timer->stop();
            }
            if(m_AcquireImage->m_timer->isActive())
            {
                m_AcquireImage->m_timer->stop();
            }
            m_AcquireImage->StopThread();
            //删除线程对象
            if (m_AcquireImage != NULL)
            {
                delete m_AcquireImage;
                m_AcquireImage = NULL;
            }

            ui->graphicsView_PreView->scene()->clear();
        }
    }

    UpdateControls();
}

void QBasicFunction::on_pushButton_Save_clicked()
{
    strFilePath = "";
    if (m_AcquireImage == NULL || m_AcquireImage->m_ShowImage.isNull())
    {
        return;
    }

    strFilePath = QFileDialog::getSaveFileName(this,tr("Save image"),".","*.bmp");
    QFileInfo fi = (strFilePath);

    if ("bmp" != fi.suffix())
    {
        strFilePath = strFilePath + ".bmp";
    }

    m_AcquireImage->m_threadMutex.lock();
    if (strFilePath.isEmpty())
    {
        return;
    }
    else
    {
       if(!m_AcquireImage->m_ShowImage.save(strFilePath,"bmp",-1))
       {
           qDebug("Save image fail!");
       }
    }

    m_AcquireImage->m_threadMutex.unlock();
}

void QBasicFunction::on_comboBox_AEMode_currentIndexChanged(int index)
{
    dvpStatus status;
     index = ui->comboBox_AEMode->currentIndex();

     if (index > 3)
     {
         return;
     }

     // 曝光优先
     if (index ==0)
     {
         status = dvpSetAeMode(m_handle,AE_MODE_AE_AG);
         if (status != DVP_STATUS_OK)
         {
             QMessageBox::about(NULL,"About","Set AE mode fail!");
         }
     }

     // 增益优先
     if (index == 1)
     {
         status = dvpSetAeMode(m_handle,AE_MODE_AG_AE);
         if (status != DVP_STATUS_OK)
         {
             QMessageBox::about(NULL,"About","Set AE mode fail!");
         }
     }

     // 仅打开曝光
     if (index == 2)
     {
         status = dvpSetAeMode(m_handle,AE_MODE_AE_ONLY);
         if (status != DVP_STATUS_OK)
         {
             QMessageBox::about(NULL,"About","Set AE mode fail!");
         }
     }

     // 仅打开增益
     if(index == 3)
     {
         status = dvpSetAeMode(m_handle,AE_MODE_AG_ONLY);
         if (status != DVP_STATUS_OK)
         {
             QMessageBox::about(NULL,"About","Set AE mode fail!");
         }
     }

     UpdateControls();
}

void QBasicFunction::on_comboBox_ROI_currentIndexChanged(int index)
{
    dvpStatus status;

    // 获取分辨率选择索引
    index = ui->comboBox_ROI->currentIndex();
    if (index < 0)
    {
        return;
    }

    // 关闭视频流
    status = dvpStop(m_handle);
    if (status != DVP_STATUS_OK)
    {
        QMessageBox::about(NULL,"About","Stop the video stream fail!");
    }

    // 设置相机分辨率
    status = dvpSetQuickRoiSel(m_handle,index);
    if (status != DVP_STATUS_OK)
    {
        QMessageBox::about(NULL,"About","Set roi fail!");
    }

    // 打开视频流
    status = dvpStart(m_handle);
    if (status != DVP_STATUS_OK)
    {
        QMessageBox::about(NULL,"About","Start the video stream fail!");
    }
}

void QBasicFunction::on_pushButton_OnceWB_clicked()
{
    dvpStatus status;

    status = dvpSetAwbOperation(m_handle,AWB_OP_ONCE);
    if (status != DVP_STATUS_OK)
    {
        QMessageBox::about(NULL,"About","Set once AWB operation fail!");
    }
}

void QBasicFunction::on_toolButton_Scan_clicked()
{
    dvpStatus status;
    dvpUint32 i,n = 0;
    dvpCameraInfo info[16];

    // 此时，n为成功枚举到的相机数量，将添加到下拉列表中，下拉列表中的内容为每个相机的FriendlyName
     ui->comboBox_Devices->clear();

    // 获得当前能连接的相机数量
    status = dvpRefresh(&n);
    if (status != DVP_STATUS_OK)
    {
        QMessageBox::about(NULL,"About","Refresh fail!");
    }

    if (status == DVP_STATUS_OK)
    {
        // 枚举最多16台相机的信息
        if (n > 16)
        {
            n = 16;
        }

        for (i = 0; i < n; i++)
        {
            // 逐个枚举出每个相机的信息
            status = dvpEnum(i, &info[i]);
            if (status != DVP_STATUS_OK)
            {
                QMessageBox::about(NULL,"About","Enumerate fail!");
            }
            else
            {
                 ui->comboBox_Devices->addItem(tr(info[i].FriendlyName));
            }
        }
        if (i != 0)
        {
            ui->comboBox_Devices->setCurrentIndex(0);
        }

        UpdateControls();
    }
}

void QBasicFunction::on_pushButton_Load_clicked()
{
   char *LoadPath = NULL;
   QByteArray transVar;
   dvpStatus status;

    strLoadPath = QFileDialog::getOpenFileName(this,tr("Get file name"), ".", tr("Files (*.ini)"));

    transVar = strLoadPath.toLatin1();
    LoadPath = transVar.data();

    status = dvpLoadConfig(m_handle,LoadPath);
    if (status != DVP_STATUS_OK)
    {
        QMessageBox::about(NULL,"About","Load fail!");
    }
}

void QBasicFunction::on_groupBox_AWB_clicked()
{
    dvpStatus status;
    if (ui->groupBox_AWB->isChecked())
    {
        status = dvpSetAwbOperation(m_handle,AWB_OP_CONTINUOUS);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Set AWB operation fail!");
        }

        ui->pushButton_OnceWB->setEnabled(false);
    }
    else
    {
        status = dvpSetAwbOperation(m_handle,AWB_OP_OFF);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Set AWB operation fail!");
        }

        ui->pushButton_OnceWB->setEnabled(true);
    }

    UpdateControls();
}

void QBasicFunction::on_groupBox_AEOper_clicked()
{
    dvpStatus status;
    if (ui->groupBox_AEOper->isChecked())
    {
        status = dvpSetAeOperation(m_handle, AE_OP_CONTINUOUS);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Set AE operation fail!");
        }
    }
    else
    {
        status = dvpSetAeOperation(m_handle,AE_OP_OFF);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Set AE operation fail!");
        }
    }

    UpdateControls();
}

void QBasicFunction::on_pushButton_trigger_clicked()
{
    dvpStatus status;
    bool triggerState;

    status = dvpGetTriggerState(m_handle,&triggerState);
    if (status != DVP_STATUS_FUNCTION_INVALID)
    {
        status = dvpTriggerFire(m_handle);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Trigger fail!");
        }
    }
}

void QBasicFunction::on_groupBox_trigger_clicked()
{
    if (ui->groupBox_trigger->isChecked())
    {
        SoftTriggerFlag = true;
    }
    else
    {
        SoftTriggerFlag = false;
    }

    UpdateControls();
}

void QBasicFunction::on_pushButton_iniSave_clicked()
{
    char *SaveIniPath = NULL;
    QByteArray transiniVar;
    QString  strSaveIni;
    dvpStatus status;

    strSaveIni = QFileDialog::getSaveFileName(this,tr("Save filename"),".","*.ini");

    QFileInfo fi = (strSaveIni);

    if ("ini" != fi.suffix())
    {
        strSaveIni = strSaveIni + ".ini";
    }

    transiniVar = strSaveIni.toLatin1();
    SaveIniPath = transiniVar.data();

    status = dvpSaveConfig(m_handle, SaveIniPath);
    if (DVP_STATUS_OK != status)
    {
        QMessageBox::about(NULL,"About","Save ini file fail!");
    }
}

void QBasicFunction::on_doubleSpinBox_ExpoTime_valueChanged(double fExposureTime)
{
    dvpStatus status;
    dvpDoubleDescr ExpDescr;

    fExposureTime= ui->doubleSpinBox_ExpoTime->value();

    // 设置曝光时间
    // 获取曝光时间的范围，避免设置值超出范围
    status = dvpGetExposureDescr(m_handle, &ExpDescr);
    if (status != DVP_STATUS_OK)
    {
        QMessageBox::about(NULL,"About","Get exposure time description fail!");
    }

    if (fExposureTime > ExpDescr.fMax)
        fExposureTime = ExpDescr.fMax;

    if (fExposureTime < ExpDescr.fMin)
        fExposureTime = ExpDescr.fMin;

    // 先设置曝光时间
    status = dvpSetExposure(m_handle,fExposureTime);
    if (status != DVP_STATUS_OK)
    {
        QMessageBox::about(NULL,"About","Set exposure time fail!");
    }
}

void QBasicFunction::on_doubleSpinBox_ExpoTime_editingFinished()
{
    double fExposureTime = 0.0;
    on_doubleSpinBox_ExpoTime_valueChanged(fExposureTime);
}

void QBasicFunction::on_doubleSpinBox_Gain_editingFinished()
{
    double fGain = 0.0;
    on_doubleSpinBox_Gain_valueChanged(fGain);
}

void QBasicFunction::on_doubleSpinBox_Gain_valueChanged(double fGain)
{
    dvpStatus status;
    float fAnalogGain;
    dvpFloatDescr sAnalogGainDescr;

    fGain = ui->doubleSpinBox_Gain->value();
    fAnalogGain = fGain;

    status = dvpGetAnalogGainDescr(m_handle,&sAnalogGainDescr);
    if (status != DVP_STATUS_OK)
    {
        QMessageBox::about(NULL,"About","Get analog gain description fail!");
    }

    if (fAnalogGain > sAnalogGainDescr.fMax)
        fAnalogGain = sAnalogGainDescr.fMax;

    if (fAnalogGain < sAnalogGainDescr.fMin)
        fAnalogGain = sAnalogGainDescr.fMin;

    status = dvpSetAnalogGain(m_handle,fAnalogGain);
    if (status != DVP_STATUS_OK)
    {
        QMessageBox::about(NULL,"About","Set analog gain fail!");
    }
}

void QBasicFunction::on_spinBox_AETarget_editingFinished()
{
    int iAETarget = 0;
    on_spinBox_AETarget_valueChanged(iAETarget);
}

void QBasicFunction::on_spinBox_AETarget_valueChanged(int iAETarget)
{
    dvpStatus status;
    dvpIntDescr sAeTargetDescr;

    if (IsValidHandle(m_handle))
    {
        iAETarget = ui->spinBox_AETarget->value();

        status = dvpGetAeTargetDescr(m_handle,&sAeTargetDescr);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Get AE target description fail!");
        }

        if (iAETarget > sAeTargetDescr.iMax)
            iAETarget = sAeTargetDescr.iMax;

        if (iAETarget < sAeTargetDescr.iMin)
            iAETarget = sAeTargetDescr.iMin;

        status = dvpSetAeTarget(m_handle,iAETarget);
        if (status != DVP_STATUS_OK)
        {
            QMessageBox::about(NULL,"About","Set AE target value fail!");
        }
    }
}

//==========================================================
// 仅在windows有效
//==========================================================
void QBasicFunction::on_pushButton_Property_clicked()
{
    if (IsValidHandle(m_handle))
    {
#ifdef Q_OS_WIN32
        //this就是要获取句柄的窗体的类名；
        dvpShowPropertyModalDialog(m_handle, (HWND)this->winId());
#endif
        UpdateControls();
    }
}
