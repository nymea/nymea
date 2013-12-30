include(../common.pri)

TARGET = hive
TEMPLATE = app

INCLUDEPATH += ../libhive

QT += network

LIBS += -L../libhive/ -lhive

SOURCES += main.cpp \
           hivecore.cpp \
           jsonrpcserver.cpp \
           radio433.cpp \
           tcpserver.cpp

HEADERS += hivecore.h \
           jsonrpcserver.h \
           radio433.h \
           tcpserver.h
