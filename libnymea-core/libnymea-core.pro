TEMPLATE = lib
TARGET = nymea-core

include(../nymea.pri)

QT += core bluetooth dbus qml sql websockets serialport
INCLUDEPATH += $$top_srcdir/libnymea $$top_builddir
LIBS += -L$$top_builddir/libnymea/ -lnymea -lssl -lcrypto

CONFIG += link_pkgconfig
PKGCONFIG += nymea-mqtt nymea-networkmanager nymea-zigbee nymea-remoteproxyclient nymea-gpio

packagesExist(systemd) {
    message(Building with systemd support)
    PKGCONFIG += systemd
    DEFINES += WITH_SYSTEMD
} else {
    message(Building without systemd support)
}

greaterThan(QT_MAJOR_VERSION, 5) {
    qtHaveModule(serialbus) {
        message("Building with QtSerialBus support.")
        QT += serialbus
        DEFINES += WITH_QTSERIALBUS
    } else {
        message("QtSerialBus package not found. Building without QtSerialBus support.")
    }

    DEFINES += ZIGBEE_DISABLE_TI

    # Separate module in Qt6
    QT += concurrent
} else {
    packagesExist(Qt5SerialBus) {
        message("Building with QtSerialBus support.")
        PKGCONFIG += Qt5SerialBus
        DEFINES += WITH_QTSERIALBUS
    } else {
        message("Qt5SerialBus package not found. Building without QtSerialBus support.")
    }

    contains(DEFINES, ZIGBEE_DISABLE_TI) {
        message(Build without zigbee TI backend support)
    }
}

# Note: udev is not available on all platforms
packagesExist(libudev) {
    message("Building with udev support")
    PKGCONFIG += libudev
    DEFINES += WITH_UDEV
} else {
    message("Building without udev support.")
}


# icons for the webserver
RESOURCES += $$top_srcdir/icons.qrc \
             $$top_srcdir/data/debug-interface/debug-interface.qrc

HEADERS += nymeacore.h \
    hardware/bluetoothlowenergy/bluetoothpairingjobimplementation.h \
    hardware/bluetoothlowenergy/nymeabluetoothagent.h \
    hardware/network/macaddressdatabasereplyimpl.h \
    hardware/serialport/serialportmonitor.h \
    hardware/zwave/zwavehardwareresourceimplementation.h \
    jsonrpc/debughandler.h \
    logging/logengineinfluxdb.h \
    scriptengine/scriptthing.h \
    scriptengine/scriptthings.h \
    zwave/zwavedevicedatabase.h \
    zwave/zwavemanagerreply.h \
    zwave/zwavenodeimplementation.h \
    integrations/apikeysprovidersloader.h \
    integrations/plugininfocache.h \
    integrations/python/pyapikeystorage.h \
    integrations/python/pybrowseractioninfo.h \
    integrations/python/pybrowseresult.h \
    integrations/python/pybrowseritem.h \
    integrations/python/pybrowseritemresult.h \
    integrations/python/pypluginstorage.h \
    integrations/python/pyplugintimer.h \
    integrations/thingmanagerimplementation.h \
    integrations/translator.h \
    experiences/experiencemanager.h \
    jsonrpc/modbusrtuhandler.h \
    jsonrpc/zigbeehandler.h \
    jsonrpc/zwavehandler.h \
    ruleengine/ruleengine.h \
    ruleengine/rule.h \
    ruleengine/stateevaluator.h \
    ruleengine/ruleaction.h \
    ruleengine/ruleactionparam.h \
    scriptengine/nymeascript.h \
    scriptengine/script.h \
    scriptengine/scriptaction.h \
    scriptengine/scriptalarm.h \
    scriptengine/scriptengine.h \
    scriptengine/scriptevent.h \
    scriptengine/scriptinterfaceaction.h \
    scriptengine/scriptinterfaceevent.h \
    scriptengine/scriptinterfacestate.h \
    scriptengine/scriptstate.h \
    transportinterface.h \
    nymeaconfiguration.h \
    servermanager.h \
    servers/tcpserver.h \
    servers/mocktcpserver.h \
    servers/webserver.h \
    servers/bluetoothserver.h \
    servers/websocketserver.h \
    servers/mqttbroker.h \
    servers/tunnelproxyserver.h \
    jsonrpc/jsonrpcserverimplementation.h \
    jsonrpc/jsonvalidator.h \
    jsonrpc/integrationshandler.h \
    jsonrpc/ruleshandler.h \
    jsonrpc/logginghandler.h \
    jsonrpc/configurationhandler.h \
    jsonrpc/networkmanagerhandler.h \
    jsonrpc/tagshandler.h \
    jsonrpc/appdatahandler.h \
    jsonrpc/systemhandler.h \
    jsonrpc/scriptshandler.h \
    jsonrpc/usershandler.h \
    time/timemanager.h \
    usermanager/userautorizer.h \
    usermanager/userinfo.h \
    usermanager/usermanager.h \
    usermanager/tokeninfo.h \
    usermanager/pushbuttondbusservice.h \
    certificategenerator.h \
    hardwaremanagerimplementation.h \
    hardware/plugintimermanagerimplementation.h \
    hardware/radio433/radio433brennenstuhl.h \
    hardware/radio433/radio433transmitter.h \
    hardware/radio433/radio433brennenstuhlgateway.h \
    hardware/bluetoothlowenergy/bluetoothlowenergymanagerimplementation.h \
    hardware/bluetoothlowenergy/bluetoothlowenergydeviceimplementation.h \
    hardware/bluetoothlowenergy/bluetoothdiscoveryreplyimplementation.h \
    hardware/modbus/modbusrtuhardwareresourceimplementation.h \
    hardware/modbus/modbusrtumanager.h \
    hardware/modbus/modbusrtumasterimpl.h \
    hardware/modbus/modbusrtureplyimpl.h \
    hardware/network/macaddressdatabase.h \
    hardware/network/networkaccessmanagerimpl.h \
    hardware/network/networkdevicediscoveryimpl.h \
    hardware/network/networkdevicediscoveryreplyimpl.h \
    hardware/network/networkdevicemonitorimpl.h \
    hardware/network/upnp/upnpdiscoveryimplementation.h \
    hardware/network/upnp/upnpdiscoveryrequest.h \
    hardware/network/upnp/upnpdiscoveryreplyimplementation.h \
    hardware/network/mqtt/mqttproviderimplementation.h \
    hardware/network/mqtt/mqttchannelimplementation.h \
    hardware/i2c/i2cmanagerimplementation.h \
    hardware/zigbee/zigbeehardwareresourceimplementation.h \
    debugserverhandler.h \
    tagging/tagsstorage.h \
    tagging/tag.h \
    debugreportgenerator.h \
    platform/platform.h \
    zigbee/zigbeeadapter.h \
    zigbee/zigbeeadapters.h \
    zigbee/zigbeemanager.h \
    zwave/zwaveadapter.h \
    zwave/zwavemanager.h \
    zwave/zwavenetwork.h \


