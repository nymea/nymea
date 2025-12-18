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

#ifndef MQTTCHANNELIMPLEMENTATION_H
#define MQTTCHANNELIMPLEMENTATION_H

#include "network/mqtt/mqttchannel.h"

namespace nymeaserver {

class MqttChannelImplementation : public MqttChannel
{
    Q_OBJECT
public:
    explicit MqttChannelImplementation();

    QString clientId() const override;
    QString username() const override;
    QString password() const override;
    QHostAddress serverAddress() const override;
    quint16 serverPort() const override;
    QStringList topicPrefixList() const override;

    void publish(const QString &topic, const QByteArray &payload) override;

    bool isConnected() const override;
    void setConnected(bool connected);

signals:
    void pluginPublished(const QString &topic, const QByteArray &payload);

private:
    QString m_clientId;
    QString m_username;
    QString m_password;
    QHostAddress m_serverAddress;
    quint16 m_serverPort;
    QStringList m_topicPrefixList;
    bool m_connected = false;

    friend class MqttProviderImplementation;
};

} // namespace nymeaserver

#endif // MQTTCHANNELIMPLEMENTATION_H
