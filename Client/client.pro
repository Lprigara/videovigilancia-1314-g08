#-------------------------------------------------
#
# Project created by QtCreator 2014-02-11T18:05:21
#
#-------------------------------------------------

QT       += core network gui
QT       += multimedia multimediawidgets
QT       += network

TARGET = viewer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    capturebuffer.cpp \
    console.cpp \
    motiondetector.cpp



HEADERS  += \
    capturebuffer.h \
    console.h \
    motiondetector.h



#include qtopencv
include(QtOpenCV.pri)

#include opencv
add_opencv_modules(core video imgproc)
