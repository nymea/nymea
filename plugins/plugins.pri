include(../guh.pri)

TEMPLATE = lib
CONFIG += plugin static

INCLUDEPATH += ../../../libguh
LIBS += -L../../../libguh -lguh

