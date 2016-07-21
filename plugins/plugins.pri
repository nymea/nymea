include(../guh.pri)

TEMPLATE = lib
CONFIG += plugin

QT += network

# Check Bluetooth LE support
contains(DEFINES, BLUETOOTH_LE) {
    QT += bluetooth
}

INCLUDEPATH += $$top_srcdir/libguh
LIBS += -L../../../libguh -lguh

infofile.output = plugininfo.h
infofile.commands = $$top_srcdir/plugins/guh-generateplugininfo ${QMAKE_FILE_NAME} ${QMAKE_FILE_OUT}
infofile.depends = $$top_srcdir/plugins/guh-generateplugininfo
infofile.CONFIG = no_link
JSONFILES = deviceplugin"$$TARGET".json
infofile.input = JSONFILES

QMAKE_EXTRA_COMPILERS += infofile

target.path = /usr/lib/guh/plugins/
INSTALLS += target

