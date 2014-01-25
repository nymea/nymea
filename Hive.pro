TEMPLATE=subdirs

SUBDIRS += libhive server plugins

server.depends = libhive plugins
plugins.depends = libhive

docs.depends = libhive server
docs.commands = cd ../doc; qdoc config.qdocconf
QMAKE_EXTRA_TARGETS += docs
