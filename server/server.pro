TARGET = hive
TEMPLATE = app

INCLUDEPATH += ../libhive jsonrpc

target.path = /usr/bin
INSTALLS += target

QT += network
CONFIG += c++11

LIBS += -L$$top_builddir/libhive/ -lhive

include(server.pri)
SOURCES += main.cpp

# FIXME: Drop this and link them dynamically
LIBS += -L../plugins/deviceplugins/elro/ -lhive_devicepluginelro
LIBS += -L../plugins/deviceplugins/intertechno/ -lhive_devicepluginintertechno
LIBS += -L../plugins/deviceplugins/meisteranker/ -lhive_devicepluginmeisteranker
LIBS += -L../plugins/deviceplugins/wifidetector/ -lhive_devicepluginwifidetector
