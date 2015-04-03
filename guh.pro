include(guh.pri)

TEMPLATE=subdirs

SUBDIRS += libguh server plugins

!disabletesting {
    SUBDIRS += tests
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

message("Building guh version $${GUH_VERSION_STRING}")

coverage {
    message("Building coverage.")
}

contains(DEFINES, GPIO433){
    message("Radio 433 for GPIO's enabled")
} else {
    message("Radio 433 for GPIO's disabled")
}
