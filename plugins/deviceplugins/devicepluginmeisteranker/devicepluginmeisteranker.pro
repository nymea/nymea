TEMPLATE = lib
CONFIG += plugin static

TARGET = $$qtLibraryTarget(hive_devicepluginmeisteranker)

INCLUDEPATH += ../../../libhive
LIBS += -L../../../libhive -lhive

SOURCES += \
    devicepluginmeisteranker.cpp

HEADERS += \
    devicepluginmeisteranker.h


