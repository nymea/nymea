TEMPLATE = lib
CONFIG += plugin static

TARGET = $$qtLibraryTarget(hive_devicepluginintertechno)

INCLUDEPATH += ../../../libhive
LIBS += -L../../../libhive -lhive

SOURCES += \
    devicepluginintertechno.cpp

HEADERS += \
    devicepluginintertechno.h


