include(guh.pri)

TEMPLATE=subdirs

SUBDIRS += libguh server plugins

!disabletesting {
    SUBDIRS += tests
}

# Bluetooth LE support
contains(DEFINES, BLUETOOTH_LE) {
    message("Bluetooth LE available (Qt $${QT_VERSION}).")
} else {
    message("Bluetooth LE not available (Qt $${QT_VERSION}).")
}

server.depends = libguh plugins
plugins.depends = libguh
tests.depends = libguh

doc.depends = libguh server
doc.commands = cd $$top_srcdir/doc; qdoc config.qdocconf; cp images/logo.png html/images/

licensecheck.commands = $$top_srcdir/tests/auto/checklicenseheaders.sh $$top_srcdir

test.depends = licensecheck
test.commands = LD_LIBRARY_PATH=$$top_builddir/libguh make check

QMAKE_EXTRA_TARGETS += licensecheck doc test

coverage {
    message("Building coverage.")
}
