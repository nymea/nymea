include(/usr/include/nymea/plugin.pri)

TARGET = $$qtLibraryTarget(nymea_deviceplugintemplate)

message(============================================)
message("Qt version: $$[QT_VERSION]")
message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    deviceplugintemplate.cpp

HEADERS += \
    deviceplugintemplate.h

