TEMPLATE = lib
TARGET = nymea-core

include(../nymea.pri)

QT += sql
INCLUDEPATH += $$top_srcdir/libnymea
LIBS += -L$$top_builddir/libnymea/ -lnymea -lssl -lcrypto -lnymea-mqtt

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

# icons for the webserver
RESOURCES += $$top_srcdir/icons.qrc \
             $$top_srcdir/data/debug-interface/debug-interface.qrc


HEADERS += nymeacore.h \
    ruleengine.h \
    rule.h \
    stateevaluator.h \
    transportinterface.h \
    nymeaconfiguration.h \
    servermanager.h \
    servers/tcpserver.h \
    servers/mocktcpserver.h \
    servers/webserver.h \
    servers/httprequest.h \
    servers/httpreply.h \
    servers/bluetoothserver.h \
    servers/rest/restserver.h \
    servers/rest/restresource.h \
    servers/rest/devicesresource.h \
    servers/rest/deviceclassesresource.h \
    servers/rest/vendorsresource.h \
    servers/rest/logsresource.h \
    servers/rest/pluginsresource.h \
    servers/rest/rulesresource.h \
    servers/websocketserver.h \
    servers/mqttbroker.h \
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
    cloud/awsconnector.h \
    cloud/cloudmanager.h \
    cloud/cloudnotifications.h \
    pushbuttondbusservice.h \
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
    hardware/network/mqtt/mqttproviderimplementation.h \
    hardware/network/mqtt/mqttchannelimplementation.h \
    debugserverhandler.h \
    tagging/tagsstorage.h \
    tagging/tag.h \
    jsonrpc/tagshandler.h \
    cloud/cloudtransport.h \
    debugreportgenerator.h \
    platform/platform.h \
    jsonrpc/systemhandler.h

SOURCES += nymeacore.cpp \
    ruleengine.cpp \
    rule.cpp \
    stateevaluator.cpp \
    transportinterface.cpp \
    nymeaconfiguration.cpp \
    servermanager.cpp \
    servers/tcpserver.cpp \
    servers/mocktcpserver.cpp \
    servers/webserver.cpp \
    servers/httprequest.cpp \
    servers/httpreply.cpp \
    servers/websocketserver.cpp \
    servers/bluetoothserver.cpp \
    servers/rest/restserver.cpp \
    servers/rest/restresource.cpp \
    servers/rest/devicesresource.cpp \
    servers/rest/deviceclassesresource.cpp \
    servers/rest/vendorsresource.cpp \
    servers/rest/logsresource.cpp \
    servers/rest/pluginsresource.cpp \
    servers/rest/rulesresource.cpp \
    servers/mqttbroker.cpp \
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
    cloud/awsconnector.cpp \
    cloud/cloudmanager.cpp \
    cloud/cloudnotifications.cpp \
    pushbuttondbusservice.cpp \
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
    hardware/network/mqtt/mqttproviderimplementation.cpp \
    hardware/network/mqtt/mqttchannelimplementation.cpp \
    debugserverhandler.cpp \
    tagging/tagsstorage.cpp \
    tagging/tag.cpp \
    jsonrpc/tagshandler.cpp \
    cloud/cloudtransport.cpp \
    debugreportgenerator.cpp \
    platform/platform.cpp \
    jsonrpc/systemhandler.cpp
