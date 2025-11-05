// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "platformsystemcontroller.h"

#include "loggingcategories.h"

PlatformSystemController::PlatformSystemController(QObject *parent) : QObject(parent)
{

}

bool PlatformSystemController::powerManagementAvailable() const
{
    return false;
}

bool PlatformSystemController::restart()
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

QString PlatformSystemController::deviceSerialNumber() const
{
    // Sadly there is no real standardized way to read the device's serial number, especially when it comes to ARM platforms.
    // Because of this, even the most common platforms (e.g. systemd, all standard linux) differ when it comes to this.
    // In order to not being forced to write a new backend plugin just for this serial number, one can also set this by
    // using the DEVICE_SERIAL environment variable.
    QString serial;
    if (qEnvironmentVariableIsSet("DEVICE_SERIAL")) {
        serial = QString::fromLocal8Bit(qgetenv("DEVICE_SERIAL"));
    } else {
        qCWarning(dcPlatform()) << "Platform plugin does not implement deviceSerialNumber and DEVICE_SERIAL is not set. Cannot determine device serial number.";
    }
    return serial;
}
