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

#ifndef MQTTBROKER_H
#define MQTTBROKER_H

#include <QObject>
#include <QHostAddress>
#include <QSslConfiguration>

#include "nymea-mqtt/mqtt.h"
#include "nymeaconfiguration.h"

class MqttServer;

namespace nymeaserver {

class NymeaMqttAuthorizer;

class MqttBroker : public QObject
{
    Q_OBJECT
public:
    explicit MqttBroker(QObject *parent = nullptr);
    ~MqttBroker();

    bool startServer(const ServerConfiguration &config, const QSslConfiguration &sslConfiguration = QSslConfiguration());
    bool isRunning(const QString &configId) const;
    bool isRunning() const;
    QList<ServerConfiguration> configurations() const;
    void stopServer(const QString &configId);

    QList<MqttPolicy> policies();
    MqttPolicy policy(const QString &clientId);
    void updatePolicy(const MqttPolicy &policy);
    void updatePolicies(const QList<MqttPolicy> &policies);
    bool removePolicy(const QString &clientId);

    void publish(const QString &topic, const QByteArray &payload);

private slots:
    void onClientConnected(int serverAddressId, const QString &clientId, const QString &username, const QHostAddress &clientAddress);
    void onClientDisconnected(const QString &clientId);
    void onPublishReceived(const QString &clientId, quint16 packetId, const QString &topic, const QByteArray &payload);
    void onClientSubscribed(const QString &clientId, const QString &topicFilter, Mqtt::QoS requestedQoS);
    void onClientUnsubscribed(const QString &clientId, const QString &topicFilter);

signals:
    void clientConnected(const QString &clientId);
    void clientDisconnected(const QString &clientId);
    void publishReceived(const QString &clientId, const QString &topic, const QByteArray &payload);
    void clientSubscribed(const QString &clientId, const QString &topicFilter);
    void clientUnsubscribed(const QString &clientId, const QString &topicFilter);
    void policyAdded(const MqttPolicy &policy);
    void policyChanged(const MqttPolicy &policy);
    void policyRemoved(const MqttPolicy &policy);

private:
    MqttServer* m_server = nullptr;
    NymeaMqttAuthorizer *m_authorizer = nullptr;
    QHash<int, ServerConfiguration> m_configs;
    QHash<QString, MqttPolicy> m_policies;


    friend class NymeaMqttAuthorizer;
};
}

#endif // MQTTBROKER_H
