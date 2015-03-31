include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginelgato)

QT+= bluetooth

SOURCES += \
    devicepluginelgato.cpp \
    aveabulb.cpp

HEADERS += \
    devicepluginelgato.h \
    aveabulb.h



