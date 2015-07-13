include(guh.pri)

TEMPLATE=subdirs

SUBDIRS += libguh server plugins

!disabletesting {
    SUBDIRS += tests
}

server.depends = libguh plugins
plugins.depends = libguh
tests.depends = libguh

doc.depends = libguh server
doc.commands = cd $$top_srcdir/doc; qdoc config.qdocconf; cp images/logo.png html/images/

licensecheck.commands = $$top_srcdir/tests/auto/checklicenseheaders.sh $$top_srcdir

test.depends = licensecheck
test.commands = LD_LIBRARY_PATH=$$top_builddir/libguh make check

QMAKE_EXTRA_TARGETS += licensecheck doc test

message("Building guh version $${GUH_VERSION_STRING} (API version $${JSON_PROTOCOL_VERSION})")

coverage {
    message("Building coverage.")
}

!isEmpty(PREFIX) {
    message("Install guhd to $$PREFIX")
}

contains(DEFINES, GPIO433){
    message("Radio 433 for GPIO's enabled")
} else {
    message("Radio 433 for GPIO's disabled")
}

contains(DEFINES, SNAPPY){
    message("Building Ubuntu snappy package.")
    isEmpty(PREFIX) {
        INSTALLDIR = ""
    } else {
        INSTALLDIR = $$PREFIX
    }

    meta.files = meta/package.yaml \
                 meta/readme.md \
                 meta/guh-logo.svg \
                 meta/license.txt \
                 meta/packLibs.sh
    meta.path = $$INSTALLDIR/meta/

    wrapper.files = meta/guhd-wrapper.sh
    wrapper.path = $$INSTALLDIR/usr/bin/

#    # We need to bring our own Qt libs, at least for now
#    qtlibs.files = /usr/lib/arm-linux-gnueabihf/libQt5Network.so.5 \
#                   /usr/lib/arm-linux-gnueabihf/libQt5Sql.so.5 \
#                   /usr/lib/arm-linux-gnueabihf/libQt5Core.so.5 \
#                   /usr/lib/arm-linux-gnueabihf/libQt5Test.so.5 \
#                   /usr/lib/arm-linux-gnueabihf/libQt5Gui.so.5 \
#                   /usr/lib/arm-linux-gnueabihf/libicui18n.so.52 \
#                   /usr/lib/arm-linux-gnueabihf/libicuuc.so.52 \
#                   /usr/lib/arm-linux-gnueabihf/libicudata.so.52
#    qtlibs.path = /usr/lib/

    # install sqlite driver
    sqlplugin.files = /usr/lib/arm-linux-gnueabihf/qt5/plugins/sqldrivers/libqsqlite.so
    sqlplugin.path = $$INSTALLDIR/usr/lib/qt5/plugins/sqldrivers/

    # install guhd.conf
    guhdconf.files = data/config/guhd.conf
    guhdconf.path =  $$INSTALLDIR/config/

    INSTALLS += meta wrapper guhdconf sqlplugin

    # command to pack libs for snappy package
    packlibs.depends = libguh server
    packlibs.commands = $$top_srcdir/meta/packLibs.sh $$INSTALLDIR/usr/lib/

    QMAKE_EXTRA_TARGETS += packlibs

}
