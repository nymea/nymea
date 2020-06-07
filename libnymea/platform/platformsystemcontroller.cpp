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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
