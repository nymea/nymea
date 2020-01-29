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

#include "platformsystemcontroller.h"

#include "loggingcategories.h"

PlatformSystemController::PlatformSystemController(QObject *parent) : QObject(parent)
{

}

bool PlatformSystemController::powerManagementAvailable() const
{
    return false;
}

bool PlatformSystemController::reboot()
{
    return false;
}

bool PlatformSystemController::shutdown()
{
    return false;
}

bool PlatformSystemController::timeManagementAvailable() const
{
    return false;
}

bool PlatformSystemController::automaticTimeAvailable() const
{
    return false;
}

bool PlatformSystemController::automaticTime() const
{
    return false;
}

bool PlatformSystemController::setTime(const QDateTime &time)
{
    Q_UNUSED(time)
    qCWarning(dcPlatform()) << "setTime not implemented in platform plugin";
    return false;
}

bool PlatformSystemController::setAutomaticTime(bool automaticTime)
{
    Q_UNUSED(automaticTime)
    qCWarning(dcPlatform()) << "setAutomaticTime not implemented in platform plugin";
    return false;
}

bool PlatformSystemController::setTimeZone(const QTimeZone &timeZone)
{
    Q_UNUSED(timeZone)
    qCWarning(dcPlatform()) << "setTimeZone not implemented in platform plugin";
    return false;
}
