QT += testlib network sql
CONFIG += testcase

INCLUDEPATH += $$top_srcdir/libnymea \
               $$top_srcdir/libnymea-core \
               $$top_srcdir/tests/auto/

LIBS += -L$$top_builddir/libnymea/ -lnymea \
        -L$$top_builddir/libnymea-core/ -lnymea-core \
        -L$$top_builddir/plugins/mock/ \
        -lssl -lcrypto -lavahi-common -lavahi-client -lnymea-remoteproxyclient

SOURCES += ../nymeatestbase.cpp \

HEADERS += ../nymeatestbase.h \

target.path = /usr/tests
INSTALLS += target
