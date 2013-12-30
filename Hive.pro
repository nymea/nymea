TEMPLATE=subdirs

SUBDIRS += libhive server plugins

server.depends = libhive plugins
libhive.deoends = libhive

