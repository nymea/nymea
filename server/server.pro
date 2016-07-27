include(../guh.pri)

TARGET = guhd
TEMPLATE = app

INCLUDEPATH += ../libguh jsonrpc

target.path = /usr/bin
INSTALLS += target

QT += sql xml websockets

LIBS += -L$$top_builddir/libguh/ -lguh

# Translations
TRANSLATIONS *= $$top_srcdir/translations/guhd_en_US.ts \
                $$top_srcdir/translations/guhd_de_DE.ts

# Update ts files and create translation qm files
lrelease.input = TRANSLATIONS
lrelease.CONFIG += no_link
lrelease.output = $$top_srcdir/${QMAKE_FILE_BASE}.qm
lrelease.commands = $$[QT_INSTALL_BINS]/lupdate $$_FILE_; \
                    $$[QT_INSTALL_BINS]/lrelease ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm;

QMAKE_EXTRA_COMPILERS += lrelease
PRE_TARGETDEPS += compiler_lrelease_make_all

# Install translation files
translations.path = /usr/share/guh/translations
translations.files = $$[QT_SOURCE_TREE]/translations/*.qm
INSTALLS += translations

# Server files
include(server.pri)
include(qtservice/qtservice.pri)

SOURCES += main.cpp \
    guhservice.cpp \
    guhapplication.cpp

HEADERS += \
    guhservice.h \
    guhapplication.h
