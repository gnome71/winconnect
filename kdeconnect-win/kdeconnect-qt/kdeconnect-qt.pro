#-------------------------------------------------
#
# Project created by QtCreator 2016-10-13T06:35:51
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kdeconnect-qt
TEMPLATE = app

#Application version
VERSION_MAJOR = 0
VERSION_MINOR = 0
VERSION_BUILD = 1

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"

#Target version
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

SOURCES += main.cpp\
        mainwindow.cpp \
    core/kclogger.cpp \
	core/kdeconnectconfig.cpp
	#core/networkpackage.cpp

HEADERS  += mainwindow.h \
    core/kclogger.h \
	core/kdeconnectconfig.h
	#core/networkpackage.h
	#core/networkpackagetypes.h

FORMS    += mainwindow.ui

RESOURCES += \
    res/resources.qrc

win32: LIBS += -L$$PWD/lib/openssl/openssl-1.0.2j-64bit-debug-DLL-VS2015/lib/ -llibeay32 -lssleay32
INCLUDEPATH += $$PWD/lib/openssl/openssl-1.0.2j-64bit-debug-DLL-VS2015/include
DEPENDPATH += $$PWD/lib/openssl/openssl-1.0.2j-64bit-debug-DLL-VS2015/include

win32: LIBS += -L$$PWD/lib/qca/qca64-debug-dll/libd/ -lqca-qt5d
INCLUDEPATH += $$PWD/lib/qca/qca64-debug-dll/include/Qca-qt5/QtCrypto
DEPENDPATH += $$PWD/lib/qca/qca64-debug-dll/include/Qca-qt5/QtCrypto

