# Parse and export GUH_VERSION_STRING
GUH_VERSION_STRING=$$system('dpkg-parsechangelog | sed -n -e "s/^Version: //p"')
DEFINES += GUH_VERSION_STRING=\\\"$${GUH_VERSION_STRING}\\\"

QT+= network

QMAKE_CXXFLAGS += -Werror
CONFIG += c++11

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

top_srcdir=$$PWD
top_builddir=$$shadowed($$PWD)

