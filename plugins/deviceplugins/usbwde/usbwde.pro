include(plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginusbwde)

message(============================================)
message(Qt version: $$[QT_VERSION])
message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    devicepluginsubwde.cpp

HEADERS += \
    devicepluginusbwde.h


