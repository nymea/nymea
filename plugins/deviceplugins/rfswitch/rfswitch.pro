TEMPLATE = lib
CONFIG += plugin static

TARGET = $$qtLibraryTarget(hive_rfswitch)

INCLUDEPATH += ../../../libhive
LIBS += -L../../../libhive -lhive

SOURCES += rfswitch.cpp

HEADERS += rfswitch.h


