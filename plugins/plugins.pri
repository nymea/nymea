include(../nymea.pri)

TEMPLATE = lib
CONFIG += plugin

QT += network

INCLUDEPATH += $$top_srcdir/libnymea
LIBS += -L../../libnymea -lnymea
HEADERS += plugininfo.h

# Create plugininfo file
JSONFILE=$$PWD/$$TARGET/deviceplugin"$$TARGET".json
plugininfo.input = JSONFILE
plugininfo.output = plugininfo.h
plugininfo.CONFIG = no_link target_predeps
plugininfo.commands = $$top_srcdir/libnymea/plugin/nymea-generateplugininfo \
                            --filetype i \
                            --jsonfile $$PWD/$$TARGET/deviceplugin"$$TARGET".json \
                            --output plugininfo.h \
                            --builddir $$OUT_PWD;
extern-plugininfo.input = JSONFILE
extern-plugininfo.output = extern-plugininfo.h
extern-plugininfo.CONFIG = no_link target_predeps
extern-plugininfo.commands = $$top_srcdir/libnymea/plugin/nymea-generateplugininfo \
                            --filetype e \
                            --jsonfile $$PWD/$$TARGET/deviceplugin"$$TARGET".json \
                            --output extern-plugininfo.h \
                            --builddir $$OUT_PWD;
# Add it as a compiler, so it will be called before building like moc
QMAKE_EXTRA_COMPILERS += plugininfo extern-plugininfo
# But also add it as a target so we can add it separately without building. E.g. for updating translations.
QMAKE_EXTRA_TARGETS += plugininfo extern-plugininfo

# Install plugin
target.path = $$[QT_INSTALL_LIBS]/nymea/plugins/
INSTALLS += target
