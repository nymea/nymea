/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#include <unistd.h>
#include "guhservice.h"
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
    qCDebug(dcApplication) << "=====================================";
    qCDebug(dcApplication) << "guhd" << GUH_VERSION_STRING << "started as daemon.";
    qCDebug(dcApplication) << "=====================================";
    GuhCore::instance()->setRunningMode(GuhCore::RunningModeService);
}

void GuhService::stop()
{
    qCDebug(dcApplication) << "=====================================";
    qCDebug(dcApplication) << "Shutting down guh daemon";
    qCDebug(dcApplication) << "=====================================";
}

}
