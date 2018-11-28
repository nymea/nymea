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

#include "mqttproviderimplementation.h"
#include "mqttchannelimplementation.h"
#include "loggingcategories.h"

#include <QtDebug>
#include <QUuid>
#include <QNetworkInterface>

namespace nymeaserver {

MqttProviderImplementation::MqttProviderImplementation(MqttBroker *broker, QObject *parent):
    MqttProvider(parent),
    m_broker(broker)
{
    connect(broker, &MqttBroker::clientConnected, this, &MqttProviderImplementation::onClientConnected);
    connect(broker, &MqttBroker::clientDisconnected, this, &MqttProviderImplementation::onClientDisconnected);
    connect(broker, &MqttBroker::publishReceived, this, &MqttProviderImplementation::onPublishReceived);
}

MqttChannel *MqttProviderImplementation::createChannel(const DeviceId &deviceId, const QHostAddress &clientAddress)
{
    if (m_broker->configurations().isEmpty()) {
        qCWarning(dcMqtt) << "MQTT broker not running. Cannot create a channel for device" << deviceId;
        return nullptr;
    }

    MqttChannelImplementation* channel = new MqttChannelImplementation();
    channel->m_clientId = deviceId.toString().remove(QRegExp("[{}-]"));
    channel->m_username = QUuid::createUuid().toString().remove(QRegExp("[{}-]"));
    channel->m_password = QUuid::createUuid().toString().remove(QRegExp("[{}-]"));
    channel->m_topicPrefix = channel->m_clientId;


    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
        foreach (const QNetworkAddressEntry &addressEntry, interface.addressEntries()) {
            if (clientAddress.isInSubnet(addressEntry.ip(), addressEntry.prefixLength())) {
                foreach (const ServerConfiguration &config, m_broker->configurations()) {
                    if (config.address == QHostAddress("0.0.0.0") || clientAddress.isInSubnet(config.address, addressEntry.prefixLength())) {
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
    policy.allowedPublishTopicFilters.append(QString("%1/#").arg(channel->m_topicPrefix));
    policy.allowedSubscribeTopicFilters.append(QString("%1/#").arg(channel->m_topicPrefix));
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

MqttClient *MqttProviderImplementation::createInternalClient(const DeviceId &deviceId)
{

    ServerConfiguration preferredConfig;
    foreach (const ServerConfiguration &config, m_broker->configurations()) {
        if (config.address == QHostAddress::Any
                || config.address == QHostAddress::AnyIPv4
                || config.address == QHostAddress::LocalHost) {
            preferredConfig = config;
            break;
        }
        preferredConfig = config;
    }
    if (preferredConfig.id.isEmpty()) {
        qCWarning(dcMqtt) << "Unable to find a matching MQTT server port for internal client";
        return nullptr;
    }

    QString clientId = deviceId.toString().remove(QRegExp("[{}-]"));
    MqttPolicy policy;
    policy.clientId = clientId;
    policy.username = QUuid::createUuid().toString().remove(QRegExp("[{}-]"));
    policy.password = QUuid::createUuid().toString().remove(QRegExp("[{}-]"));
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

    if (preferredConfig.address == QHostAddress::Any
            || preferredConfig.address == QHostAddress::AnyIPv4
            || preferredConfig.address == QHostAddress::LocalHost) {
        client->connectToHost("127.0.0.1", preferredConfig.port);
    } else {
        client->connectToHost(preferredConfig.address.toString(), preferredConfig.port);
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
        MqttChannel* channel = m_createdChannels.value(clientId);
        emit channel->clientConnected(channel);
    }
}

void MqttProviderImplementation::onClientDisconnected(const QString &clientId)
{
    if (m_createdChannels.contains(clientId)) {
        MqttChannel *channel = m_createdChannels.value(clientId);
        emit channel->clientDisconnected(channel);
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
    if (!topic.startsWith(channel->topicPrefix())) {
        qCWarning(dcMqtt) << "Attempt to publish to MQTT channel for client" << channel->clientId() << "but topic is not within allowed topic prefix. Discarding message.";
        return;
    }
    m_broker->publish(topic, payload);
}

}
