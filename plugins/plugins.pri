include(../guh.pri)

TEMPLATE = lib
CONFIG += plugin c++11

INCLUDEPATH += ../../../libguh
LIBS += -L../../../libguh -lguh

target.path = /usr/lib/guh/plugins/
INSTALLS += target
