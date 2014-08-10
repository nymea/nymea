include(../guh.pri)

message("Building guh version $${GUH_VERSION_STRING}")

TARGET = guh
TEMPLATE = app

INCLUDEPATH += ../libguh jsonrpc

target.path = /usr/bin
INSTALLS += target

QT += network
CONFIG += c++11

LIBS += -L$$top_builddir/libguh/ -lguh

include(server.pri)
SOURCES += main.cpp

# FIXME: Drop this and link them dynamically
LIBS += -L../plugins/deviceplugins/elro/ -lguh_devicepluginelro
LIBS += -L../plugins/deviceplugins/intertechno/ -lguh_devicepluginintertechno
#LIBS += -L../plugins/deviceplugins/meisteranker/ -lguh_devicepluginmeisteranker
LIBS += -L../plugins/deviceplugins/wifidetector/ -lguh_devicepluginwifidetector
LIBS += -L../plugins/deviceplugins/conrad -lguh_devicepluginconrad
LIBS += -L../plugins/deviceplugins/mock -lguh_devicepluginmock
LIBS += -L../plugins/deviceplugins/openweathermap -lguh_devicepluginopenweathermap
LIBS += -L../plugins/deviceplugins/lircd -lguh_devicepluginlircd
LIBS += -L../plugins/deviceplugins/mailnotification -lguh_devicepluginmailnotification
LIBS += -L../plugins/deviceplugins/wakeonlan -lguh_devicepluginwakeonlan
LIBS += -L../plugins/deviceplugins/philipshue -lguh_devicepluginphilipshue
LIBS += -L../plugins/deviceplugins/eq-3 -lguh_deviceplugineq3
LIBS += -L../plugins/deviceplugins/wemo -lguh_devicepluginwemo

boblight {
    xcompile {
        LIBS += -L../plugins/deviceplugins/boblight -lguh_devicepluginboblight -lboblight
    } else {
        LIBS += -L../plugins/deviceplugins/boblight -lguh_devicepluginboblight -L/usr/local/lib/ -lboblight
    }
    DEFINES += USE_BOBLIGHT
}
