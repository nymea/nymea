include(../guh.pri)

TARGET = guhd
TEMPLATE = app

INCLUDEPATH += ../libguh jsonrpc

target.path = /usr/bin
INSTALLS += target

QT += sql xml websockets bluetooth

LIBS += -L$$top_builddir/libguh/ -lguh

# Translations
TRANSLATIONS *= $$top_srcdir/translations/guhd_en_US.ts \
                $$top_srcdir/translations/guhd_de_DE.ts

include(../translations.pri)

# Server files
include(server.pri)
include(qtservice/qtservice.pri)

SOURCES += main.cpp \
    guhservice.cpp \
    guhapplication.cpp

HEADERS += \
    guhservice.h \
    guhapplication.h
