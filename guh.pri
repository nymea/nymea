# Parse and export GUH_VERSION_STRING
GUH_VERSION_STRING=$$system('dpkg-parsechangelog | sed -n -e "s/^Version: //p"')

# Install path for plugins
GUH_PLUGINS_PATH=/usr/lib/$$system('dpkg-architecture -q DEB_HOST_MULTIARCH')/guh/plugins/

# define protocol versions
JSON_PROTOCOL_VERSION=49
REST_API_VERSION=1

DEFINES += GUH_VERSION_STRING=\\\"$${GUH_VERSION_STRING}\\\" \
           JSON_PROTOCOL_VERSION=\\\"$${JSON_PROTOCOL_VERSION}\\\" \
           REST_API_VERSION=\\\"$${REST_API_VERSION}\\\" \
           GUH_PLUGINS_PATH=\\\"$${GUH_PLUGINS_PATH}\\\"

QT *= network websockets bluetooth dbus

QMAKE_CXXFLAGS *= -Werror -std=c++11 -g
QMAKE_LFLAGS *= -std=c++11

top_srcdir=$$PWD
top_builddir=$$shadowed($$PWD)

# Check if ccache is enabled
ccache {
    QMAKE_CXX = ccache g++
}

debian {
    QMAKE_CPPFLAGS *= $(shell dpkg-buildflags --get CPPFLAGS)
    QMAKE_CFLAGS   *= $(shell dpkg-buildflags --get CFLAGS)
    QMAKE_CXXFLAGS *= $(shell dpkg-buildflags --get CXXFLAGS)
    QMAKE_LFLAGS   *= $(shell dpkg-buildflags --get LDFLAGS)
}

# Check for Bluetoot LE support (Qt >= 5.4)
equals(QT_MAJOR_VERSION, 5):greaterThan(QT_MINOR_VERSION, 3) {
    DEFINES += BLUETOOTH_LE
}

# Enable coverage option    
coverage {
    OBJECTS_DIR =
    MOC_DIR =

    LIBS += -lgcov
    QMAKE_CXXFLAGS += --coverage
    QMAKE_LDFLAGS += --coverage

    QMAKE_EXTRA_TARGETS += coverage cov
    QMAKE_EXTRA_TARGETS += clean-gcno clean-gcda coverage-html \
        generate-coverage-html clean-coverage-html coverage-gcovr \
        generate-gcovr generate-coverage-gcovr clean-coverage-gcovr

    clean-gcno.commands = \
        "@echo Removing old coverage instrumentation"; \
        "find -name '*.gcno' -print | xargs -r rm"

    clean-gcda.commands = \
        "@echo Removing old coverage results"; \
        "find -name '*.gcda' -print | xargs -r rm"

    coverage-html.depends = clean-gcda check generate-coverage-html

    generate-coverage-html.commands = \
        "@echo Collecting coverage data"; \
        "lcov --directory $${top_srcdir} --capture --output-file coverage.info --no-checksum --compat-libtool"; \
        "lcov --extract coverage.info \"*/server/*.cpp\" --extract coverage.info \"*/libguh/*.cpp\" -o coverage.info"; \
        "lcov --remove coverage.info \"moc_*.cpp\" --remove coverage.info \"*/test/*\" -o coverage.info"; \
        "LANG=C genhtml --prefix $${top_srcdir} --output-directory coverage-html --title \"guh coverage\" --legend --show-details coverage.info"

    clean-coverage-html.depends = clean-gcda
    clean-coverage-html.commands = \
        "lcov --directory $${top_srcdir} -z"; \
        "rm -rf coverage.info coverage-html"

    coverage-gcovr.depends = clean-gcda check generate-coverage-gcovr

    generate-coverage-gcovr.commands = \
        "@echo Generating coverage GCOVR report"; \
        "gcovr -x -r $${top_srcdir} -o $${top_srcdir}/coverage.xml -e \".*/moc_.*\" -e \"tests/.*\" -e \".*\\.h\""

    clean-coverage-gcovr.depends = clean-gcda
    clean-coverage-gcovr.commands = \
        "rm -rf $${top_srcdir}/coverage.xml"

    QMAKE_CLEAN += *.gcda *.gcno coverage.info coverage.xml
}

# Ubuntu snappy
snappy {
    DEFINES += SNAPPY
}

# Enable Radio 433 MHz for GPIO's
enable433gpio {
    DEFINES += GPIO433
}


