TEMPLATE = lib
TARGET = guh-core

include(../guh.pri)

QT += sql
INCLUDEPATH += $$top_srcdir/libguh jsonrpc
LIBS += -L$$top_builddir/libguh/ -lguh -lssl -lcrypto

target.path = /usr/lib/$$system('dpkg-architecture -q DEB_HOST_MULTIARCH')
INSTALLS += target

# icons for the webserver
RESOURCES += $$top_srcdir/icons.qrc

HEADERS += $$top_srcdir/libguh-core/guhcore.h \
    $$top_srcdir/libguh-core/tcpserver.h \
    $$top_srcdir/libguh-core/mocktcpserver.h \
    $$top_srcdir/libguh-core/ruleengine.h \
    $$top_srcdir/libguh-core/rule.h \
    $$top_srcdir/libguh-core/stateevaluator.h \
    $$top_srcdir/libguh-core/webserver.h \
    $$top_srcdir/libguh-core/transportinterface.h \
    $$top_srcdir/libguh-core/servermanager.h \
    $$top_srcdir/libguh-core/httprequest.h \
    $$top_srcdir/libguh-core/websocketserver.h \
    $$top_srcdir/libguh-core/httpreply.h \
    $$top_srcdir/libguh-core/guhconfiguration.h \
    $$top_srcdir/libguh-core/bluetoothserver.h \
    $$top_srcdir/libguh-core/jsonrpc/jsonrpcserver.h \
    $$top_srcdir/libguh-core/jsonrpc/jsonhandler.h \
    $$top_srcdir/libguh-core/jsonrpc/devicehandler.h \
    $$top_srcdir/libguh-core/jsonrpc/jsontypes.h \
    $$top_srcdir/libguh-core/jsonrpc/ruleshandler.h \
    $$top_srcdir/libguh-core/jsonrpc/actionhandler.h \
    $$top_srcdir/libguh-core/jsonrpc/eventhandler.h \
    $$top_srcdir/libguh-core/jsonrpc/statehandler.h \
    $$top_srcdir/libguh-core/jsonrpc/logginghandler.h \
    $$top_srcdir/libguh-core/jsonrpc/configurationhandler.h \
    $$top_srcdir/libguh-core/jsonrpc/networkmanagerhandler.h \
    $$top_srcdir/libguh-core/logging/logging.h \
    $$top_srcdir/libguh-core/logging/logengine.h \
    $$top_srcdir/libguh-core/logging/logfilter.h \
    $$top_srcdir/libguh-core/logging/logentry.h \
    $$top_srcdir/libguh-core/rest/restserver.h \
    $$top_srcdir/libguh-core/rest/restresource.h \
    $$top_srcdir/libguh-core/rest/devicesresource.h \
    $$top_srcdir/libguh-core/rest/deviceclassesresource.h \
    $$top_srcdir/libguh-core/rest/vendorsresource.h \
    $$top_srcdir/libguh-core/rest/logsresource.h \
    $$top_srcdir/libguh-core/rest/pluginsresource.h \
    $$top_srcdir/libguh-core/rest/rulesresource.h \
    $$top_srcdir/libguh-core/time/timedescriptor.h \
    $$top_srcdir/libguh-core/time/calendaritem.h \
    $$top_srcdir/libguh-core/time/repeatingoption.h \
    $$top_srcdir/libguh-core/time/timeeventitem.h \
    $$top_srcdir/libguh-core/time/timemanager.h \
    $$top_srcdir/libguh-core/networkmanager/dbus-interfaces.h \
    $$top_srcdir/libguh-core/networkmanager/networkmanager.h \
    $$top_srcdir/libguh-core/networkmanager/networkdevice.h \
    $$top_srcdir/libguh-core/networkmanager/wirelessaccesspoint.h \
    $$top_srcdir/libguh-core/networkmanager/wirelessnetworkdevice.h \
    $$top_srcdir/libguh-core/networkmanager/networksettings.h \
    $$top_srcdir/libguh-core/networkmanager/networkconnection.h \
    $$top_srcdir/libguh-core/networkmanager/wirednetworkdevice.h \
    $$top_srcdir/libguh-core/usermanager.h \
    $$top_srcdir/libguh-core/tokeninfo.h \
    $$top_srcdir/libguh-core/certificategenerator.h \
    $$top_srcdir/libguh-core/logging/logvaluetool.h
    $$top_srcdir/libguh-core/awsconnector.h \
    $$top_srcdir/libguh-core/cloudconnector.h \
    $$top_srcdir/libguh-core/MbedTLS/MbedTLSConnection.hpp \


