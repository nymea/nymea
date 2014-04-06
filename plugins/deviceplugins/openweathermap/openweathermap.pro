include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginopenweathermap)

QT+= network

SOURCES += \
    devicepluginopenweathermap.cpp \
    openweathermap.cpp

HEADERS += \
    devicepluginopenweathermap.h \
    openweathermap.h


