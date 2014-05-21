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


TARGET = server
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

unix {          # Esta configuración específica de Linux y UNIX
    # Variables
    #
    isEmpty(PREFIX) {
        PREFIX = /usr
    }
    BINDIR  = $$PREFIX/bin
    DATADIR = $$PREFIX/share
    CONFDIR = /etc
    SCRIPT = /etc/init.d

    isEmpty(VARDIR) {
        VARDIR  = /var/lib/$${TARGET}
    }

    DEFINES += APP_DATADIR=\\\"$$DATADIR\\\"
    DEFINES += APP_VARDIR=\\\"$$VARDIR\\\"
    DEFINES += APP_CONFFILE=\\\"$$CONFDIR/$${TARGET}.ini\\\"

    # Install
    #
    INSTALLS += target ssl config vardir script

    ## Instalar ejecutable
    target.path = $$BINDIR

    ## Instalar archivo de configuración
    config.path = $$CONFDIR
    config.files += $${TARGET}.ini

    ##Instalar configuracion ssl
    ssl.path = $$DATADIR/SSL
    ssl.files += SSL/server.crt \
                 SSL/server.key

    ## Crear directorio de archivos variables
    vardir.path = $$VARDIR
    vardir.commands = :

    ##Script
    script.path = $$SCRIPT
    script.files += $${TARGET}.sh
}