SOURCES += $$top_srcdir/libguh-core/guhcore.cpp \
    $$top_srcdir/libguh-core/tcpserver.cpp \
    $$top_srcdir/libguh-core/mocktcpserver.cpp \
    $$top_srcdir/libguh-core/ruleengine.cpp \
    $$top_srcdir/libguh-core/rule.cpp \
    $$top_srcdir/libguh-core/stateevaluator.cpp \
    $$top_srcdir/libguh-core/webserver.cpp \
    $$top_srcdir/libguh-core/transportinterface.cpp \
    $$top_srcdir/libguh-core/servermanager.cpp \
    $$top_srcdir/libguh-core/httprequest.cpp \
    $$top_srcdir/libguh-core/websocketserver.cpp \
    $$top_srcdir/libguh-core/httpreply.cpp \
    $$top_srcdir/libguh-core/guhconfiguration.cpp \
    $$top_srcdir/libguh-core/bluetoothserver.cpp \
    $$top_srcdir/libguh-core/jsonrpc/jsonrpcserver.cpp \
    $$top_srcdir/libguh-core/jsonrpc/jsonhandler.cpp \
    $$top_srcdir/libguh-core/jsonrpc/devicehandler.cpp \
    $$top_srcdir/libguh-core/jsonrpc/jsontypes.cpp \
    $$top_srcdir/libguh-core/jsonrpc/ruleshandler.cpp \
    $$top_srcdir/libguh-core/jsonrpc/actionhandler.cpp \
    $$top_srcdir/libguh-core/jsonrpc/eventhandler.cpp \
    $$top_srcdir/libguh-core/jsonrpc/statehandler.cpp \
    $$top_srcdir/libguh-core/jsonrpc/logginghandler.cpp \
    $$top_srcdir/libguh-core/jsonrpc/configurationhandler.cpp \
    $$top_srcdir/libguh-core/jsonrpc/networkmanagerhandler.cpp \
    $$top_srcdir/libguh-core/logging/logengine.cpp \
    $$top_srcdir/libguh-core/logging/logfilter.cpp \
    $$top_srcdir/libguh-core/logging/logentry.cpp \
    $$top_srcdir/libguh-core/rest/restserver.cpp \
    $$top_srcdir/libguh-core/rest/restresource.cpp \
    $$top_srcdir/libguh-core/rest/devicesresource.cpp \
    $$top_srcdir/libguh-core/rest/deviceclassesresource.cpp \
    $$top_srcdir/libguh-core/rest/vendorsresource.cpp \
    $$top_srcdir/libguh-core/rest/logsresource.cpp \
    $$top_srcdir/libguh-core/rest/pluginsresource.cpp \
    $$top_srcdir/libguh-core/rest/rulesresource.cpp \
    $$top_srcdir/libguh-core/time/timedescriptor.cpp \
    $$top_srcdir/libguh-core/time/calendaritem.cpp \
    $$top_srcdir/libguh-core/time/repeatingoption.cpp \
    $$top_srcdir/libguh-core/time/timeeventitem.cpp \
    $$top_srcdir/libguh-core/time/timemanager.cpp \
    $$top_srcdir/libguh-core/networkmanager/networkmanager.cpp \
    $$top_srcdir/libguh-core/networkmanager/networkdevice.cpp \
    $$top_srcdir/libguh-core/networkmanager/wirelessaccesspoint.cpp \
    $$top_srcdir/libguh-core/networkmanager/wirelessnetworkdevice.cpp \
    $$top_srcdir/libguh-core/networkmanager/networksettings.cpp \
    $$top_srcdir/libguh-core/networkmanager/networkconnection.cpp \
    $$top_srcdir/libguh-core/networkmanager/wirednetworkdevice.cpp \
    $$top_srcdir/libguh-core/usermanager.cpp \
    $$top_srcdir/libguh-core/tokeninfo.cpp \
    $$top_srcdir/libguh-core/certificategenerator.cpp \
    $$top_srcdir/libguh-core/logging/logvaluetool.cpp
    $$top_srcdir/libguh-core/awsconnector.cpp \
    $$top_srcdir/libguh-core/cloudconnector.cpp \
    $$top_srcdir/libguh-core/MbedTLS/MbedTLSConnection.cpp \
