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

}

#endif // MQTTCHANNELIMPLEMENTATION_H
