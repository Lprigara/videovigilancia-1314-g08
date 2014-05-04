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
    console.cpp



HEADERS  += \
    capturebuffer.h \
    console.h


FORMS    +=
