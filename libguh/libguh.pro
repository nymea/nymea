include(../guh.pri)

TARGET = guh
TEMPLATE = lib

QT += network
DEFINES += LIBGUH_LIBRARY

QMAKE_LFLAGS += -fPIC

target.path = /usr/lib
INSTALLS += target

# check Bluetooth LE support
contains(DEFINES, BLUETOOTH_LE) {
    SOURCES += bluetooth/bluetoothscanner.cpp \
               bluetooth/bluetoothlowenergydevice.cpp \

    HEADERS += bluetooth/bluetoothscanner.h \
               bluetooth/bluetoothlowenergydevice.h \
}

SOURCES += devicemanager.cpp \
           loggingcategories.cpp \
           guhsettings.cpp \
           plugin/device.cpp \
           plugin/deviceclass.cpp \
           plugin/deviceplugin.cpp \
           plugin/devicedescriptor.cpp \
           hardware/gpio.cpp \
           hardware/gpiomonitor.cpp \
           hardware/radio433/radio433.cpp \
           hardware/radio433/radio433transmitter.cpp \
           hardware/radio433/radio433receiver.cpp \
           hardware/radio433/radio433brennenstuhlgateway.cpp \
           network/upnpdiscovery/upnpdiscovery.cpp \
           network/upnpdiscovery/upnpdevice.cpp \
           network/upnpdiscovery/upnpdevicedescriptor.cpp \
           network/upnpdiscovery/upnpdiscoveryrequest.cpp \
           network/networkmanager.cpp \
           network/oauth2.cpp \
           coap/coap.cpp \
           coap/coappdu.cpp \
           coap/coapoption.cpp \
           coap/coaprequest.cpp \
           coap/coapreply.cpp \
           coap/coappdublock.cpp \
           coap/corelinkparser.cpp \
           coap/corelink.cpp \
           coap/coapobserveresource.cpp \
           types/action.cpp \
           types/actiontype.cpp \
           types/state.cpp \
           types/statetype.cpp \
           types/eventtype.cpp \
           types/event.cpp \
           types/eventdescriptor.cpp \
           types/vendor.cpp \
           types/paramtype.cpp \
           types/param.cpp \
           types/paramdescriptor.cpp \
           types/ruleaction.cpp \
           types/ruleactionparam.cpp \
           types/statedescriptor.cpp \

HEADERS += devicemanager.h \
           libguh.h \
           typeutils.h \
           loggingcategories.h \
           guhsettings.h \
           plugin/device.h \
           plugin/deviceclass.h \
           plugin/deviceplugin.h \
           plugin/devicedescriptor.h \
           hardware/gpio.h \
           hardware/gpiomonitor.h \
           hardware/radio433/radio433.h \
           hardware/radio433/radio433transmitter.h \
           hardware/radio433/radio433receiver.h \
           hardware/radio433/radio433brennenstuhlgateway.h \
           network/upnpdiscovery/upnpdiscovery.h \
           network/upnpdiscovery/upnpdevice.h \
           network/upnpdiscovery/upnpdevicedescriptor.h \
           network/upnpdiscovery/upnpdiscoveryrequest.h \
           network/networkmanager.h \
           network/oauth2.h \
           coap/coap.h \
           coap/coappdu.h \
           coap/coapoption.h \
           coap/coaprequest.h \
           coap/coapreply.h \
           coap/coappdublock.h \
           coap/corelinkparser.h \
           coap/corelink.h \
           coap/coapobserveresource.h \
           types/action.h \
           types/actiontype.h \
           types/state.h \
           types/statetype.h \
           types/eventtype.h \
           types/event.h \
           types/eventdescriptor.h \
           types/vendor.h \
           types/paramtype.h \
           types/param.h \
           types/paramdescriptor.h \
           types/ruleaction.h \
           types/ruleactionparam.h \
           types/statedescriptor.h \

# install plugininfo python script for libguh-dev
generateplugininfo.files = $$top_srcdir/plugins/guh-generateplugininfo
generateplugininfo.path = /usr/bin

INSTALLS +=  generateplugininfo

# install header file with relative subdirectory
for(header, HEADERS) {
    path = /usr/include/guh/$${dirname(header)}
    eval(headers_$${path}.files += $${header})
    eval(headers_$${path}.path = $${path})
    eval(INSTALLS *= headers_$${path})
}




