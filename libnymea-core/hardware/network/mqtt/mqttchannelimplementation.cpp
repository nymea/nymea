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

#include "mqttchannelimplementation.h"

namespace nymeaserver {

MqttChannelImplementation::MqttChannelImplementation()
    : MqttChannel()
{}

QString MqttChannelImplementation::clientId() const
{
    return m_clientId;
}

QString MqttChannelImplementation::username() const
{
    return m_username;
}

QString MqttChannelImplementation::password() const
{
    return m_password;
}

QHostAddress MqttChannelImplementation::serverAddress() const
{
    return m_serverAddress;
}

quint16 MqttChannelImplementation::serverPort() const
{
    return m_serverPort;
}

QStringList MqttChannelImplementation::topicPrefixList() const
{
    return m_topicPrefixList;
}

void MqttChannelImplementation::publish(const QString &topic, const QByteArray &payload)
{
    emit pluginPublished(topic, payload);
}

bool MqttChannelImplementation::isConnected() const
{
    return m_connected;
}

void MqttChannelImplementation::setConnected(bool connected)
{
    m_connected = connected;
    if (m_connected) {
        emit clientConnected(this);
    } else {
        emit clientDisconnected(this);
    }
}

} // namespace nymeaserver
