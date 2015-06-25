include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginkodi)

RESOURCES += images.qrc

SOURCES += \
    devicepluginkodi.cpp \
    kodiconnection.cpp \
    jsonhandler.cpp \
    kodi.cpp \
    kodireply.cpp

HEADERS += \
    devicepluginkodi.h \
    kodiconnection.h \
    jsonhandler.h \
    kodi.h \
    kodireply.h

