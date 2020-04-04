include(../nymea.pri)

TEMPLATE = lib
CONFIG += plugin

QT += network

INCLUDEPATH += $$top_srcdir/libnymea
LIBS += -L../../libnymea -lnymea
HEADERS += plugininfo.h extern-plugininfo.h

# A target for manually updating the plugininfo file
# NOTE: In cross compiling environments we can't run the plugininfocompiler
# so we can't do this automatically at build time
plugininfo.commands = LD_LIBRARY_PATH=$$top_builddir/libnymea \
                      $$top_builddir/tools/nymea-plugininfocompiler/nymea-plugininfocompiler \
                      $$PWD/$${TARGET}/integrationplugin"$$TARGET".json \
                      --output $$PWD/$${TARGET}/plugininfo.h \
                      --extern $$PWD/$${TARGET}/extern-plugininfo.h
QMAKE_EXTRA_TARGETS += plugininfo


# Install plugin
target.path = $$[QT_INSTALL_LIBS]/nymea/plugins/
INSTALLS += target
