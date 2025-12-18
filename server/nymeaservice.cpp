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

/*!
    \class nymeaserver::NymeaService
    \brief The daemon service reprenetation of the nymea server.

    \ingroup core
    \inmodule server

    The \l{NymeaService} represents the forked nymead.

    \sa QtService
*/

#include "stdio.h"
#include "unistd.h"

#include <QDir>

#include "loggingcategories.h"
#include "nymeacore.h"
#include "nymeaservice.h"
#include "nymeasettings.h"
#include "version.h"

Q_DECLARE_LOGGING_CATEGORY(dcApplication)

namespace nymeaserver {

/*! Constructs the forked nymead application with the given argument count \a argc and argument vector \a argv. */
NymeaService::NymeaService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, "nymea - IoT server")
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
NymeaService::~NymeaService() {}

/*! Starts the forked nymead application. */
void NymeaService::start()
{
    // check if config directory for logfile exists
    if (!QDir().mkpath(NymeaSettings::settingsPath())) {
        fprintf(stdout, "Could not create nymea settings directory %s", qPrintable(NymeaSettings::settingsPath()));
        exit(EXIT_FAILURE);
    }

    qCDebug(dcApplication()) << "=====================================";
    qCDebug(dcApplication()) << "nymead" << NYMEA_VERSION_STRING << "started as daemon.";
    qCDebug(dcApplication()) << "=====================================";
    NymeaCore::instance();
}

} // namespace nymeaserver
