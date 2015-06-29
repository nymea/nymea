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
message(============================================)
message(Qt version: $$[QT_VERSION])
message("Building guh version $${GUH_VERSION_STRING}")
message("JSON-RPC API version $${JSON_PROTOCOL_VERSION}")
message("REST API version $${REST_API_VERSION}")

coverage {
    message("Building coverage.")
}

# Build tests
!disabletesting {
    message("Building guh with tests")
    SUBDIRS += tests
    DEFINES += TESTING_ENABLED
} else {
    message("Building guh without tests")
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

contains(DEFINES, GPIO433){
    message("Radio 433 for GPIO's enabled")
} else {
    message("Radio 433 for GPIO's disabled")
}

ubuntu {
    message("Building Ubuntu snappy package.")
    meta.files = meta/package.yaml \
                 meta/readme.md
    meta.path = /meta/

    wrapper.files = meta/guhd-wrapper.sh
    wrapper.path = /usr/bin/

    # We need to bring our own Qt libs, at least for now
    qtlibs.files=/usr/lib/arm-linux-gnueabihf/libQt5Network.so.5 \
                 /usr/lib/arm-linux-gnueabihf/libQt5Sql.so.5 \
                 /usr/lib/arm-linux-gnueabihf/libQt5Core.so.5 \
                 /usr/lib/arm-linux-gnueabihf/libicui18n.so.52 \
                 /usr/lib/arm-linux-gnueabihf/libicuuc.so.52 \
                 /usr/lib/arm-linux-gnueabihf/libicudata.so.52
    qtlibs.path=/usr/lib/

    INSTALLS += meta wrapper qtlibs
}
