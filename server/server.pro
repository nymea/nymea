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
           radio433.cpp \
           tcpserver.cpp \
           gpio.cpp \
           deviceplugins/rfswitch/rfswitch.cpp \
           devicemanager.cpp \

HEADERS += hivecore.h \
           jsonrpcserver.h \
           radio433.h \
           tcpserver.h \
           gpio.h \
           deviceplugins/rfswitch/rfswitch.h \
           devicemanager.h \
