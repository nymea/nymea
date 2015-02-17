/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#include "wemoswitch.h"

WemoSwitch::WemoSwitch(QObject *parent, UpnpDeviceDescriptor upnpDeviceDescriptor):
    UpnpDevice(parent, upnpDeviceDescriptor),
    m_powerState(false),
    m_reachable(false)
{
}

void WemoSwitch::setPowerState(const bool &powerState)
{
    m_powerState = powerState;
}

bool WemoSwitch::powerState() const
{
    return m_powerState;
}

void WemoSwitch::setReachable(const bool &reachable)
{
    m_reachable = reachable;
}

bool WemoSwitch::reachable() const
{
    return m_reachable;
}
