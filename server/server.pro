TARGET = hive
TEMPLATE = app

INCLUDEPATH += ../libhive

target.path = /usr/bin
INSTALLS += target

QT += network

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
LIBS += -L../plugins/deviceplugins/rfremotemumbi/ -lhive_rfremotemumbi
LIBS += -L../plugins/deviceplugins/rfremoteintertechno/ -lhive_rfremoteintertechno
