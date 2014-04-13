include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginboblight)

INCLUDEPATH += /usr/local/include/
LIBS += -L/usr/local/lib/libboblight.a

SOURCES += \
    devicepluginboblight.cpp \
    bobclient.cpp \

HEADERS += \
    devicepluginboblight.h \
    bobclient.h \



