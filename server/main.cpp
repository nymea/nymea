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
#include <QtPlugin>
#include <unistd.h>

#include "guhcore.h"
#include "guhservice.h"

int main(int argc, char *argv[])
{
    QStringList arguments;
    for (int i = 0; i < argc; ++i) {
        arguments.append(QString(argv[i]));
    }

    if (arguments.contains("-h") || arguments.contains("--help")) {
        qDebug() << "guhd" << GUH_VERSION_STRING << "(C) 2014-2015 guh" ;
        qDebug() << "Released under the GNU GENERAL PUBLIC LICENSE Version 2";
        qDebug() << "";
        qDebug() << "guh (/[guːh]/ - pronounced German and sounds like \"goo\") is an open source";
        qDebug() << "home automation server, which allows to control a lot of different devices ";
        qDebug() << "from many different manufacturers.";
        qDebug() << "";
        qDebug() << "options:";
        qDebug() << "   -h,     --help              print this help message";
        qDebug() << "   -v,     --version           print version";
        qDebug() << "   -e,     --executable        start guh as application, not as daemon";
        qDebug() << "";
        exit(0);
    }

    if (arguments.contains("-v") || arguments.contains("--version")) {
        qDebug() << "guhd" << GUH_VERSION_STRING;
        exit(0);
    }

    if (!arguments.contains("-e") && !arguments.contains("--executable")) {
        qDebug() << "guhd started as daemon.";
        GuhService service(argc, argv);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        return service.exec();
    }

    QCoreApplication a(argc, argv);
    qDebug() << "guhd started as executable.";
    a.setOrganizationName("guh");
    a.setApplicationName("guhd");
    GuhCore::instance();
    return a.exec();
}
