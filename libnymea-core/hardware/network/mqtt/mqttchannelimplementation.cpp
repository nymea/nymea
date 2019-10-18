/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "mqttchannelimplementation.h"

namespace nymeaserver {

MqttChannelImplementation::MqttChannelImplementation() : MqttChannel()
{

}

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

}
