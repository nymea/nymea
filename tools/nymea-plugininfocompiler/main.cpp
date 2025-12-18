// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>

#include "plugininfocompiler.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("The nymea plugin info compiler. Compiles a plugininfo.json into a plugininfo.h/plugininfo-extern.h.");
    parser.addHelpOption();
    parser.addOption({{"o", "output"}, "Write generated output header to <file>.", "file"});
    parser.addOption({{"e", "extern"}, "Write generated output header (extern definitions) to <file>.", "file"});
    parser.addOption({{"t", "translations"}, "Write generated translations file stub to <directory>.", "directory"});
    parser.addOption({{"n", "non-strict"}, "Non-strict run. Don't exit on duplicate UUID warnings."});
    parser.addPositionalArgument("input", "The input json file");

    parser.process(a);

    if (parser.positionalArguments().count() != 1) {
        qWarning() << qUtf8Printable(parser.helpText());
        return 1;
    }

    bool strictMode = !parser.isSet("non-strict");

    PluginInfoCompiler pic;

    int ret = pic.compile(parser.positionalArguments().first(), parser.value("output"), parser.value("extern"), parser.value("translations"), strictMode);

    return ret;
}
