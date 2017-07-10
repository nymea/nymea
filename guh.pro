include(guh.pri)

TEMPLATE=subdirs

SUBDIRS += libguh server plugins

server.depends = libguh plugins
plugins.depends = libguh
tests.depends = libguh

doc.depends = libguh server
# Note: some how extraimages in qdocconf did not the trick
doc.commands += cd $$top_srcdir/libguh/interfaces; ./generatedoc.sh;
doc.commands += cd $$top_srcdir/doc; qdoc config.qdocconf; cp -r images/* html/images/; \
               cp -r favicons/* html/; cp -r $$top_srcdir/doc/html $$top_builddir/

licensecheck.commands = $$top_srcdir/tests/auto/checklicenseheaders.sh $$top_srcdir

test.depends = licensecheck
test.commands = LD_LIBRARY_PATH=$$top_builddir/libguh:$$top_builddir/tests/libguh-core make check

QMAKE_EXTRA_TARGETS += licensecheck doc test

# Inform about guh build
message(============================================)
message("Qt version:" $$[QT_VERSION])
message("Building guh version $${GUH_VERSION_STRING}")
message("JSON-RPC API version $${JSON_PROTOCOL_VERSION}")
message("REST API version $${REST_API_VERSION}")
message("Plugin path $${GUH_PLUGINS_PATH}")

# Check debug mode
CONFIG(debug, debug|release) {
    message("Debug build")
} else {
    message("Release build")
}

# Build coverage
coverage {
    message("Building coverage.")
}

# Build using ccache
ccache {
    message("Using ccache.")
}

# Build tests
disabletesting {
    message("Building guh without tests")
} else {
    message("Building guh with tests")
    SUBDIRS += tests
}

# Bluetooth LE support
contains(DEFINES, BLUETOOTH_LE) {
    message("Bluetooth LE enabled.")
} else {
    message("Bluetooth LE disabled (Qt $${QT_VERSION} < 5.4.0).")
}

# Ubuntu snappy
contains(DEFINES, SNAPPY) {
    message("Building Ubuntu snappy package.")
}

# GPIO RF 433 MHz support
contains(DEFINES, GPIO433) {
    message("Radio 433 for GPIO's enabled")
} else {
    message("Radio 433 for GPIO's disabled")
}
