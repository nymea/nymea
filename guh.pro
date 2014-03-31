TEMPLATE=subdirs

SUBDIRS += libguh server plugins tests

server.depends = libguh plugins
plugins.depends = libguh
tests.depends = libguh

doc.depends = libguh server
doc.commands = cd $$top_srcdir/doc; qdoc config.qdocconf
QMAKE_EXTRA_TARGETS += doc
