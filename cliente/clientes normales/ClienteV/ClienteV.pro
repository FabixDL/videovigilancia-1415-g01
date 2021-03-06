#-------------------------------------------------
#
# Project created by QtCreator 2015-04-17T12:45:16
#
#-------------------------------------------------
PROTOS = VAF.proto
include(protobuf.pri)
include(QtOpenCV.pri)
add_opencv_modules(core video imgproc)

QT       += core gui
QT       += multimedia multimediawidgets
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ClienteV
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        clientev.cpp \
    acercade.cpp \
    combobox.cpp

HEADERS  += clientev.h \
    acercade.h \
    capturebuffer.h \
    combobox.h



FORMS    += clientev.ui \
    acercade.ui \
    combobox.ui

OTHER_FILES +=

unix {
    # Variables
    #
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }

    BINDIR = $$PREFIX/bin
    DATADIR = $$PREFIX/share
    CONFDIR = /etc

    isEmpty(VARDIR) {
        VARDIR = /var/lib/$${TARGET}
    }

    DEFINES += APP_DATADIR="$$DATADIR"
    DEFINES += APP_VARDIR='"$$VARDIR"'
    DEFINES += APP_CONFFILE="$$CONFDIR/$${TARGET}.ini"

    # Install
    #
    INSTALLS += target config desktop icon32 vardir
        ## Instalar ejecutable
        target.path = $$BINDIR

        ## Instalar archivo de configuración
        config.path = $$CONFDIR
        config.files += $${TARGET}.ini

        ## Instalar acceso directo en el menú del escritorio
        desktop.path = $$DATADIR/applications
        desktop.files += $${TARGET}.desktop

        ## Instalar icono de aplicación
        icon32.path = $$DATADIR/icons/hicolor/32x32/apps
        icon32.files += ./data/32x32/$${TARGET}.png

    ## Crear directorio de archivos variables
    vardir.path = $$VARDIR
    vardir.commands = true
}



