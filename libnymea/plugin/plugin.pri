#  Copyright (C) 2018 Michael Zanetti <michael.zanetti@guh.io>
#  Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>
#
#  This file is part of nymea.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; If not, see
#  <http://www.gnu.org/licenses/>.

# This project include file is meant to be used by nymea plugins.
# Example project file for a plugin:

# include(/usr/include/nymea/plugi.pri)
# TARGET = $$qtLibraryTarget(nymea_devicepluginexample)
# SOURCES += devicepluginexample.cpp
# HEADERS += devicepluginexample.h


TEMPLATE = lib
CONFIG += plugin link_pkgconfig

PKGCONFIG += nymea

QMAKE_CXXFLAGS *= -Werror -std=c++11 -g
QMAKE_LFLAGS *= -std=c++11

# Check if ccache is enabled
ccache {
    QMAKE_CXX = ccache g++
}

# Make the device plugin json file visible in the Qt Creator
OTHER_FILES+=deviceplugin"$$TARGET".json

# NOTE: if the code includes "plugininfo.h", it would fail if we only give it a compiler for $$OUT_PWD/plugininfo.h
# Let's add a dummy target with the plugininfo.h file without any path to allow the developer to just include it like that.

# Create plugininfo file
plugininfo.target = $$OUT_PWD/plugininfo.h
plugininfo_dummy.target = plugininfo.h
extern_plugininfo.target = $$OUT_PWD/extern-plugininfo.h
extern_plugininfo_dummy.target = extern-plugininfo.h
plugininfo.depends = FORCE
plugininfo.commands = nymea-generateplugininfo --filetype i --jsonfile $${_PRO_FILE_PWD_}/deviceplugin"$$TARGET".json --output plugininfo.h --builddir $$OUT_PWD; \
                      nymea-generateplugininfo --filetype e --jsonfile $${_PRO_FILE_PWD_}/deviceplugin"$$TARGET".json --output extern-plugininfo.h --builddir $$OUT_PWD
plugininfo_dummy.commands = $$plugininfo.commands
QMAKE_EXTRA_TARGETS += plugininfo plugininfo_dummy extern_plugininfo extern_plugininfo_dummy

plugininfo_clean.commands = rm -f $$OUT_PWD/plugininfo.h $$OUT_PWD/extern-plugininfo.h
clean.depends = plugininfo_clean
QMAKE_EXTRA_TARGETS += clean plugininfo_clean

# Install translation files
TRANSLATIONS *= $$files($${_PRO_FILE_PWD_}/translations/*ts, true)
lupdate.depends = FORCE
lupdate.depends += plugininfo
lupdate.commands = lupdate -recursive -no-obsolete $${_PRO_FILE_PWD_}/"$$TARGET".pro;
QMAKE_EXTRA_TARGETS += lupdate

# make lrelease to build .qm from .ts
lrelease.depends = FORCE
lrelease.commands += lrelease $$files($$_PRO_FILE_PWD_/translations/*.ts, true);
lrelease.commands += rsync -a $${_PRO_FILE_PWD_}/translations/*.qm $${OUT_PWD}/translations/;
QMAKE_EXTRA_TARGETS += lrelease

translations.path = /usr/share/nymea/translations
translations.files = $$[QT_SOURCE_TREE]/translations/*.qm

HEADERS += $$OUT_PWD/plugininfo.h \
           $$OUT_PWD/extern-plugininfo.h
DEPENDPATH += $$OUT_PWD

# Install plugin
target.path = $$[QT_INSTALL_LIBS]/nymea/plugins/
INSTALLS += target translations
