include(../nymea.pri)

TEMPLATE = lib
CONFIG += plugin

QT += network

INCLUDEPATH += $$top_srcdir/libnymea
LIBS += -L../../libnymea -lnymea
HEADERS += plugininfo.h

# Create plugininfo file
plugininfo.target = plugininfo
plugininfo.commands = $$top_srcdir/plugins/nymea-generateplugininfo \
                            --filetype i \
                            --jsonfile $$PWD/$$TARGET/deviceplugin"$$TARGET".json \
                            --output plugininfo.h \
                            --builddir $$OUT_PWD; \
                      $$top_srcdir/plugins/nymea-generateplugininfo \
                            --filetype e \
                            --jsonfile $$PWD/$$TARGET/deviceplugin"$$TARGET".json \
                            --output extern-plugininfo.h \
                            --builddir $$OUT_PWD;
QMAKE_EXTRA_TARGETS += plugininfo

PRE_TARGETDEPS += plugininfo

# Install plugin
target.path = $$[QT_INSTALL_LIBS]/nymea/plugins/
INSTALLS += target
