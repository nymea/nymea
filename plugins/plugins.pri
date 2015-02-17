include(../guh.pri)

TEMPLATE = lib
CONFIG += plugin

QT += network

INCLUDEPATH += $$top_srcdir/libguh
LIBS += -L../../../libguh -lguh

infofile.output = plugininfo.h
infofile.commands = $$top_srcdir/plugins/generateplugininfo.py ${QMAKE_FILE_NAME} ${QMAKE_FILE_OUT}
infofile.depends = $$top_srcdir/plugins/generateplugininfo.py
infofile.CONFIG = no_link
JSONFILES = deviceplugin"$$TARGET".json
infofile.input = JSONFILES

QMAKE_EXTRA_COMPILERS += infofile

target.path = /usr/lib/guh/plugins/
INSTALLS += target
