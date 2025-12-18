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

#include "mqttbroker.h"
#include "loggingcategories.h"

#include <mqttserver.h>

namespace nymeaserver {

class NymeaMqttAuthorizer : public MqttAuthorizer
{
public:
    NymeaMqttAuthorizer(MqttBroker *broker)
        : m_broker(broker)
    {}

    Mqtt::ConnectReturnCode authorizeConnect(int serverAddressId, const QString &clientId, const QString &username, const QString &password, const QHostAddress &peerAddress) override
    {
        Q_UNUSED(peerAddress)
        if (!m_broker->m_configs.value(serverAddressId).authenticationEnabled) {
            qCDebug(dcMqtt) << "Accepting client" << clientId << ". Server configuration does not require authentication.";
            return Mqtt::ConnectReturnCodeAccepted;
        }
        if (!m_broker->m_policies.contains(clientId)) {
            qCDebug(dcMqtt) << "Rejecting client" << clientId << ". No policy for this client installed.";
            return Mqtt::ConnectReturnCodeIdentifierRejected;
        }
        MqttPolicy policy = m_broker->m_policies.value(clientId);
        if (policy.username != username || policy.password != password) {
            qCDebug(dcMqtt) << "Rejecting client" << clientId << ". Bad username or password.";
            return Mqtt::ConnectReturnCodeBadUsernameOrPassword;
        }
        qCDebug(dcMqtt) << "Accepting client" << clientId << ". Login successful.";
        return Mqtt::ConnectReturnCodeAccepted;
    }

    bool authorizeSubscribe(int serverAddressId, const QString &clientId, const QString &topicFilter) override
    {
        if (!m_broker->m_configs.value(serverAddressId).authenticationEnabled) {
            return true;
        }
        if (!m_broker->m_policies.contains(clientId)) {
            return false;
        }
        MqttPolicy policy = m_broker->m_policies.value(clientId);
        return matchPolicy(policy.allowedSubscribeTopicFilters, topicFilter);
    }

    bool authorizePublish(int serverAddressId, const QString &clientId, const QString &topic) override
    {
        if (!m_broker->m_configs.value(serverAddressId).authenticationEnabled) {
            return true;
        }
        if (!m_broker->m_policies.contains(clientId)) {
            return false;
        }
        MqttPolicy policy = m_broker->m_policies.value(clientId);
        return matchPolicy(policy.allowedPublishTopicFilters, topic);
    }

