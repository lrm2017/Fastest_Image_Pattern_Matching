#ifndef DRIVER_CAMERA_H
#define DRIVER_CAMERA_H

#include <QObject>
//#include <api/api.hpp>
//#include "api/api_test.h"
#include "DsCamera/video_device.h"
#include <QThread>
#include <QTimer>
#include <iostream>
#include <QDebug>

class Driver_Camera : public QObject
{
    Q_OBJECT
public:
    explicit Driver_Camera(QObject *parent = nullptr);

public slots:

    void openCamera();
    void closeCamera();

    void update_showlabel(QImage frame, float frameRate);

    void timer_test();

signals:
    void send_openCam_to_vd();
    void send_closeCam_to_vd();

    void send_open_ds_cam(QString cam_name);
    void send_close_ds_cam();

    void send_start_grab();
    void send_stop_grab();

    void send_scan_camera();

    void send_frame(QImage qimg);

private:

    QVector<QString> nCam;
    CameraDriver *ds_vd;
    QThread *ds_vd_thread;
    QTimer *vd_timer;

    void camera_init();

    int current_exposure = 0;
    int vd2_current_exposure = 0;

    int save_count = 0;

    QTimer *timer;
};

#endif // DRIVER_CAMERA_H
