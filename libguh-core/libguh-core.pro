TEMPLATE = lib
TARGET = guh-core

include(../guh.pri)

QT += sql
INCLUDEPATH += $$top_srcdir/libguh jsonrpc
LIBS += -L$$top_builddir/libguh/ -lguh -lssl -lcrypto

target.path = /usr/lib/$$system('dpkg-architecture -q DEB_HOST_MULTIARCH')
INSTALLS += target

exists("/usr/include/mbedtls/net_sockets.h") {
    DEFINES += MBEDTLS_NEW_HEADERS
}

# icons for the webserver
RESOURCES += $$top_srcdir/icons.qrc

HEADERS += guhcore.h \
    tcpserver.h \
    mocktcpserver.h \
    ruleengine.h \
    rule.h \
    stateevaluator.h \
    webserver.h \
    transportinterface.h \
    servermanager.h \
    httprequest.h \
    websocketserver.h \
    httpreply.h \
    guhconfiguration.h \
    bluetoothserver.h \
    jsonrpc/jsonrpcserver.h \
    jsonrpc/jsonhandler.h \
    jsonrpc/devicehandler.h \
    jsonrpc/jsontypes.h \
    jsonrpc/ruleshandler.h \
    jsonrpc/actionhandler.h \
    jsonrpc/eventhandler.h \
    jsonrpc/statehandler.h \
    jsonrpc/logginghandler.h \
    jsonrpc/configurationhandler.h \
    jsonrpc/networkmanagerhandler.h \
    logging/logging.h \
    logging/logengine.h \
    logging/logfilter.h \
    logging/logentry.h \
    logging/logvaluetool.h \
    rest/restserver.h \
    rest/restresource.h \
    rest/devicesresource.h \
    rest/deviceclassesresource.h \
    rest/vendorsresource.h \
    rest/logsresource.h \
    rest/pluginsresource.h \
    rest/rulesresource.h \
    time/timedescriptor.h \
    time/calendaritem.h \
    time/repeatingoption.h \
    time/timeeventitem.h \
    time/timemanager.h \
    networkmanager/dbus-interfaces.h \
    networkmanager/networkmanager.h \
    networkmanager/networkdevice.h \
    networkmanager/wirelessaccesspoint.h \
    networkmanager/wirelessnetworkdevice.h \
    networkmanager/networksettings.h \
    networkmanager/networkconnection.h \
    networkmanager/wirednetworkdevice.h \
    usermanager.h \
    tokeninfo.h \
    certificategenerator.h \
    awsconnector.h \
    cloudmanager.h \
    MbedTLS/MbedTLSConnection.hpp \
    janusconnector.h \

SOURCES += guhcore.cpp \
    tcpserver.cpp \
    mocktcpserver.cpp \
    ruleengine.cpp \
    rule.cpp \
    stateevaluator.cpp \
    webserver.cpp \
    transportinterface.cpp \
    servermanager.cpp \
    httprequest.cpp \
    websocketserver.cpp \
    httpreply.cpp \
    guhconfiguration.cpp \
    bluetoothserver.cpp \
    jsonrpc/jsonrpcserver.cpp \
    jsonrpc/jsonhandler.cpp \
    jsonrpc/devicehandler.cpp \
    jsonrpc/jsontypes.cpp \
    jsonrpc/ruleshandler.cpp \
    jsonrpc/actionhandler.cpp \
    jsonrpc/eventhandler.cpp \
    jsonrpc/statehandler.cpp \
    jsonrpc/logginghandler.cpp \
    jsonrpc/configurationhandler.cpp \
    jsonrpc/networkmanagerhandler.cpp \
    logging/logengine.cpp \
    logging/logfilter.cpp \
    logging/logentry.cpp \
    logging/logvaluetool.cpp \
    rest/restserver.cpp \
    rest/restresource.cpp \
    rest/devicesresource.cpp \
    rest/deviceclassesresource.cpp \
    rest/vendorsresource.cpp \
    rest/logsresource.cpp \
    rest/pluginsresource.cpp \
    rest/rulesresource.cpp \
    time/timedescriptor.cpp \
    time/calendaritem.cpp \
    time/repeatingoption.cpp \
    time/timeeventitem.cpp \
    time/timemanager.cpp \
    networkmanager/networkmanager.cpp \
    networkmanager/networkdevice.cpp \
    networkmanager/wirelessaccesspoint.cpp \
    networkmanager/wirelessnetworkdevice.cpp \
    networkmanager/networksettings.cpp \
    networkmanager/networkconnection.cpp \
    networkmanager/wirednetworkdevice.cpp \
    usermanager.cpp \
    tokeninfo.cpp \
    certificategenerator.cpp \
    awsconnector.cpp \
    cloudmanager.cpp \
    MbedTLS/MbedTLSConnection.cpp \
    janusconnector.cpp \
