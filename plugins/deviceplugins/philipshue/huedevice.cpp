/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#include "huedevice.h"

HueDevice::HueDevice(QObject *parent) :
    QObject(parent)
{

}

int HueDevice::id() const
{
    return m_id;
}

void HueDevice::setId(const int &id)
{
    m_id = id;
}

QString HueDevice::name() const
{
    return m_name;
}

void HueDevice::setName(const QString &name)
{
    m_name = name;
}

DeviceId HueDevice::bridgeId() const
{
    return m_bridgeId;
}

void HueDevice::setBridgeId(const DeviceId &bridgeId)
{
    m_bridgeId = bridgeId;
}

QHostAddress HueDevice::hostAddress() const
{
    return m_hostAddress;
}

void HueDevice::setHostAddress(const QHostAddress hostAddress)
{
    m_hostAddress = hostAddress;
}

QString HueDevice::uuid()
{
    return m_uuid;
}

void HueDevice::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString HueDevice::apiKey() const
{
    return m_apiKey;
}

void HueDevice::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

QString HueDevice::modelId() const
{
    return m_modelId;
}

void HueDevice::setModelId(const QString &modelId)
{
    m_modelId = modelId;
}

QString HueDevice::type() const
{
    return m_type;
}

void HueDevice::setType(const QString &type)
{
    m_type = type;
}

QString HueDevice::softwareVersion() const
{
    return m_softwareVersion;
}

void HueDevice::setSoftwareVersion(const QString &softwareVersion)
{
    m_softwareVersion = softwareVersion;
}

bool HueDevice::reachable() const
{
    return m_reachable;
}

void HueDevice::setReachable(const bool &reachable)
{
    m_reachable = reachable;
}

