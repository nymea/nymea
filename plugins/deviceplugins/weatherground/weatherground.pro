include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginweatherground)

QT+= network

SOURCES += \
    devicepluginweatherground.cpp \
    weathergroundparser.cpp

HEADERS += \
    devicepluginweatherground.h \
    weathergroundparser.h


