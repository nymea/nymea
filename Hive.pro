TEMPLATE=subdirs

SUBDIRS += libhive server plugins

server.depends = libhive plugins
plugins.depends = libhive

