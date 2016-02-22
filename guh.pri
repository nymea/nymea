# Parse and export GUH_VERSION_STRING
GUH_VERSION_STRING=$$system('dpkg-parsechangelog | sed -n -e "s/^Version: //p"')

# define protocol versions
JSON_PROTOCOL_VERSION=36
REST_API_VERSION=1

DEFINES += GUH_VERSION_STRING=\\\"$${GUH_VERSION_STRING}\\\" \
           JSON_PROTOCOL_VERSION=\\\"$${JSON_PROTOCOL_VERSION}\\\" \
           REST_API_VERSION=\\\"$${REST_API_VERSION}\\\"

QT+= network

QMAKE_CXXFLAGS += -Werror -std=c++11
QMAKE_LFLAGS += -std=c++11

# Check for Bluetoot LE support (Qt >= 5.4)
equals(QT_MAJOR_VERSION, 5):greaterThan(QT_MINOR_VERSION, 3) {
    QT += bluetooth
    DEFINES += BLUETOOTH_LE
}

# Enable coverage option    
coverage {
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage -O0
    LIBS += -lgcov
    QMAKE_LFLAGS += -fprofile-arcs
}

# Enable Radio 433 MHz for GPIO's
enable433gpio {
    DEFINES += GPIO433
}

# check websocket support (Qt >= 5.3)
equals(QT_MAJOR_VERSION, 5):greaterThan(QT_MINOR_VERSION, 2) {
    DEFINES += WEBSOCKET
}

top_srcdir=$$PWD
top_builddir=$$shadowed($$PWD)
