#-------------------------------------------------
#
# Project created by QtCreator 2021-03-01 10:11:40
#
#-------------------------------------------------

QT       += core gui
INCLUDEPATH += /home/QTCreatorPro/
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BasicFunction
TEMPLATE = app


QMAKE_LFLAGS += -Wl,-rpath=.

SOURCES += main.cpp\
        BasicFunction.cpp \
    ImageAcquisition.cpp

HEADERS  += BasicFunction.h \
    ImageAcquisition.h \
    DVPCamera.h

FORMS    += BasicFunction.ui

#RC_ICONS = BasicFunction.ico

win32{
    contains(QMAKE_HOST.arch, x86_64) {
        message("x86_64 build")
        ## Windows x64 (64bit) specific build here
        LIBS += -L$$PWD/../ -lDVPCamera64
        INCLUDEPATH += $$PWD/../
        DEPENDPATH += $$PWD/../
    }else{
        message("x86_32 build")
        ## Windows x86 (32bit) specific build here
        LIBS += -L$$PWD/../ -lDVPCamera
        INCLUDEPATH += $$PWD/../
        DEPENDPATH += $$PWD/../
    }
}


unix{
    #Linux平台库配置,右键项目配置库
    if(contains(QT_ARCH, x86_64)){
        message("linux_x86_64 build")
        LIBS += -L$$PWD/../ -ldvp

        INCLUDEPATH += $$PWD/../
        DEPENDPATH += $$PWD/../
    }else{
        if(contains(QT_ARCH, arm64)){
            message("linux_arm64 build")
            LIBS += -L$$PWD/../ -ldvp

            INCLUDEPATH += $$PWD/../
            DEPENDPATH += $$PWD/../
        }else{
            contains(QT_ARCH, arm){
                message("linux_arm build")
                LIBS += -L$$PWD/../ -ldvp

                INCLUDEPATH += $$PWD/../
                DEPENDPATH += $$PWD/../
            }
        }
    }
}

