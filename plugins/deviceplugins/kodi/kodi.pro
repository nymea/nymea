include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginkodi)

RESOURCES += images.qrc

SOURCES += \
    devicepluginkodi.cpp \
    kodiconnection.cpp \
    kodijsonhandler.cpp \
    kodi.cpp \
    kodireply.cpp

HEADERS += \
    devicepluginkodi.h \
    kodiconnection.h \
    kodijsonhandler.h \
    kodi.h \
    kodireply.h

