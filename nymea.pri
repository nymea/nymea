COPYRIGHT_YEAR_FROM=2013
COPYRIGHT_YEAR_TO=2025

DEFINES += COPYRIGHT_YEAR_STRING=\\\"$${COPYRIGHT_YEAR_FROM}-$${COPYRIGHT_YEAR_TO}\\\"

greaterThan(QT_MAJOR_VERSION, 5) {
    message("Building using Qt6 support")
    CONFIG *= c++17
    QMAKE_LFLAGS *= -std=c++17
    QMAKE_CXXFLAGS *= -std=c++17
} else {
    message("Building using Qt5 support")
    CONFIG *= c++11
    QMAKE_LFLAGS *= -std=c++11
    QMAKE_CXXFLAGS *= -std=c++11
    DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x050F00
}


QMAKE_CXXFLAGS *= -Werror -g -Wno-deprecated-declarations

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

# Enable coverage option    
coverage {
    # Note: this works only if you build in the source dir
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
        "lcov --extract coverage.info \"*/server/*.cpp\" --extract coverage.info \"*/libnymea-core/*.cpp\" --extract coverage.info \"*/libnymea/*.cpp\" -o coverage.info"; \
        "lcov --remove coverage.info \"moc_*.cpp\" --remove coverage.info \"*/test/*\" -o coverage.info"; \
        "LANG=C genhtml --prefix $${top_srcdir} --output-directory coverage-html --title \"nymea coverage\" --legend --show-details coverage.info"

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

asan: {
    QMAKE_CXXFLAGS *= -fsanitize=address
    QMAKE_LFLAGS *= -fsanitize=address
}

