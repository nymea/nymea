/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QtPlugin>

#include "guhcore.h"
#include "guhservice.h"

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    application.setOrganizationName("guh");
    application.setApplicationName("guhd");
    application.setApplicationVersion(GUH_VERSION_STRING);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QString description = QString("\nguh ( /[guːh]/ ) is an open source home automation server, which allows to\n"
                                  "control a lot of different devices from many different manufacturers.\n"
                                  "guhd %1 (C) 2014-2015 guh\n"
                                  "Released under the GNU GENERAL PUBLIC LICENSE Version 2.").arg(GUH_VERSION_STRING);

    parser.setApplicationDescription(description);

    QCommandLineOption foregroundOption(QStringList() << "n" << "no-daemon", QCoreApplication::translate("main", "Run guhd in the foreground, not as daemon."));
    parser.addOption(foregroundOption);

    parser.process(application);

    bool startForeground = parser.isSet(foregroundOption);
    if (startForeground) {
        GuhCore::instance()->setRunningMode(GuhCore::RunningModeApplication);
        return application.exec();
    }

    GuhService service(argc, argv);
    return service.exec();
}
