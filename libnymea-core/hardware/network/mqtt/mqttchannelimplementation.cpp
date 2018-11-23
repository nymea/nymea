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

QString MqttChannelImplementation::topicPrefix() const
{
    return m_topicPrefix;
}

void MqttChannelImplementation::publish(const QString &topic, const QByteArray &payload)
{
    emit pluginPublished(topic, payload);
}

}
