#-------------------------------------------------
#
# Project created by QtCreator 2014-02-11T18:05:21
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia multimediawidgets
QT       += network
QT       += gui


TARGET = viewer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app


SOURCES += main.cpp\
    capturebuffer.cpp \
    client.cpp \
    console.cpp \
    server.cpp

HEADERS  += \
    capturebuffer.h \
    client.h \
    console.h \
    server.h

FORMS    +=
