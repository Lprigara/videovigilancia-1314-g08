#-------------------------------------------------
#
# Project created by QtCreator 2014-02-11T18:05:21
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia multimediawidgets
QT       += network
QT       += gui
QT       += sql


TARGET = viewer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app


SOURCES += main.cpp\
    capturebuffer.cpp \
    console.cpp \
    server.cpp \
    clientthread.cpp

HEADERS  += \
    capturebuffer.h \
    console.h \
    server.h \
    clientthread.h

FORMS    +=
