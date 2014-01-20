TARGET = hive
TEMPLATE = app

INCLUDEPATH += ../libhive jsonrpc

target.path = /usr/bin
INSTALLS += target

QT += network
CONFIG += c++11

LIBS += -L../libhive/ -lhive

SOURCES += main.cpp \
    hivecore.cpp \
    tcpserver.cpp \
    ruleengine.cpp \
    rule.cpp \
    jsonrpc/jsonrpcserver.cpp \
    jsonrpc/jsonhandler.cpp \
    jsonrpc/devicehandler.cpp \
    jsonrpc/jsontypes.cpp \
    jsonrpc/ruleshandler.cpp \
    jsonrpc/actionhandler.cpp

HEADERS += hivecore.h \
    tcpserver.h \
    ruleengine.h \
    rule.h \
    jsonrpc/jsonrpcserver.h \
    jsonrpc/jsonhandler.h \
    jsonrpc/devicehandler.h \
    jsonrpc/jsontypes.h \
    jsonrpc/ruleshandler.h \
    jsonrpc/actionhandler.h

# FIXME: Drop this and link them dynamically
LIBS += -L../plugins/deviceplugins/elro/ -lhive_devicepluginelro
LIBS += -L../plugins/deviceplugins/intertechno/ -lhive_devicepluginintertechno
LIBS += -L../plugins/deviceplugins/meisteranker/ -lhive_devicepluginmeisteranker
LIBS += -L../plugins/deviceplugins/wifidetector/ -lhive_devicepluginwifidetector
