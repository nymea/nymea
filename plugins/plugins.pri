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

# Create plugininfo file
infofile.output = plugininfo.h
infofile.depends = $$top_srcdir/plugins/guh-generateplugininfo
infofile.CONFIG = no_link
JSONFILES = deviceplugin"$$TARGET".json
infofile.input = JSONFILES
infofile.commands = $$top_srcdir/plugins/guh-generateplugininfo -j ${QMAKE_FILE_NAME} \
                                                                -o ${QMAKE_FILE_OUT} \
                                                                -b $$OUT_PWD \
                                                                -t $$TRANSLATIONS; \
                    rsync -a "$$OUT_PWD"/translations/*.qm $$top_builddir/translations/;

QMAKE_EXTRA_COMPILERS += infofile
PRE_TARGETDEPS += compiler_infofile_make_all

# Install translation files
translations.path = /usr/share/guh/translations
translations.files = $$[QT_SOURCE_TREE]/translations/*.qm

# Install plugin
target.path = $$GUH_PLUGINS_PATH
INSTALLS += target translations

