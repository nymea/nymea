include(nymea.pri)

TEMPLATE=subdirs

SUBDIRS += libnymea libnymea-core server plugins

libnymea-core.depends = libnymea
server.depends = libnymea libnymea-core plugins
plugins.depends = libnymea
tests.depends = libnymea libnymea-core

doc.depends = FORCE
# Note: some how extraimages in qdocconf did not the trick
doc.commands += cd $$top_srcdir/doc; ./generate-interfaces-qdoc.py;
doc.commands += cd $$top_srcdir/doc; ./generate-api-qdoc.py;
doc.commands += cd $$top_srcdir/doc; qdoc --highlighting config.qdocconf; cp -r images/* html/images/; \
                cp -r favicons/* html/; cp -r $$top_srcdir/doc/html $$top_builddir/
QMAKE_EXTRA_TARGETS += doc

licensecheck.commands = $$top_srcdir/tests/auto/checklicenseheaders.sh $$top_srcdir
QMAKE_EXTRA_TARGETS += licensecheck

# Translations:
# make lupdate to update .ts files
CORE_TRANSLATIONS += $$files($${top_srcdir}/translations/*.ts, true)
lupdate.commands = lupdate -recursive -no-obsolete $${top_srcdir} -ts $${CORE_TRANSLATIONS};
lupdate.commands += make -C plugins/mock plugininfo;
PLUGIN_TRANSLATIONS += $$files($${top_srcdir}/plugins/mock/translations/*.ts, true)
lupdate.commands += lupdate -recursive -no-obsolete $${top_builddir}/plugins/mock/ -ts $${PLUGIN_TRANSLATIONS};
lupdate.depends = FORCE qmake_all
TRANSLATIONS = $${CORE_TRANSLATIONS} $${PLUGIN_TRANSLATIONS}

# make lrelease to compile .ts to .qm
lrelease.depends = FORCE
lrelease.commands = lrelease $$_FILE_; \
                    rsync -a $$top_srcdir/translations/*.qm $$top_builddir/translations/; \
                    rsync -a $$top_srcdir/plugins/mock/translations/*.qm $$top_builddir/plugins/mock/translations/;
first.depends = $(first) lrelease

# Install translation files
translations.path = /usr/share/nymea/translations
translations.depends = lrelease
INSTALLS += translations

QMAKE_EXTRA_TARGETS += lupdate lrelease

test.depends = licensecheck lrelease
test.commands = LD_LIBRARY_PATH=$$top_builddir/libnymea-core:$$top_builddir/libnymea:$$top_builddir/tests/testlib make check
QMAKE_EXTRA_TARGETS += test

# Show doc files in project tree
OTHER_FILES += doc/*.qdoc* \
               doc/tutorials/*.qdoc*

# Inform about nymea build
message(============================================)
message("Qt version:" $$[QT_VERSION])
message("Copyright $${COPYRIGHT_YEAR_FROM} - $${COPYRIGHT_YEAR_TO}")
message("Building nymea version $${NYMEA_VERSION_STRING}")
message("JSON-RPC API version $${JSON_PROTOCOL_VERSION_MAJOR}.$${JSON_PROTOCOL_VERSION_MINOR}")
message("REST API version $${REST_API_VERSION}")
message("Source directory: $${top_srcdir}")
message("Build directory: $${top_builddir}")
message("Translations: $${TRANSLATIONS}")

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
