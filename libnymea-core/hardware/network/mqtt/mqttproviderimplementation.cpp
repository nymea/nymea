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

#include "mqttproviderimplementation.h"
#include "mqttchannelimplementation.h"
#include "loggingcategories.h"

#include <QUuid>
#include <QtDebug>
#include <QNetworkInterface>
#include <QRegularExpression>

namespace nymeaserver {

MqttProviderImplementation::MqttProviderImplementation(MqttBroker *broker, QObject *parent):
    MqttProvider(parent),
    m_broker(broker)
{
    connect(broker, &MqttBroker::clientConnected, this, &MqttProviderImplementation::onClientConnected);
    connect(broker, &MqttBroker::clientDisconnected, this, &MqttProviderImplementation::onClientDisconnected);
    connect(broker, &MqttBroker::publishReceived, this, &MqttProviderImplementation::onPublishReceived);
}

MqttChannel *MqttProviderImplementation::createChannel(const QHostAddress &clientAddress, const QStringList &topicPrefixList)
{
    QString clientId;
    // Generate a clientId that hasn't been used yet.
    do {
        clientId = QUuid::createUuid().toString().remove(QRegularExpression("[{}-]")).left(16);
    } while (m_createdChannels.contains(clientId));

    return createChannel(clientId, clientAddress, topicPrefixList);
}

MqttChannel *MqttProviderImplementation::createChannel(const QString &clientId, const QHostAddress &clientAddress, const QStringList &topicPrefixList)
{
    QString username = QUuid::createUuid().toString().remove(QRegularExpression("[{}-]")).left(16);
    QString password = QUuid::createUuid().toString().remove(QRegularExpression("[{}-]")).left(16);

    return createChannel(clientId, username, password, clientAddress, topicPrefixList);
}

MqttChannel *MqttProviderImplementation::createChannel(const QString &clientId, const QString &username, const QString &password, const QHostAddress &clientAddress, const QStringList &topicPrefixList)
{
    if (m_broker->configurations().isEmpty()) {
        qCWarning(dcMqtt) << "MQTT broker not running. Cannot create a channel for thing" << clientId;
        return nullptr;
    }

    if (m_createdChannels.contains(clientId)) {
        qCWarning(dcMqtt()) << "ClientId" << clientId << "already in use. Cannot create channel.";
        return nullptr;
    }

    MqttChannelImplementation* channel = new MqttChannelImplementation();
    channel->m_clientId = clientId;
    channel->m_username = username;
    channel->m_password = password;
    if (!topicPrefixList.isEmpty()) {
        channel->m_topicPrefixList = topicPrefixList;
    } else {
        QString defaultTopicPrefix = channel->m_clientId;
        channel->m_topicPrefixList.append(defaultTopicPrefix);
    }


    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
        foreach (const QNetworkAddressEntry &addressEntry, interface.addressEntries()) {
            if (clientAddress.isInSubnet(addressEntry.ip(), addressEntry.prefixLength())) {
                foreach (const ServerConfiguration &config, m_broker->configurations()) {
                    if (QHostAddress(config.address) == QHostAddress("0.0.0.0") || clientAddress.isInSubnet(QHostAddress(config.address), addressEntry.prefixLength())) {
                        channel->m_serverAddress = addressEntry.ip();
                        channel->m_serverPort = config.port;
                        break;
                    }
                }
            }
        }
    }
    if (channel->serverAddress().isNull()) {
        qCWarning(dcMqtt) << "Unable to find a matching MQTT server port for client address" << clientAddress.toString();
        delete channel;
        return nullptr;
    }
    qCDebug(dcMqtt) << "Suitable MQTT server for" << clientAddress.toString() << "found at" << channel->m_serverAddress.toString() << "on port" << channel->m_serverPort;

    connect(channel, &MqttChannelImplementation::pluginPublished, this, &MqttProviderImplementation::onPluginPublished);

    m_createdChannels.insert(channel->clientId(), channel);

    // Create a policy for this client
    MqttPolicy policy;
    policy.clientId = channel->clientId();
    policy.username = channel->username();
    policy.password = channel->password();
    foreach (const QString &topicPrefix, channel->m_topicPrefixList) {
        policy.allowedPublishTopicFilters.append(QString("%1/#").arg(topicPrefix));
        policy.allowedSubscribeTopicFilters.append(QString("%1/#").arg(topicPrefix));
    }
    m_broker->updatePolicy(policy);

