TARGET = hive
TEMPLATE = app

INCLUDEPATH += ../libhive

target.path = /usr/bin
INSTALLS += target

QT += network
CONFIG += c++11

LIBS += -L../libhive/ -lhive

SOURCES += main.cpp \
           hivecore.cpp \
           jsonrpcserver.cpp \
           tcpserver.cpp \
           ruleengine.cpp \
           rule.cpp

HEADERS += hivecore.h \
           jsonrpcserver.h \
           tcpserver.h \
           ruleengine.h \
           rule.h

# FIXME: Drop this and link them dynamically
LIBS += -L../plugins/deviceplugins/devicepluginelro/ -lhive_devicepluginelro
LIBS += -L../plugins/deviceplugins/devicepluginintertechno/ -lhive_devicepluginintertechno
LIBS += -L../plugins/deviceplugins/devicepluginmeisteranker/ -lhive_devicepluginmeisteranker