SOURCES += nymeacore.cpp \
    hardware/bluetoothlowenergy/bluetoothpairingjobimplementation.cpp \
    hardware/bluetoothlowenergy/nymeabluetoothagent.cpp \
    hardware/network/macaddressdatabasereplyimpl.cpp \
    hardware/serialport/serialportmonitor.cpp \
    hardware/zwave/zwavehardwareresourceimplementation.cpp \
    jsonrpc/debughandler.cpp \
    logging/logengineinfluxdb.cpp \
    scriptengine/scriptthing.cpp \
    scriptengine/scriptthings.cpp \
    zwave/zwavedevicedatabase.cpp \
    zwave/zwavemanagerreply.cpp \
    zwave/zwavenodeimplementation.cpp \
    integrations/apikeysprovidersloader.cpp \
    integrations/plugininfocache.cpp \
    integrations/thingmanagerimplementation.cpp \
    integrations/translator.cpp \
    experiences/experiencemanager.cpp \
    jsonrpc/modbusrtuhandler.cpp \
    jsonrpc/zigbeehandler.cpp \
    jsonrpc/zwavehandler.cpp \
    ruleengine/ruleengine.cpp \
    ruleengine/rule.cpp \
    ruleengine/stateevaluator.cpp \
    ruleengine/ruleaction.cpp \
    ruleengine/ruleactionparam.cpp \
    scriptengine/nymeascript.cpp \
    scriptengine/script.cpp \
    scriptengine/scriptaction.cpp \
    scriptengine/scriptalarm.cpp \
    scriptengine/scriptengine.cpp \
    scriptengine/scriptevent.cpp \
    scriptengine/scriptinterfaceaction.cpp \
    scriptengine/scriptinterfaceevent.cpp \
    scriptengine/scriptinterfacestate.cpp \
    scriptengine/scriptstate.cpp \
    transportinterface.cpp \
    nymeaconfiguration.cpp \
    servermanager.cpp \
    servers/tcpserver.cpp \
    servers/mocktcpserver.cpp \
    servers/webserver.cpp \
    servers/websocketserver.cpp \
    servers/bluetoothserver.cpp \
    servers/mqttbroker.cpp \
    servers/tunnelproxyserver.cpp \
    jsonrpc/jsonrpcserverimplementation.cpp \
    jsonrpc/jsonvalidator.cpp \
    jsonrpc/integrationshandler.cpp \
    jsonrpc/ruleshandler.cpp \
    jsonrpc/logginghandler.cpp \
    jsonrpc/configurationhandler.cpp \
    jsonrpc/networkmanagerhandler.cpp \
    jsonrpc/tagshandler.cpp \
    jsonrpc/appdatahandler.cpp \
    jsonrpc/systemhandler.cpp \
    jsonrpc/scriptshandler.cpp \
    jsonrpc/usershandler.cpp \
    time/timemanager.cpp \
    usermanager/userautorizer.cpp \
    usermanager/userinfo.cpp \
    usermanager/usermanager.cpp \
    usermanager/tokeninfo.cpp \
    usermanager/pushbuttondbusservice.cpp \
    certificategenerator.cpp \
    hardwaremanagerimplementation.cpp \
    hardware/plugintimermanagerimplementation.cpp \
    hardware/radio433/radio433brennenstuhl.cpp \
    hardware/radio433/radio433transmitter.cpp \
    hardware/radio433/radio433brennenstuhlgateway.cpp \
    hardware/bluetoothlowenergy/bluetoothlowenergymanagerimplementation.cpp \
    hardware/bluetoothlowenergy/bluetoothlowenergydeviceimplementation.cpp \
    hardware/bluetoothlowenergy/bluetoothdiscoveryreplyimplementation.cpp \
    hardware/modbus/modbusrtuhardwareresourceimplementation.cpp \
    hardware/modbus/modbusrtumanager.cpp \
    hardware/modbus/modbusrtumasterimpl.cpp \
    hardware/modbus/modbusrtureplyimpl.cpp \
    hardware/network/macaddressdatabase.cpp \
    hardware/network/networkaccessmanagerimpl.cpp \
    hardware/network/networkdevicediscoveryimpl.cpp \
    hardware/network/networkdevicediscoveryreplyimpl.cpp \
    hardware/network/networkdevicemonitorimpl.cpp \
    hardware/network/upnp/upnpdiscoveryimplementation.cpp \
    hardware/network/upnp/upnpdiscoveryrequest.cpp \
    hardware/network/upnp/upnpdiscoveryreplyimplementation.cpp \
    hardware/network/mqtt/mqttproviderimplementation.cpp \
    hardware/network/mqtt/mqttchannelimplementation.cpp \
    hardware/i2c/i2cmanagerimplementation.cpp \
    hardware/zigbee/zigbeehardwareresourceimplementation.cpp \
    debugserverhandler.cpp \
    tagging/tagsstorage.cpp \
    tagging/tag.cpp \
    debugreportgenerator.cpp \
    platform/platform.cpp \
    zigbee/zigbeeadapter.cpp \
    zigbee/zigbeeadapters.cpp \
    zigbee/zigbeemanager.cpp \
    zwave/zwaveadapter.cpp \
    zwave/zwavemanager.cpp \
    zwave/zwavenetwork.cpp \

