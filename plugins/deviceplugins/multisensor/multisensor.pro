include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginmultisensor)

message(============================================)
message(Qt version: $$[QT_VERSION])
message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    devicepluginmultisensor.cpp \

HEADERS += \
    devicepluginmultisensor.h \
