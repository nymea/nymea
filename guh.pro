TEMPLATE=subdirs

SUBDIRS += libguh server plugins tests

server.depends = libguh plugins
plugins.depends = libguh
tests.depends = libguh

doc.depends = libguh server
doc.commands = cd $$top_srcdir/doc; qdoc config.qdocconf

licensecheck.commands = $$top_srcdir/tests/auto/checklicenseheaders.sh $$top_srcdir

test.depends = licensecheck check

QMAKE_EXTRA_TARGETS += licensecheck doc test
