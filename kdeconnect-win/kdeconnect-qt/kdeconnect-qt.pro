#-------------------------------------------------
#
# Project created by QtCreator 2016-09-27T14:49:45
#
#-------------------------------------------------

QT       += core gui network concurrent crypto

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kdeconnect-qt
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    core/kdeconnectconfig.cpp

HEADERS  += mainwindow.h \
    core/kdeconnectconfig.h

FORMS    += mainwindow.ui

LIBS     += lqca

