HEADERS += \
    $$PWD/video_device.h \
    $$PWD/driver_camera.h

SOURCES += \
    $$PWD/video_device.cpp \
    $$PWD/driver_camera.cpp

INCLUDEPATH += $$PWD/../../include

LIBS += $$PWD/../../lib/libdvp.so \
        $$PWD/../../lib/libhzd.so
