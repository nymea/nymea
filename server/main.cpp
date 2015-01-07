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
#include <guhcore.h>

#include <QtPlugin>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

void daemonizeGuh() {
    // Our process ID and Session ID
    pid_t pid, sid;

    // Fork off the parent process
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    // If we got a good PID, then we can exit the parent process.
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Change the file mode mask
    umask(0);
    // Create a new SID for the child process
    sid = setsid();
    if (sid < 0) {
        // Log the failure
        exit(EXIT_FAILURE);
    }

    // Change the current working directory
    if ((chdir("/")) < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    // Close out the standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QStringList arguments = a.arguments();

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
        qDebug() << GUH_VERSION_STRING;
        exit(0);
    }

    if (!arguments.contains("-e") && !arguments.contains("--executable")) {
        qDebug() << "Starting guhd as daemon.";
        daemonizeGuh();
    } else {
        qDebug() << "Starting guhd as executable application.";
    }

    a.setOrganizationName("guh");
    GuhCore::instance();
    return a.exec();
}
