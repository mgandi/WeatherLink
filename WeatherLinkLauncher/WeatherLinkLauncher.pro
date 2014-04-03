#-------------------------------------------------
#
# Project created by QtCreator 2014-04-03T14:05:29
#
#-------------------------------------------------

QT       += core sql

QT       -= gui

TARGET = wl_launcher
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    weatherlinklauncher.cpp

HEADERS += \
    weatherlinklauncher.h

target.path = /usr/bin
INSTALLS += target
