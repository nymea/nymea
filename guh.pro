include(guh.pri)

TEMPLATE=subdirs

SUBDIRS += libguh server plugins

!disabletesting {
    message("Building guh tests enabled")
    SUBDIRS += tests
    DEFINES += TESTING_ENABLED
} else {
    message("Building guh tests disabled")
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
doc.commands = cd $$top_srcdir/doc; qdoc config.qdocconf; cp images/logo.png html/images/; \
               cp favicons/* html/; cp -r $$top_srcdir/doc/html $$top_builddir/

licensecheck.commands = $$top_srcdir/tests/auto/checklicenseheaders.sh $$top_srcdir

test.depends = licensecheck
test.commands = LD_LIBRARY_PATH=$$top_builddir/libguh:$$top_builddir/tests/libguh-core make check

QMAKE_EXTRA_TARGETS += licensecheck doc test

# Inform about guh build
message("--------------------------------------")
message(Qt version: $$[QT_VERSION])
message("Building guh version $${GUH_VERSION_STRING}")
message("JSON-RPC API version $${JSON_PROTOCOL_VERSION}")
message("REST API version $${REST_API_VERSION}")

coverage {
    message("Building coverage.")
}

# Build tests
disabletesting {
    message("Building guh without tests")
} else {
    message("Building guh with tests")
    SUBDIRS += tests
    DEFINES += TESTING_ENABLED
}

# Bluetooth LE support
contains(DEFINES, BLUETOOTH_LE) {
    message("Bluetooth LE enabled.")
} else {
    message("Bluetooth LE disabled (Qt $${QT_VERSION} < 5.4.0).")
}

# Websocket support
contains(DEFINES, WEBSOCKET){
    message("Building guh with websocket.")
} else {
    message("Building guh without websocket.")
}

# GPIO RF 433 MHz support
contains(DEFINES, GPIO433){
    message("Radio 433 for GPIO's enabled")
} else {
    message("Radio 433 for GPIO's disabled")
}
