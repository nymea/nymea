QT += testlib network sql
CONFIG += testcase

DEFINES += TESTING_ENABLED

INCLUDEPATH += $$top_srcdir/server/ \
               $$top_srcdir/server/jsonrpc \
               $$top_srcdir/libguh \
               $$top_srcdir/tests/auto/

LIBS += -L$$top_builddir/libguh/ -lguh -L$$top_builddir/plugins/deviceplugins/mock/ \
        -L$$top_builddir/tests/libguh-core/ -lguh-core

SOURCES += ../guhtestbase.cpp \
    ../mocktcpserver.cpp \

HEADERS += ../guhtestbase.h \
    ../mocktcpserver.h

isEmpty(PREFIX) {
    INSTALLDIR = /usr/tests/
} else {
    INSTALLDIR = $$PREFIX/usr/tests
}

target.path = $$INSTALLDIR
INSTALLS += target
