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


#include "devicesetupinfo.h"

DeviceSetupInfo::DeviceSetupInfo()
{

}

DeviceSetupInfo::DeviceSetupInfo(const DeviceId &deviceId, Device::DeviceError status, const QString &displayMessage):
    m_deviceId(deviceId),
    m_status(status),
    m_displayMessage(displayMessage)
{

}

DeviceId DeviceSetupInfo::deviceId() const
{
    return m_deviceId;
}

Device::DeviceError DeviceSetupInfo::status() const
{
    return m_status;
}

void DeviceSetupInfo::setStatus(Device::DeviceError status)
{
    m_status = status;
}

QString DeviceSetupInfo::displayMessage() const
{
    return m_displayMessage;
}

void DeviceSetupInfo::setDisplayMessage(const QString &displayMessage)
{
    m_displayMessage = displayMessage;
}
