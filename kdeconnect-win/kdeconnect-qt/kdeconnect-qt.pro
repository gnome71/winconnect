#-------------------------------------------------
#
# Project created by QtCreator 2016-10-13T06:35:51
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kdeconnect-qt
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    core/kclogger.cpp \
    core/kdeconnectconfig.cpp

HEADERS  += mainwindow.h \
    core/kclogger.h \
    core/kdeconnectconfig.h

FORMS    += mainwindow.ui


win32: LIBS += -L$$PWD/lib/openssl/openssl-1.0.2j-64bit-debug-DLL-VS2015/lib/ -llibeay32 -lssleay32
INCLUDEPATH += $$PWD/lib/openssl/openssl-1.0.2j-64bit-debug-DLL-VS2015/include
DEPENDPATH += $$PWD/lib/openssl/openssl-1.0.2j-64bit-debug-DLL-VS2015/include

win32: LIBS += -L$$PWD/lib/qca/qca64-debug-dll/libd/ -lqca-qt5d
INCLUDEPATH += $$PWD/lib/qca/qca64-debug-dll/include/Qca-qt5/QtCrypto
DEPENDPATH += $$PWD/lib/qca/qca64-debug-dll/include/Qca-qt5/QtCrypto