    bool matchPolicy(const QStringList &policies, const QString &topic)
    {
        foreach (const QString &policyFilter, policies) {
            QStringList policyParts = policyFilter.split('/');
            QStringList topicParts = topic.split('/');
            if (topicParts.count() < policyParts.count() - 1) {
                // Nope... actual topic is shorter than filter
                continue;
            }
            bool bad = false;
            for (int i = 0; i < policyParts.length(); i++) {
                if (policyParts.at(i) == QStringLiteral("+")) {
                    continue;
                }
                if (policyParts.at(i) == QStringLiteral("#")) {
                    continue;
                }
                if (policyParts.at(i) == topicParts.at(i)) {
                    continue;
                }
                // Nope... this part does neither match nor is covered by a wildcard in the policy
                bad = true;
            }
            if (bad) {
                continue;
            }
            if (policyParts.count() == topicParts.count() || policyFilter.endsWith('#')) {
                // OK, either the policy is matching or the topicFilter is longer as the policy and policy ends with #
                return true;
            }
        }
        // Nope... none of the policies matched...
        return false;
    }

private:
    MqttBroker *m_broker;
};

MqttBroker::MqttBroker(QObject *parent)
    : QObject(parent)
{
    m_server = new MqttServer(this);
    m_authorizer = new NymeaMqttAuthorizer(this);
    m_server->setAuthorizer(m_authorizer);

    connect(m_server, &MqttServer::clientConnected, this, &MqttBroker::onClientConnected);
    connect(m_server, &MqttServer::clientDisconnected, this, &MqttBroker::onClientDisconnected);
    connect(m_server, &MqttServer::publishReceived, this, &MqttBroker::onPublishReceived);
    connect(m_server, &MqttServer::clientSubscribed, this, &MqttBroker::onClientSubscribed);
    connect(m_server, &MqttServer::clientUnsubscribed, this, &MqttBroker::onClientUnsubscribed);
}

MqttBroker::~MqttBroker()
{
    delete m_server;
    delete m_authorizer;
}

bool MqttBroker::startServer(const ServerConfiguration &config, const QSslConfiguration &sslConfiguration)
{
    int serverAddressId = m_server->listen(QHostAddress(config.address), config.port, config.sslEnabled ? sslConfiguration : QSslConfiguration());
    if (serverAddressId == -1) {
        qCWarning(dcMqtt) << "Error starting MQTT server on port" << config.port;
        return false;
    }
    qCDebug(dcMqtt) << "MQTT server running on" << config.address << ":" << config.port;
    m_configs.insert(serverAddressId, config);
    return true;
}

bool MqttBroker::isRunning(const QString &configId) const
{
    foreach (const ServerConfiguration &config, m_configs) {
        if (config.id == configId) {
            return true;
        }
    }
    return false;
}

bool MqttBroker::isRunning() const
{
    return !m_configs.isEmpty();
}

QList<ServerConfiguration> MqttBroker::configurations() const
{
    return m_configs.values();
}

void MqttBroker::stopServer(const QString &configId)
{
    int serverAddressId = -1;
    foreach (const ServerConfiguration &config, m_configs) {
        if (config.id == configId) {
            serverAddressId = m_configs.key(config);
            break;
        }
    }
    if (serverAddressId == -1) {
        qCWarning(dcMqtt) << "Config" << configId << "unknown to MQTT server. Cannot stop server";
        return;
    }
    m_server->close(serverAddressId);
    qCDebug(dcMqtt) << "MQTT server stopped on" << m_configs.value(serverAddressId).address << ":" << m_configs.value(serverAddressId).port;
    m_configs.remove(serverAddressId);
}

QList<MqttPolicy> MqttBroker::policies()
{
    return m_policies.values();
}

MqttPolicy MqttBroker::policy(const QString &clientId)
{
    return m_policies.value(clientId);
}

void MqttBroker::updatePolicy(const MqttPolicy &policy)
{
    if (m_policies.contains(policy.clientId)) {
        m_policies[policy.clientId] = policy;
        qCDebug(dcMqtt) << "Policy for client" << policy.clientId << "updated.";
        emit policyChanged(policy);
        return;
    }
    qCDebug(dcMqtt) << "Policy for client" << policy.clientId << "added.";
    m_policies.insert(policy.clientId, policy);
    emit policyAdded(policy);
}

void MqttBroker::updatePolicies(const QList<MqttPolicy> &policies)
{
    foreach (const MqttPolicy &policy, policies) {
        updatePolicy(policy);
    }
}

bool MqttBroker::removePolicy(const QString &clientId)
{
    if (m_policies.contains(clientId)) {
        // Is there a client connected for this policy?
        if (m_server->clients().contains(clientId)) {
            m_server->disconnectClient(clientId);
        }

        qCDebug(dcMqtt) << "Policy for client" << clientId << "removed";
        emit policyRemoved(m_policies.take(clientId));
        return true;
    }
    return false;
}

void MqttBroker::publish(const QString &topic, const QByteArray &payload)
{
    m_server->publish(topic, payload);
}

void MqttBroker::onClientConnected(int serverAddressId, const QString &clientId, const QString &username, const QHostAddress &clientAddress)
{
    Q_UNUSED(serverAddressId)
    qCDebug(dcMqtt) << "Client" << clientId << "connected with username" << username << "from" << clientAddress.toString();
    emit clientConnected(clientId);
}

void MqttBroker::onClientDisconnected(const QString &clientId)
{
    qCDebug(dcMqtt) << "Client" << clientId << "disconnected";
    emit clientDisconnected(clientId);
}

void MqttBroker::onPublishReceived(const QString &clientId, quint16 packetId, const QString &topic, const QByteArray &payload)
{
    Q_UNUSED(packetId)
    qCDebug(dcMqtt) << "Publish received from client" << clientId << ":" << topic << ">" << payload;
    emit publishReceived(clientId, topic, payload);
}

void MqttBroker::onClientSubscribed(const QString &clientId, const QString &topicFilter, Mqtt::QoS requestedQoS)
{
    qCDebug(dcMqtt) << "Client" << clientId << "subscribed to" << topicFilter << "(QoS:" << requestedQoS << ")";
    emit clientSubscribed(clientId, topicFilter);
}

void MqttBroker::onClientUnsubscribed(const QString &clientId, const QString &topicFilter)
{
    qCDebug(dcMqtt) << "Client" << clientId << "unsubscribed from" << topicFilter;
    emit clientUnsubscribed(clientId, topicFilter);
}

} // namespace nymeaserver
