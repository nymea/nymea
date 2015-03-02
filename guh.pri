# Parse and export GUH_VERSION_STRING
GUH_VERSION_STRING=$$system('dpkg-parsechangelog | sed -n -e "s/^Version: //p"')
DEFINES += GUH_VERSION_STRING=\\\"$${GUH_VERSION_STRING}\\\"

QT+= network

QMAKE_CXXFLAGS += -Werror
CONFIG += c++11

# Check for Bluetoot LE support (Qt >= 5.4.0)
!contains(QT_VERSION, ^5\\.[0-3]\\..*) {
    QT += bluetooth
    DEFINES += BLUETOOTH_LE
}

# Enable coverage option    
coverage {
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage -O0
    LIBS += -lgcov
    QMAKE_LFLAGS += -fprofile-arcs
}

top_srcdir=$$PWD
top_builddir=$$shadowed($$PWD)

