/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "platformupdatecontroller.h"

PlatformUpdateController::PlatformUpdateController(QObject *parent) : QObject(parent)
{

}

/*! Override this to indicate whether update management is available. Defaults to false.
    When a plugin returns true here, it is assumed that the system is capable of updating and nymea
    has permissions to do so.
    */
bool PlatformUpdateController::updateManagementAvailable()
{
    return false;
}

bool PlatformUpdateController::updateRunning() const
{
    return false;
}

QList<Package> PlatformUpdateController::packages() const
{
    return {};
}

QList<Repository> PlatformUpdateController::repositories() const
{
    return {};
}

bool PlatformUpdateController::startUpdate(const QStringList &packageIds)
{
    Q_UNUSED(packageIds)
    return false;
}

bool PlatformUpdateController::rollback(const QStringList &packageIds)
{
    Q_UNUSED(packageIds)
    return false;
}

bool PlatformUpdateController::removePackages(const QStringList &packageIds)
{
    Q_UNUSED(packageIds)
    return false;
}

bool PlatformUpdateController::enableRepository(const QString &repositoryId, bool enabled)
{
    Q_UNUSED(repositoryId)
    Q_UNUSED(enabled)
    return false;
}