versionAtLeast(QT_VERSION, 5.12.0) {
message("Building with JS plugin support")
HEADERS += \
    integrations/scriptintegrationplugin.h

SOURCES += \
    integrations/scriptintegrationplugin.cpp
}

CONFIG(python) {
message("Building with Python plugin support")
DEFINES += WITH_PYTHON
HEADERS += \
    integrations/pythonintegrationplugin.h \
    integrations/python/pynymealogginghandler.h \
    integrations/python/pynymeamodule.h \
    integrations/python/pyparam.h \
    integrations/python/pystdouthandler.h \
    integrations/python/pything.h \
    integrations/python/pythingactioninfo.h \
    integrations/python/pythingdescriptor.h \
    integrations/python/pythingdiscoveryinfo.h \
    integrations/python/pythingpairinginfo.h \
    integrations/python/pythingsetupinfo.h \
    integrations/python/pyutils.h

SOURCES += \
    integrations/pythonintegrationplugin.cpp
}

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

# install header file with relative subdirectory
for(header, HEADERS) {
    path = $$[QT_INSTALL_PREFIX]/include/nymea-core/$${dirname(header)}
    eval(headers_$${path}.files += $${header})
    eval(headers_$${path}.path = $${path})
    eval(INSTALLS *= headers_$${path})
}

# Create pkgconfig file
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_NAME = libnymea-core
QMAKE_PKGCONFIG_DESCRIPTION = nymea core development library
QMAKE_PKGCONFIG_PREFIX = $$[QT_INSTALL_PREFIX]
QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_PREFIX]/include/nymea-core/
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_VERSION = $$NYMEA_VERSION_STRING
QMAKE_PKGCONFIG_FILE = nymea-core
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
