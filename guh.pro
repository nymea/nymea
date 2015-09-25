include(guh.pri)

TEMPLATE=subdirs

SUBDIRS += libguh server plugins

server.depends = libguh plugins
plugins.depends = libguh
tests.depends = libguh

doc.depends = libguh server
# Note: some how extraimages in qdocconf did not the trick
doc.commands = cd $$top_srcdir/doc; qdoc config.qdocconf; cp images/logo.png html/images/; \
               cp -v favicons/* html/; cp -r $$top_srcdir/doc/html $$top_builddir/

licensecheck.commands = $$top_srcdir/tests/auto/checklicenseheaders.sh $$top_srcdir

test.depends = licensecheck
test.commands = LD_LIBRARY_PATH=$$top_builddir/libguh make check

QMAKE_EXTRA_TARGETS += licensecheck doc test

# Inform about guh build
message("--------------------------------------")
message(Qt version: $$[QT_VERSION])
message("Building guh version $${GUH_VERSION_STRING}")
message("JSON-RPC API version $${JSON_PROTOCOL_VERSION}")
message("REST API version $${REST_API_VERSION}")

# Build coverage
coverage {
    message("Building coverage.")
}

# Bluetooth LE support
contains(DEFINES, BLUETOOTH_LE) {
    message("Bluetooth LE enabled.")
} else {
    message("Bluetooth LE disabled (Qt $${QT_VERSION} < 5.4.0).")
}

# Websocket support
contains(DEFINES, WEBSOCKET){
    message("Building guh with websocket.")
} else {
    message("Building guh without websocket.")
}

# GPIO RF 433 MHz support
contains(DEFINES, GPIO433){
    message("Radio 433 for GPIO's enabled")
} else {
    message("Radio 433 for GPIO's disabled")
}

# Check installation prefix
!isEmpty(PREFIX) {
    message("Install guhd to $$PREFIX")
}

contains(DEFINES, SNAPPY){
    message("Building Ubuntu snappy package.")

    CONFIG += disabletesting

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

    # install sqlite driver
    sqlplugin.files = /usr/lib/arm-linux-gnueabihf/qt5/plugins/sqldrivers/libqsqlite.so
    sqlplugin.path = $$INSTALLDIR/usr/lib/qt5/plugins/sqldrivers/

    # We need to bring our own Qt libs, at least for now
    qtlibs.files = /usr/lib/arm-linux-gnueabihf/libQt5Network.so.5 \
                   /usr/lib/arm-linux-gnueabihf/libQt5Sql.so.5 \
                   /usr/lib/arm-linux-gnueabihf/libQt5Core.so.5 \
                   /usr/lib/arm-linux-gnueabihf/libQt5Test.so.5 \
                   /usr/lib/arm-linux-gnueabihf/libQt5Gui.so.5 \
                   /usr/lib/arm-linux-gnueabihf/libQt5WebSockets.so.5 \
                   /usr/lib/arm-linux-gnueabihf/libicui18n.so.52 \
                   /usr/lib/arm-linux-gnueabihf/libicuuc.so.52 \
                   /usr/lib/arm-linux-gnueabihf/libpng12.so.0 \
                   /usr/lib/arm-linux-gnueabihf/libharfbuzz.so.0 \
                   /usr/lib/arm-linux-gnueabihf/libicudata.so.52 \
                   /usr/lib/arm-linux-gnueabihf/libz.so \
                   /usr/lib/arm-linux-gnueabihf/libicuuc.so.52 \
                   /usr/lib/arm-linux-gnueabihf/libicui18n.so.52 \
                   /usr/lib/arm-linux-gnueabihf/libstdc++.so.6 \
                   /usr/lib/arm-linux-gnueabihf/libsqlite3.so.0 \
                   /usr/lib/arm-linux-gnueabihf/mesa-egl/libGLESv2.so.2.0.0 \
                   /usr/lib/arm-linux-gnueabihf/mesa-egl/libGLESv2.so.2 \
                   /usr/lib/arm-linux-gnueabihf/mesa-egl/libGLESv2.so \
                   /usr/lib/arm-linux-gnueabihf/libfreetype.so.6 \
                   /usr/lib/arm-linux-gnueabihf/libgraphite2.so.3 \
                   /usr/lib/arm-linux-gnueabihf/libglapi.so.0 \
                   /usr/lib/arm-linux-gnueabihf/libpthread.so \
                   /usr/lib/arm-linux-gnueabihf/libdl.so \
                   /usr/lib/arm-linux-gnueabihf/librt.so \
                   /usr/lib/arm-linux-gnueabihf/libm.so
    qtlibs.path = /usr/lib/

    # install guhd.conf
    guhdconf.files = data/config/guhd.conf
    guhdconf.path =  $$INSTALLDIR/config/

    INSTALLS += wrapper qtlibs sqlplugin

    # command to pack libs for snappy package
    packlibs.depends = libguh server
    packlibs.commands = $$top_srcdir/meta/packLibs.sh $$INSTALLDIR

    QMAKE_EXTRA_TARGETS += packlibs
}

# Build tests
!disabletesting {
    message("Building guh tests enabled")
    SUBDIRS += tests
    DEFINES += TESTING_ENABLED
} else {
    message("Building guh tests disabled")
}
