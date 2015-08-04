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

server.depends = libguh plugins
plugins.depends = libguh
tests.depends = libguh

doc.depends = libguh server
doc.commands = cd $$top_srcdir/doc; qdoc config.qdocconf; cp images/logo.png html/images/

licensecheck.commands = $$top_srcdir/tests/auto/checklicenseheaders.sh $$top_srcdir

test.depends = licensecheck
test.commands = LD_LIBRARY_PATH=$$top_builddir/libguh make check

QMAKE_EXTRA_TARGETS += licensecheck doc test

message(Qt version: $$[QT_VERSION])
message("Building guh version $${GUH_VERSION_STRING}")
message("JSON-RPC API version $${JSON_PROTOCOL_VERSION}")
message("REST API version $${REST_API_VERSION}")

coverage {
    message("Building coverage.")
}

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
