include(/usr/include/nymea/plugin.pri)

TARGET = $$qtLibraryTarget(nymea_devicepluginsimplebutton)

message(============================================)
message("Qt version: $$[QT_VERSION]")
message("Building $$deviceplugin$${TARGET}.so")

SOURCES += \
    devicepluginsimplebutton.cpp

HEADERS += \
    devicepluginsimplebutton.h

