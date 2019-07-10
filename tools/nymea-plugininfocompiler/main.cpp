/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
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
    parser.addPositionalArgument("input", "The input json file");

    parser.process(a);

    if (parser.positionalArguments().count() != 1) {
        qWarning() << qUtf8Printable(parser.helpText());
        return 1;
    }

    PluginInfoCompiler pic;

    int ret = pic.compile(parser.positionalArguments().first(), parser.value("output"), parser.value("extern"));

    return ret;

}
