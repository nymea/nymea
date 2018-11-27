#ifndef MQTTPROVIDERIMPLEMENTATION_H
#define MQTTPROVIDERIMPLEMENTATION_H

#include <QObject>

#include "servers/mqttbroker.h"

#include "network/mqtt/mqttprovider.h"
namespace nymeaserver {

class MqttProviderImplementation : public MqttProvider
{
    Q_OBJECT
public:
    explicit MqttProviderImplementation(MqttBroker *broker, QObject *parent = nullptr);

    MqttChannel* createChannel(const DeviceId &deviceId, const QHostAddress &clientAddress) override;
    void releaseChannel(MqttChannel* channel) override;

    MqttClient* createInternalClient(const DeviceId &deviceId) override;

    bool available() const override;
    bool enabled() const override;
    void setEnabled(bool enabled) override;

private slots:
    void onClientConnected(const QString &clientId);
    void onClientDisconnected(const QString &clientId);
    void onPublishReceived(const QString &clientId, const QString &topic, const QByteArray &payload);
    void onPluginPublished(const QString &topic, const QByteArray &payload);

private:
    MqttBroker* m_broker = nullptr;

    QHash<QString, MqttChannel*> m_createdChannels;
};

}

#endif // MQTTPROVIDERIMPLEMENTATION_H
