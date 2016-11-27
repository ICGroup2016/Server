QT += core
QT += network

CONFIG += c++11

TARGET = Server
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    tcpsock.cpp \
    message.cpp \
    roomsrv.cpp \
    Server/main.cpp \
    runtime.cpp

HEADERS += \
    tcpsock.h \
    message.h \
    stdafx.h \
    roomsrv.h \
    runtime.h

SUBDIRS += \
    Server/nbproject/qt-Debug.pro \
    Server/nbproject/qt-Release.pro

DISTFILES += \
    Server/nbproject/Package-Debug.bash \
    Server/nbproject/Package-Release.bash \
    Server/nbproject/private/configurations.xml \
    Server/nbproject/private/private.xml \
    Server/nbproject/configurations.xml \
    Server/nbproject/project.xml \
    Server/nbproject/private/launcher.properties
