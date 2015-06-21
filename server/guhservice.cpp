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

#include <unistd.h>
#include "guhservice.h"
#include "loggingcategories.h"

GuhService::GuhService(int argc, char **argv):
    QtService<QCoreApplication>(argc, argv, "guh daemon")
{
    qDebug() << "guhd started as daemon.";
    application()->setOrganizationName("guh");
    application()->setApplicationName("guhd");
    application()->setApplicationVersion(GUH_VERSION_STRING);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    setServiceDescription("guh daemon");
    setServiceFlags(QtServiceBase::CanBeSuspended);
}

GuhService::~GuhService()
{
}

void GuhService::start()
{
    GuhCore::instance()->setRunningMode(GuhCore::RunningModeService);
}
