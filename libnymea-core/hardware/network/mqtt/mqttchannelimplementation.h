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
    QString topicPrefix() const override;

    void publish(const QString &topic, const QByteArray &payload) override;

signals:
    void pluginPublished(const QString &topic, const QByteArray &payload);

private:
    QString m_clientId;
    QString m_username;
    QString m_password;
    QHostAddress m_serverAddress;
    quint16 m_serverPort;
    QString m_topicPrefix;

    friend class MqttProviderImplementation;
};

}

#endif // MQTTCHANNELIMPLEMENTATION_H
