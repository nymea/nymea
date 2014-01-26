TEMPLATE=subdirs

SUBDIRS += libhive server plugins tests

server.depends = libhive plugins
plugins.depends = libhive
tests.depends = libhive

doc.depends = libhive server
doc.commands = cd $$top_srcdir/doc; qdoc config.qdocconf
QMAKE_EXTRA_TARGETS += doc
