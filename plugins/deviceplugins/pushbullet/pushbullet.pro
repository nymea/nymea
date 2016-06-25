include(plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginpushbullet)

message(============================================)
message(Qt version: $$[QT_VERSION])
message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    devicepluginpushbullet.cpp

HEADERS += \
    devicepluginpushbullet.h
