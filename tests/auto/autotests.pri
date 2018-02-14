QT += testlib network sql
CONFIG += testcase

INCLUDEPATH += $$top_srcdir/libguh \
               $$top_srcdir/libguh-core \
               $$top_srcdir/libguh-core/jsonrpc \
               $$top_srcdir/tests/auto/

LIBS += -L$$top_builddir/libguh/ -lguh -L$$top_builddir/plugins/mock/ \
        -L$$top_builddir/libguh-core/ -lguh-core -lssl -lcrypto -laws-iot-sdk-cpp -lavahi-common -lavahi-client

SOURCES += ../guhtestbase.cpp \

HEADERS += ../guhtestbase.h \

target.path = /usr/tests
INSTALLS += target
