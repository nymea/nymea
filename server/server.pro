TARGET = hive
TEMPLATE = app

INCLUDEPATH += ../libhive

target.path = /usr/bin
INSTALLS += target

QT += network

LIBS += -L../libhive/ -lhive -L../plugins/deviceplugins/rfswitch/ -lhive_rfswitch

SOURCES += main.cpp \
           hivecore.cpp \
           jsonrpcserver.cpp \
           tcpserver.cpp \

HEADERS += hivecore.h \
           jsonrpcserver.h \
           tcpserver.h \

# FIXME: Drop this and link them dynamically
