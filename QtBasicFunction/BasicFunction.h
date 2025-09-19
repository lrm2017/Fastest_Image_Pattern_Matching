#ifndef BASICFUNCTION_H
#define BASICFUNCTION_H

#include <QWidget>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QTime>
#include <QtGlobal>
#include <QThread>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include "ImageAcquisition.h"

//#define TIME_OUT         500    // 定时器超时时间

namespace Ui {
class QBasicFunction;
}

class QBasicFunction : public QWidget
{
    Q_OBJECT

public:
    explicit QBasicFunction(QWidget *parent = 0);
    ~QBasicFunction();

    bool IsValidHandle(dvpHandle handle);
    void InitSpinExpoTime();
    void InitSpinGain();
    void InitAEMode();
    void InitAETarget();
    void InitTrigger();
    void InitROIMode();
    void InitTargetFormat();
    void UpdateControls();

protected:
    void changeEvent(QEvent *e);
protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void on_pushButton_Open_clicked();
    void on_pushButton_Start_clicked();
    void on_pushButton_Save_clicked();
    void on_comboBox_AEMode_currentIndexChanged(int index);
    void on_comboBox_ROI_currentIndexChanged(int index);
    void on_pushButton_OnceWB_clicked();
    void on_toolButton_Scan_clicked();
    void on_pushButton_Load_clicked();
    void on_groupBox_AWB_clicked();
    void on_groupBox_AEOper_clicked();
    void on_pushButton_trigger_clicked();
    void on_groupBox_trigger_clicked();
    void on_pushButton_iniSave_clicked();
    void on_doubleSpinBox_ExpoTime_valueChanged(double arg1);
    void on_doubleSpinBox_ExpoTime_editingFinished();
    void on_doubleSpinBox_Gain_editingFinished();
    void on_doubleSpinBox_Gain_valueChanged(double fGain);
    void on_spinBox_AETarget_editingFinished();
    void on_spinBox_AETarget_valueChanged(int iAETarget);

    // 自定义的槽函数
    void slotDispRate();
    void slotDispImage();
    void on_pushButton_Property_clicked();

private:
    Ui::QBasicFunction *ui;

    bool                SoftTriggerFlag;
    bool                bMono;
    QString             strFrameCount;              // 相机获取到的图像帧数
    QString             strFrameRate;               // 图像获取帧率
    QString             strFrameInfo;
    QString             strFilePath;                // 保存图像时用到的完整路径
    QString             strDir;                     // 图像保存的路径
    QString             strLoadPath;
    QString             m_FriendlyName;

    dvpHandle           m_handle;
    dvpStreamState      state;
    dvpFrameCount       FrameCount;

    QTimer              *m_Timer;                    // 定时器
    QImageAcquisition   *m_AcquireImage;             // 图像采集线程的对象指针
    unsigned int        m_DisplayCountBackUp=0;
    unsigned int        m_DisplayCount=0;

    QGraphicsScene      *scene;
signals:
    void                TriggerSignal(bool triFlags);
    void                PrepareDispSignal();

};

#endif // BASICFUNCTION_H
