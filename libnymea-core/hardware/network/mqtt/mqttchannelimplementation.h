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
