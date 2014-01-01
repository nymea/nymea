TEMPLATE = lib
CONFIG += plugin static

TARGET = $$qtLibraryTarget(hive_rfremoteintertechno)

INCLUDEPATH += ../../../libhive
LIBS += -L../../../libhive -lhive

SOURCES += \
    rfremoteintertechno.cpp

HEADERS += \
    rfremoteintertechno.h


