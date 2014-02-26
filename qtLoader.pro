#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T16:55:39
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtloader
TEMPLATE = app

unix:!symbian {
    DEFINES += PLATFORM_MAC
}

win32 {
    DEFINES += PLATFORM_WIN32
}

SOURCES += main.cpp\
        mainwindow.cpp \
    downloadmanager.cpp \
    ioapi.cpp \
    unzip.cpp

HEADERS  += mainwindow.h \
    downloadmanager.h \
    ioapi.h \
    unzip.h

FORMS    += mainwindow.ui \
    authenticationdialog.ui

RESOURCES += \
    resources.qrc
