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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

}
