include(nymea.pri)

TEMPLATE=subdirs

SUBDIRS += libnymea libnymea-core server plugins

libnymea-core.depends = libnymea
server.depends = libnymea libnymea-core plugins
plugins.depends = libnymea
tests.depends = libnymea libnymea-core

doc.depends = FORCE
# Note: some how extraimages in qdocconf did not the trick
doc.commands += cd $$top_srcdir/libnymea/interfaces; ./generatedoc.sh;
doc.commands += cd $$top_srcdir/doc; ./generate-api-qdoc.py;
doc.commands += cd $$top_srcdir/doc; qdoc config.qdocconf; cp -r images/* html/images/; \
                cp -r favicons/* html/; cp -r $$top_srcdir/doc/html $$top_builddir/

licensecheck.commands = $$top_srcdir/tests/auto/checklicenseheaders.sh $$top_srcdir

test.depends = licensecheck
test.commands = LD_LIBRARY_PATH=$$top_builddir/libnymea-core:$$top_builddir/libnymea make check

# Translations:
# make lupdate to update .ts files
TRANSLATIONS += $$files(translations/*.ts, true)
TRANSLATIONS += $$files(plugins/mock/translations/*.ts, true)
lupdate.depends = FORCE
lupdate.commands = lupdate -recursive -no-obsolete $$_FILE_;

# make lrelease to compile .ts to .qm
lrelease.depends = FORCE
lrelease.commands = lrelease $$_FILE_; \
                    rsync -a $$top_srcdir/translations/*.qm $$top_builddir/translations/;

# Install translation files
translations.path = /usr/share/nymea/translations
translations.files = $$[QT_SOURCE_TREE]/translations/*.qm
translations.depends = lrelease
INSTALLS += translations

QMAKE_EXTRA_TARGETS += licensecheck doc test lupdate lrelease

# Inform about nymea build
message(============================================)
message("Qt version:" $$[QT_VERSION])
message("Building nymea version $${NYMEA_VERSION_STRING}")
message("JSON-RPC API version $${JSON_PROTOCOL_VERSION_MAJOR}.$${JSON_PROTOCOL_VERSION_MINOR}")
message("REST API version $${REST_API_VERSION}")
message("Plugin path $${NYMEA_PLUGINS_PATH}")
message("Source directory: $${top_srcdir}")
message("Build directory: $${top_builddir}")

# Check debug mode
CONFIG(debug, debug|release) {
    message("Debug build")
} else {
    message("Release build")
}

# Build coverage
coverage {
    message("Building coverage.")
}

# Build using ccache
ccache {
    message("Using ccache.")
}

# Build tests
disabletesting {
    message("Building nymea without tests")
} else {
    message("Building nymea with tests")
    SUBDIRS += tests
}
