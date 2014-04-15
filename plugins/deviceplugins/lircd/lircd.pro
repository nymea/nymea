include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginlircd)

QT += network

#INCLUDEPATH += /usr/local/include/
#LIBS += -L/usr/local/lib/libboblight.a

SOURCES += \
    devicepluginlircd.cpp \
    lircdclient.cpp \

HEADERS += \
    devicepluginlircd.h \
    lircdclient.h \



