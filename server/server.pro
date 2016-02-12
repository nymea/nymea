include(../guh.pri)

TARGET = guhd
TEMPLATE = app

INCLUDEPATH += ../libguh jsonrpc

isEmpty(PREFIX) {
    INSTALLDIR = /usr/bin
} else {
    INSTALLDIR = $$PREFIX/usr/bin/
}

target.path = $$INSTALLDIR
INSTALLS += target

QT += sql xml

LIBS += -L$$top_builddir/libguh/ -lguh

include(server.pri)
include(qtservice/qtservice.pri)

SOURCES += main.cpp \
    guhservice.cpp \
    guhapplication.cpp

boblight {
    xcompile {
        LIBS += -L../plugins/deviceplugins/boblight -lguh_devicepluginboblight -lboblight
    } else {
        LIBS += -L../plugins/deviceplugins/boblight -lguh_devicepluginboblight -L/usr/local/lib/ -lboblight
    }
    DEFINES += USE_BOBLIGHT
}

HEADERS += \
    guhservice.h \
    guhapplication.h
