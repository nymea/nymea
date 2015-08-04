include(../../../guh.pri)
include(../autotests.pri)

contains(DEFINES, WEBSOCKET){
    QT += websockets
}

TARGET = websocketserver
SOURCES += testwebsocketserver.cpp

