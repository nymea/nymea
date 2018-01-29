include(../nymea.pri)

TEMPLATE = lib
CONFIG += plugin

QT += network bluetooth

INCLUDEPATH += $$top_srcdir/libnymea
LIBS += -L../../libnymea -lnymea
HEADERS += plugininfo.h

# Create plugininfo file
JSONFILES = deviceplugin"$$TARGET".json
plugininfo.target = plugininfo.h
plugininfo.output = plugininfo.h
plugininfo.CONFIG = no_link
plugininfo.input = JSONFILES
plugininfo.commands = touch ${QMAKE_FILE_OUT}; $$top_srcdir/plugins/nymea-generateplugininfo \
                            --filetype i \
                            --jsonfile ${QMAKE_FILE_NAME} \
                            --output ${QMAKE_FILE_OUT} \
                            --builddir $$OUT_PWD;

externplugininfo.target = extern-plugininfo.h
externplugininfo.output = extern-plugininfo.h
externplugininfo.CONFIG = no_link
externplugininfo.input = JSONFILES
externplugininfo.commands = touch ${QMAKE_FILE_OUT}; $$top_srcdir/plugins/nymea-generateplugininfo \
                            --filetype e \
                            --jsonfile ${QMAKE_FILE_NAME} \
                            --output ${QMAKE_FILE_OUT} \
                            --builddir $$OUT_PWD;
PRE_TARGETDEPS += compiler_plugininfo_make_all compiler_externplugininfo_make_all
QMAKE_EXTRA_COMPILERS += plugininfo externplugininfo

# Install plugin
target.path = $$NYMEA_PLUGINS_PATH
INSTALLS += target
