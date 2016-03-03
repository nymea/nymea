/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#include "devicepairinginfo.h"

DevicePairingInfo::DevicePairingInfo()
{

}

DevicePairingInfo::DevicePairingInfo(const DeviceClassId &deviceClassId, const QString &deviceName, const ParamList &params) :
    m_deviceClassId(deviceClassId),
    m_deviceName(deviceName),
    m_params(params)
{

}

DevicePairingInfo::DevicePairingInfo(const DeviceClassId &deviceClassId, const QString &deviceName, const DeviceDescriptorId &deviceDescriptorId) :
    m_deviceClassId(deviceClassId),
    m_deviceName(deviceName),
    m_deviceDescriptorId(deviceDescriptorId)
{

}

DeviceClassId DevicePairingInfo::deviceClassId() const
{
    return m_deviceClassId;
}

QString DevicePairingInfo::deviceName() const
{
    return m_deviceName;
}

ParamList DevicePairingInfo::params() const
{
    return m_params;
}

DeviceDescriptorId DevicePairingInfo::deviceDescriptorId() const
{
    return m_deviceDescriptorId;
}

