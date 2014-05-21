#-------------------------------------------------
#
# Project created by QtCreator 2014-02-11T18:05:21
#
#-------------------------------------------------

QT       += core network gui
QT       += multimedia multimediawidgets
QT       += network

TARGET = client
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

unix {          # Esta configuración específica de Linux y UNIX
    # Variables
    #
    isEmpty(PREFIX) {
        PREFIX = /usr
    }
    BINDIR  = $$PREFIX/bin
    DATADIR = $$PREFIX/share
    CONFDIR = /etc


    isEmpty(VARDIR) {
        VARDIR  = /var/lib/$${TARGET}
    }

    DEFINES += APP_DATADIR=\\\"$$DATADIR\\\"
    DEFINES += APP_VARDIR=\\\"$$VARDIR\\\"
    DEFINES += APP_CONFFILE=\\\"$$CONFDIR/$${TARGET}.ini\\\"

    # Install
    #
    INSTALLS += target config vardir

    ## Instalar ejecutable
    target.path = $$BINDIR

    ## Instalar archivo de configuración
    config.path = $$CONFDIR
    config.files += $${TARGET}.ini

    ## Crear directorio de archivos variables
    vardir.path = $$VARDIR
    vardir.commands = :
}
