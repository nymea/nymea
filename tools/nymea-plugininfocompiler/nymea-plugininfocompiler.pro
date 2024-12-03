QT -= gui
CONFIG += c++11 console
CONFIG -= app_bundle

include(../../nymea.pri)
INCLUDEPATH += $$top_srcdir/libnymea $$top_builddir

# FIXME: Rebuilding types here so that we can build the nymea-plugininfocompiler
# without having to build the entire libnymea
# Libnymea needs to be split up into 2 parts, one with full hardaremanager support
# the other with something more minimalistic that doesn't pull in Bluetooth, ZigBee,
# MQTT and whatnot.

SOURCES += \
    main.cpp \
    plugininfocompiler.cpp \
    $$top_srcdir/libnymea/integrations/pluginmetadata.cpp \
    $$top_srcdir/libnymea/integrations/thingutils.cpp \
    $$top_srcdir/libnymea/types/paramtype.cpp \
    $$top_srcdir/libnymea/types/param.cpp \
    $$top_srcdir/libnymea/types/vendor.cpp \
    $$top_srcdir/libnymea/types/thingclass.cpp \
    $$top_srcdir/libnymea/types/statetype.cpp \
    $$top_srcdir/libnymea/types/eventtype.cpp \
    $$top_srcdir/libnymea/types/actiontype.cpp \
    $$top_srcdir/libnymea/types/interface.cpp \
    $$top_srcdir/libnymea/types/interfaceparamtype.cpp \
    $$top_srcdir/libnymea/types/interfacestatetype.cpp \
    $$top_srcdir/libnymea/types/interfaceeventtype.cpp \
    $$top_srcdir/libnymea/types/interfaceactiontype.cpp \
    $$top_srcdir/libnymea/loggingcategories.cpp \

HEADERS += \
    plugininfocompiler.h \
    $$top_srcdir/libnymea/typeutils.h \
    $$top_srcdir/libnymea/integrations/pluginmetadata.h \
    $$top_srcdir/libnymea/integrations/thingutils.h \
    $$top_srcdir/libnymea/types/paramtype.h \
    $$top_srcdir/libnymea/types/param.h \
    $$top_srcdir/libnymea/types/vendor.h \
    $$top_srcdir/libnymea/types/thingclass.h \
    $$top_srcdir/libnymea/types/statetype.h \
    $$top_srcdir/libnymea/types/eventtype.h \
    $$top_srcdir/libnymea/types/actiontype.h \
    $$top_srcdir/libnymea/types/interface.h \
    $$top_srcdir/libnymea/types/interfaceparamtype.h \
    $$top_srcdir/libnymea/types/interfacestatetype.h \
    $$top_srcdir/libnymea/types/interfaceeventtype.h \
    $$top_srcdir/libnymea/types/interfaceactiontype.h \
    $$top_srcdir/libnymea/loggingcategories.h \

RESOURCES += \
    $$top_srcdir/libnymea/interfaces/interfaces.qrc


target.path = $$[QT_INSTALL_PREFIX]/bin
INSTALLS += target
