TEMPLATE=subdirs

SUBDIRS += libguh server plugins

!disabletesting {
    SUBDIRS += tests
}

server.depends = libguh plugins
plugins.depends = libguh
tests.depends = libguh

doc.depends = libguh server
doc.commands = cd $$top_srcdir/doc; qdoc config.qdocconf; cp images/mascot.png html/images/

licensecheck.commands = $$top_srcdir/tests/auto/checklicenseheaders.sh $$top_srcdir

test.depends = licensecheck check

QMAKE_EXTRA_TARGETS += licensecheck doc test

coverage {
    message("Building coverage.")
}
