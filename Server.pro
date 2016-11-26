QT += core
QT += network

CONFIG += c++11

TARGET = Server
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    tcpsock.cpp \
    message.cpp

HEADERS += \
    tcpsock.h \
    message.h \
    stdafx.h
