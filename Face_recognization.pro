#-------------------------------------------------
#
# Project created by QtCreator 2021-01-18T19:55:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Face_recognization
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui
INCLUDEPATH += E:/opencv/opencv3.4-install/install/include
INCLUDEPATH += E:/opencv/opencv3.4-install/install/include/opencv
INCLUDEPATH += E:/opencv/opencv3.4-install/install/include/opencv2
LIBS += E:/opencv/opencv3.4-install/install/x86/mingw/lib/libopencv_*.a
