#include "driver_camera.h"
#include <QFile>
#include <unistd.h>

Driver_Camera::Driver_Camera(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QVector<QString>>("QVector<QString>");

    // QString filePath = "/home/lrm/RK3568/DVP2-Linux64_250625/DVP2-Linux64/usb3_m3s_all.dscam.so";
    // while (1)
    // {
    //     if (QFile::exists(filePath))
    //     {
    //         qDebug() << "file exist";
    //         break;
    //     }
    //     else
    //     {
    //         qDebug() << "file not exist";
    //         sleep(1);
    //     }
    // }
    camera_init();
}

void Driver_Camera::camera_init()
{
    vd_timer = new QTimer();
    connect(vd_timer, &QTimer::timeout, this, &Driver_Camera::timer_test);

    ds_vd = new CameraDriver(50);
    ds_vd_thread = new QThread();
    ds_vd->moveToThread(ds_vd_thread);
    ds_vd_thread->start();

    connect(vd_timer, &QTimer::timeout, ds_vd, &CameraDriver::OnGetFrame);

    connect(ds_vd, &CameraDriver::send_scan_results, this, [this](QVector<QString> cameraList){
        if (cameraList.empty())
        {
            qDebug() << "camera list count is : empty";
            /*emit send_scan_camera();*/
            nCam.clear();
        }
        else
        {
            qDebug() << "camera list count is : " << cameraList.size();
            nCam = cameraList;}});
//    connect(this, &MainWindow::send_openCam_to_vd, ds_vd, &CameraDriver::on_openCamera);
    connect(this, &Driver_Camera::send_openCam_to_vd, [=](){
        if (nCam.empty())
        {
            emit send_scan_camera();
            return;
        }
        emit send_open_ds_cam(nCam[0]);
        emit send_start_grab();
        vd_timer->start(16);
    });

    connect(this, &Driver_Camera::send_scan_camera, ds_vd, &CameraDriver::on_scanCamera);
    connect(this, &Driver_Camera::send_open_ds_cam, ds_vd, &CameraDriver::on_openCamera);
    connect(this, &Driver_Camera::send_start_grab, ds_vd, &CameraDriver::on_startCamera);

    connect(this, &Driver_Camera::send_closeCam_to_vd, [=](){
        vd_timer->stop();
        emit send_stop_grab();
        emit send_close_ds_cam();
    });

    connect(this, &Driver_Camera::send_close_ds_cam, ds_vd, &CameraDriver::on_closeCamera);
    connect(this, &Driver_Camera::send_stop_grab, ds_vd, &CameraDriver::on_stopCamera);
    connect(ds_vd, &CameraDriver::send_updateFrame, this, &Driver_Camera::update_showlabel);

//    ds_vd->on_setExposure(10000);
    emit send_scan_camera();
}

void Driver_Camera::timer_test()
{
//    printf("timer test\n");
}

void Driver_Camera::openCamera()
{
    emit send_openCam_to_vd();
}

void Driver_Camera::closeCamera()
{
    emit send_closeCam_to_vd();
}

void Driver_Camera::update_showlabel(QImage frame, float frameRate)
{
//    QImage show_qimg =  frame;
//    show_qimg.scaled(840, 560, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//    show_label->setPixmap(QPixmap::fromImage(show_qimg));
    emit send_frame(frame);
}
