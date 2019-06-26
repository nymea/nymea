QT -= gui
CONFIG += c++11 console
CONFIG -= app_bundle

include(../../nymea.pri)
INCLUDEPATH += $$top_srcdir/libnymea
LIBS += -L$$top_builddir/libnymea -lnymea

SOURCES += \
        main.cpp \
    plugininfocompiler.cpp

HEADERS += \
    plugininfocompiler.h

target.path = $$[QT_INSTALL_PREFIX]/bin
INSTALLS += target