    return channel;
}

void MqttProviderImplementation::releaseChannel(MqttChannel *channel)
{
    if (!m_createdChannels.contains(channel->clientId())) {
        qCWarning(dcMqtt) << "ReleaseChannel called for a channel we don't manage. Potential memory leak!";
        return;
    }
    m_createdChannels.take(channel->clientId());
    m_broker->removePolicy(channel->clientId());
    qCDebug(dcMqtt) << "Released MQTT channel for client ID" << channel->clientId();
    delete channel;
}

MqttClient *MqttProviderImplementation::createInternalClient(const QString &clientId)
{

    ServerConfiguration preferredConfig;
    foreach (const ServerConfiguration &config, m_broker->configurations()) {
        if (QHostAddress(config.address) == QHostAddress::Any
                || QHostAddress(config.address) == QHostAddress::AnyIPv4
                || QHostAddress(config.address) == QHostAddress::LocalHost) {
            preferredConfig = config;
            break;
        }
        preferredConfig = config;
    }
    if (preferredConfig.id.isEmpty()) {
        qCWarning(dcMqtt) << "Unable to find a matching MQTT server port for internal client";
        return nullptr;
    }

    MqttPolicy policy;
    policy.clientId = clientId;
    policy.username = QUuid::createUuid().toString().remove(QRegularExpression("[{}-]"));
    policy.password = QUuid::createUuid().toString().remove(QRegularExpression("[{}-]"));
    policy.allowedPublishTopicFilters.append("#");
    policy.allowedSubscribeTopicFilters.append("#");
    m_broker->updatePolicy(policy);

    MqttClient* client = new MqttClient(clientId, this);
    client->setUsername(policy.username);
    client->setPassword(policy.password);
    client->setAutoReconnect(false);

    connect(client, &MqttClient::destroyed, this, [this, clientId]() {
        qCDebug(dcMqtt) << "Internal MQTT client destroyed. Removing policy";
        m_broker->removePolicy(clientId);
    });

    if (QHostAddress(preferredConfig.address) == QHostAddress::Any
            || QHostAddress(preferredConfig.address) == QHostAddress::AnyIPv4
            || QHostAddress(preferredConfig.address) == QHostAddress::LocalHost) {
        client->connectToHost("127.0.0.1", preferredConfig.port);
    } else {
        client->connectToHost(preferredConfig.address, preferredConfig.port);
    }
    return client;
}

bool MqttProviderImplementation::available() const
{
    return m_broker->isRunning();
}

bool MqttProviderImplementation::enabled() const
{
    return available();
}

void MqttProviderImplementation::setEnabled(bool enabled)
{
    Q_UNUSED(enabled)
    qCWarning(dcMqtt) << "MQTT hardware resource cannot be disabled";
}

void MqttProviderImplementation::onClientConnected(const QString &clientId)
{
    if (m_createdChannels.contains(clientId)) {
        MqttChannelImplementation *channel = qobject_cast<MqttChannelImplementation *>(m_createdChannels.value(clientId));
        channel->setConnected(true);
    }
}

void MqttProviderImplementation::onClientDisconnected(const QString &clientId)
{
    if (m_createdChannels.contains(clientId)) {
        MqttChannelImplementation *channel = qobject_cast<MqttChannelImplementation *>(m_createdChannels.value(clientId));
        channel->setConnected(false);
    }
}

void MqttProviderImplementation::onPublishReceived(const QString &clientId, const QString &topic, const QByteArray &payload)
{
    if (m_createdChannels.contains(clientId)) {
        MqttChannel* channel = m_createdChannels.value(clientId);
        emit channel->publishReceived(channel, topic, payload);
    }
}

void MqttProviderImplementation::onPluginPublished(const QString &topic, const QByteArray &payload)
{
    MqttChannelImplementation *channel = static_cast<MqttChannelImplementation*>(sender());
    bool found = false;
    foreach (const QString &topicPrefix, channel->topicPrefixList()) {
        if (topicPrefix.startsWith(topicPrefix)) {
            found = true;
            break;
        }

    }
    if (!found) {
        qCWarning(dcMqtt) << "Attempt to publish to MQTT channel for client" << channel->clientId() << "but topic is not within allowed topic prefix. Discarding message.";
    }
    m_broker->publish(topic, payload);
}

}
