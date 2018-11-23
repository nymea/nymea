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
    MqttChannelImplementation* channel = new MqttChannelImplementation();
    channel->m_clientId = deviceId.toString().remove(QRegExp("[{}-]"));
    channel->m_username = QUuid::createUuid().toString().remove(QRegExp("[{}-]"));
    channel->m_password = QUuid::createUuid().toString().remove(QRegExp("[{}-]"));
    channel->m_topicPrefix = channel->m_clientId;

    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
        qCDebug(dcMqtt) << "### Interface:" << interface.name();
        foreach (const QNetworkAddressEntry &addressEntry, interface.addressEntries()) {
            qCDebug(dcMqtt) << "#### Address entry:" << addressEntry.ip();
            if (clientAddress.isInSubnet(addressEntry.ip(), addressEntry.prefixLength())) {
                qCDebug(dcMqtt) << "##### Is in subnet";
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
    qCDebug(dcMqtt).nospace() << "Found matching MQTT server interface " << channel->m_serverAddress << ":" << channel->m_serverPort << " for client IP " << clientAddress;

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
