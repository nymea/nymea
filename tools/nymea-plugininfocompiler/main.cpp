/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
