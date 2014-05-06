#-------------------------------------------------
#
# Project created by QtCreator 2014-02-11T18:05:21
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia multimediawidgets
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = viewer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    capturebuffer.cpp \
    acerca.cpp \
    sslserver.cpp \
    client.cpp

HEADERS  += mainwindow.h \
    capturebuffer.h \
    acerca.h \
    sslserver.h \
    client.h

FORMS    += mainwindow.ui \
    acerca.ui
