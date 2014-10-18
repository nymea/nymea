include(../guh.pri)

TEMPLATE = lib
CONFIG += plugin

QT += network

QT += network

INCLUDEPATH += ../../../libguh
LIBS += -L../../../libguh -lguh

target.path = /usr/lib/guh/plugins/
INSTALLS += target
