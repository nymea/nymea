TEMPLATE = lib
CONFIG += plugin static

TARGET = $$qtLibraryTarget(hive_devicepluginelro)

INCLUDEPATH += ../../../libhive
LIBS += -L../../../libhive -lhive

SOURCES += \
    devicepluginelro.cpp

HEADERS += \
    devicepluginelro.h


