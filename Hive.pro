TEMPLATE=subdirs

SUBDIRS += libhive server plugins

server.depends = libhive plugins
plugins.depends = libhive

doc.depends = libhive server
doc.commands = cd ${PWD}; qdoc config.qdocconf
QMAKE_EXTRA_TARGETS += doc
