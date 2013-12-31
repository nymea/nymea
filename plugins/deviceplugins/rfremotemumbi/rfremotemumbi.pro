TEMPLATE = lib
CONFIG += plugin static

TARGET = $$qtLibraryTarget(hive_rfremotemumbi)

INCLUDEPATH += ../../../libhive
LIBS += -L../../../libhive -lhive

SOURCES += \
    rfremotemumbi.cpp

HEADERS += \
    rfremotemumbi.h


