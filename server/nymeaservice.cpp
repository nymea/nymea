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

/*!
    \class nymeaserver::NymeaService
    \brief The daemon service reprenetation of the nymea server.

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
#include "version.h"

Q_DECLARE_LOGGING_CATEGORY(dcApplication)

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
