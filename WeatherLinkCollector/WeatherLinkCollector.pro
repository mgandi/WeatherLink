#-------------------------------------------------
#
# Project created by QtCreator 2014-04-03T14:04:32
#
#-------------------------------------------------

QT       += core network sql

QT       -= gui

TARGET = wl_collector
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    weatherlinkcollector.cpp

HEADERS += \
    weatherlinkcollector.h

target.path = /usr/bin
INSTALLS += target
