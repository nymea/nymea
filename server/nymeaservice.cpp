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
    \class nymeaserver::NymeaService
    \brief The daemon service reprenetation of the guh server.

    \ingroup core
    \inmodule server

    The \l{NymeaService} represents the forked nymead.

    \sa QtService
*/

#include "unistd.h"
#include "stdio.h"

#include <QDir>

#include "nymeaservice.h"
#include "nymeacore.h"
#include "nymeasettings.h"
#include "loggingcategories.h"


namespace nymeaserver {

/*! Constructs the forked nymead application with the given argument count \a argc and argument vector \a argv. */
NymeaService::NymeaService(int argc, char **argv):
    QtService<QCoreApplication>(argc, argv, "nymea - IoT server")
{
    application()->setOrganizationName("nymea");
    application()->setApplicationName("nymead");
    application()->setApplicationVersion(NYMEA_VERSION_STRING);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    setServiceDescription("nymea - IoT server");
    setServiceFlags(QtServiceBase::CanBeSuspended);
}

/*! Destroyes the forked nymead application. */
NymeaService::~NymeaService()
{
}

/*! Starts the forked nymead application. */
void NymeaService::start()
{
    // check if config directory for logfile exists
    if (!QDir().mkpath(NymeaSettings::settingsPath())) {
        fprintf(stdout, "Could not create nymea settings directory %s", qPrintable(NymeaSettings::settingsPath()));
        exit(EXIT_FAILURE);
    }
    qCDebug(dcApplication) << "=====================================";

    qCDebug(dcApplication) << "nymead" << NYMEA_VERSION_STRING << "started as daemon.";
    qCDebug(dcApplication) << "=====================================";
    NymeaCore::instance();
}

}
