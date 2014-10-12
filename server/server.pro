include(../guh.pri)

message("Building guh version $${GUH_VERSION_STRING}")

TARGET = guhd
TEMPLATE = app

INCLUDEPATH += ../libguh jsonrpc

target.path = /usr/bin
INSTALLS += target

QT += network

LIBS += -L$$top_builddir/libguh/ -lguh

include(server.pri)
SOURCES += main.cpp

boblight {
    xcompile {
        LIBS += -L../plugins/deviceplugins/boblight -lguh_devicepluginboblight -lboblight
    } else {
        LIBS += -L../plugins/deviceplugins/boblight -lguh_devicepluginboblight -L/usr/local/lib/ -lboblight
    }
    DEFINES += USE_BOBLIGHT
}
