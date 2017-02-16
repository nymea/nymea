/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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


/*!
    \class guhserver::GuhService
    \brief The daemon service reprenetation of the guh server.

    \ingroup core
    \inmodule server

    The \l{GuhService} represents the forked guhd.

    \sa QtService
*/

#include "unistd.h"
#include "stdio.h"

#include <QDir>

#include "guhservice.h"
#include "guhsettings.h"
#include "loggingcategories.h"


namespace guhserver {

/*! Constructs the forked guhd application with the given argument count \a argc and argument vector \a argv. */
GuhService::GuhService(int argc, char **argv):
    QtService<QCoreApplication>(argc, argv, "guh - IoT server")
{
    application()->setOrganizationName("guh");
    application()->setApplicationName("guhd");
    application()->setApplicationVersion(GUH_VERSION_STRING);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    setServiceDescription("guh - IoT server");
    setServiceFlags(QtServiceBase::CanBeSuspended);
}

/*! Destroyes the forked guhd application. */
GuhService::~GuhService()
{
}

/*! Starts the forked guhd application. */
void GuhService::start()
{
    // check if config directory for logfile exists
    if (!QDir().mkpath(GuhSettings::settingsPath())) {
        fprintf(stdout, "Could not create guh settings directory %s", qPrintable(GuhSettings::settingsPath()));
        exit(EXIT_FAILURE);
    }
    qCDebug(dcApplication) << "=====================================";
    qCDebug(dcApplication) << "guhd" << GUH_VERSION_STRING << "started as daemon.";
    qCDebug(dcApplication) << "=====================================";
    GuhCore::instance();
}

}
