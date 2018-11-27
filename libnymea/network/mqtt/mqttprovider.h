#ifndef MQTTPROVIDER_H
#define MQTTPROVIDER_H

#include <QObject>
#include <QHostAddress>

#include "typeutils.h"
#include "hardwareresource.h"
#include "nymea-mqtt/mqttclient.h"

class MqttChannel;

class MqttProvider : public HardwareResource
{
    Q_OBJECT
public:
    explicit MqttProvider(QObject *parent = nullptr);

    virtual MqttChannel* createChannel(const DeviceId &deviceId, const QHostAddress &clientAddress) = 0;
    virtual void releaseChannel(MqttChannel *channel) = 0;

    virtual MqttClient* createInternalClient(const DeviceId &deviceId) = 0;
};

#endif // MQTTPROVIDER_H
