TEMPLATE = lib
TARGET = guh-core

include(../guh.pri)

QT += sql
INCLUDEPATH += $$top_srcdir/libguh $$top_srcdir jsonrpc
LIBS += -L$$top_builddir/libguh/ -lguh -lssl -lcrypto -lavahi-common -lavahi-client

target.path = /usr/lib/$$system('dpkg-architecture -q DEB_HOST_MULTIARCH')
INSTALLS += target

# icons for the webserver
RESOURCES += $$top_srcdir/icons.qrc \
             $$top_srcdir/data/debug-interface/debug-interface.qrc


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
    cloudnotifications.h \
    OpenSSL/OpenSSLConnection.hpp \
    janusconnector.h \
    pushbuttondbusservice.h \
    guhdbusservice.h \
    hardwaremanagerimplementation.h \
    hardware/plugintimermanagerimplementation.h \
    hardware/radio433/radio433brennenstuhl.h \
    hardware/radio433/radio433transmitter.h \
    hardware/radio433/radio433brennenstuhlgateway.h \
    hardware/bluetoothlowenergy/bluetoothlowenergymanagerimplementation.h \
    hardware/bluetoothlowenergy/bluetoothlowenergydeviceimplementation.h \
    hardware/bluetoothlowenergy/bluetoothdiscoveryreplyimplementation.h \
    hardware/network/networkaccessmanagerimpl.h \
    hardware/network/upnp/upnpdiscoveryimplementation.h \
    hardware/network/upnp/upnpdiscoveryrequest.h \
    hardware/network/upnp/upnpdiscoveryreplyimplementation.h \
    hardware/network/avahi/qt-watch.h \
    hardware/network/avahi/qtavahiclient.h \
    hardware/network/avahi/qtavahiservice.h \
    hardware/network/avahi/qtavahiservice_p.h \
    hardware/network/avahi/qtavahiservicebrowserimplementation.h \
    hardware/network/avahi/qtavahiservicebrowserimplementation_p.h \
    debugserverhandler.h

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
    cloudnotifications.cpp \
    OpenSSL/OpenSSLConnection.cpp \
    janusconnector.cpp \
    pushbuttondbusservice.cpp \
    guhdbusservice.cpp \
    hardwaremanagerimplementation.cpp \
    hardware/plugintimermanagerimplementation.cpp \
    hardware/radio433/radio433brennenstuhl.cpp \
    hardware/radio433/radio433transmitter.cpp \
    hardware/radio433/radio433brennenstuhlgateway.cpp \
    hardware/bluetoothlowenergy/bluetoothlowenergymanagerimplementation.cpp \
    hardware/bluetoothlowenergy/bluetoothlowenergydeviceimplementation.cpp \
    hardware/bluetoothlowenergy/bluetoothdiscoveryreplyimplementation.cpp \
    hardware/network/networkaccessmanagerimpl.cpp \
    hardware/network/upnp/upnpdiscoveryimplementation.cpp \
    hardware/network/upnp/upnpdiscoveryrequest.cpp \
    hardware/network/upnp/upnpdiscoveryreplyimplementation.cpp \
    hardware/network/avahi/qt-watch.cpp \
    hardware/network/avahi/qtavahiclient.cpp \
    hardware/network/avahi/qtavahiservice.cpp \
    hardware/network/avahi/qtavahiservice_p.cpp \
    hardware/network/avahi/qtavahiservicebrowserimplementation.cpp \
    hardware/network/avahi/qtavahiservicebrowserimplementation_p.cpp \
    debugserverhandler.cpp
