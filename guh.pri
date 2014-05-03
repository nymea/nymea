GUH_VERSION_STRING=$$system('dpkg-parsechangelog | sed -n -e "s/^Version: //p"')

coverage {
    message("Building coverage.")
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage -O0
    LIBS += -lgcov
    QMAKE_LFLAGS += -fprofile-arcs
}

DEFINES += GUH_VERSION_STRING=\\\"$${GUH_VERSION_STRING}\\\"
