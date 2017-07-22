include(../guh.pri)

TEMPLATE = lib
CONFIG += plugin

QT += network

# Check Bluetooth LE support
contains(DEFINES, BLUETOOTH_LE) {
    QT += bluetooth
}

INCLUDEPATH += $$top_srcdir/libguh
LIBS += -L../../libguh -lguh

# Create plugininfo file
JSONFILES = deviceplugin"$$TARGET".json
plugininfo.target = plugininfo.h
plugininfo.output = plugininfo.h
plugininfo.CONFIG = no_link
plugininfo.input = JSONFILES
plugininfo.commands = touch ${QMAKE_FILE_OUT}; $$top_srcdir/plugins/guh-generateplugininfo \
                            --filetype i \
                            --jsonfile ${QMAKE_FILE_NAME} \
                            --output ${QMAKE_FILE_OUT} \
                            --builddir $$OUT_PWD \
                            --translations $$TRANSLATIONS; \
                      rsync -a "$$OUT_PWD"/translations/*.qm $$top_builddir/translations/;
PRE_TARGETDEPS +=
QMAKE_EXTRA_COMPILERS +=

externplugininfo.target = extern-plugininfo.h
externplugininfo.output = extern-plugininfo.h
externplugininfo.CONFIG = no_link
externplugininfo.input = JSONFILES
externplugininfo.commands = touch ${QMAKE_FILE_OUT}; $$top_srcdir/plugins/guh-generateplugininfo \
                            --filetype e \
                            --jsonfile ${QMAKE_FILE_NAME} \
                            --output ${QMAKE_FILE_OUT} \
                            --builddir $$OUT_PWD \
                            --translations $$TRANSLATIONS;
PRE_TARGETDEPS += compiler_plugininfo_make_all compiler_externplugininfo_make_all
QMAKE_EXTRA_COMPILERS += plugininfo externplugininfo


# Install translation files
translations.path = /usr/share/guh/translations
translations.files = $$[QT_SOURCE_TREE]/translations/*.qm

# Install plugin
target.path = $$GUH_PLUGINS_PATH
INSTALLS += target translations

