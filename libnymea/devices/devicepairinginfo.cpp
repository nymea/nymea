/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "devicepairinginfo.h"

DevicePairingInfo::DevicePairingInfo()
{

}

DevicePairingInfo::DevicePairingInfo(const DeviceClassId &deviceClassId, const QString &deviceName, const ParamList &params, const DeviceId &deviceId) :
    m_transactionId(PairingTransactionId::createPairingTransactionId()),
    m_deviceClassId(deviceClassId),
    m_deviceId(deviceId),
    m_deviceName(deviceName),
    m_params(params)
{

}

PairingTransactionId DevicePairingInfo::transactionId() const
{
    return m_transactionId;
}

DeviceClassId DevicePairingInfo::deviceClassId() const
{
    return m_deviceClassId;
}

DeviceId DevicePairingInfo::deviceId() const
{
    return m_deviceId;
}

QString DevicePairingInfo::deviceName() const
{
    return m_deviceName;
}

ParamList DevicePairingInfo::params() const
{
    return m_params;
}

Device::DeviceError DevicePairingInfo::status() const
{
    return m_status;
}

void DevicePairingInfo::setStatus(Device::DeviceError status)
{
    m_status = status;
}

QString DevicePairingInfo::message() const
{
    return m_message;
}

void DevicePairingInfo::setMessage(const QString &message)
{
    m_message = message;
}

QUrl DevicePairingInfo::oAuthUrl() const
{
    return m_oAuthUrl;
}

void DevicePairingInfo::setOAuthUrl(const QUrl &url)
{
    m_oAuthUrl = url;
}
