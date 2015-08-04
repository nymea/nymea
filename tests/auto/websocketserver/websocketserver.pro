include(../../../guh.pri)
include(../autotests.pri)

TARGET = websocketserver

contains(DEFINES, WEBSOCKET){
    QT += websockets
    SOURCES += testwebsocketserver.cpp
}
