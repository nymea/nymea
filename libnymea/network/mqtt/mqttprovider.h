#ifndef MQTTPROVIDER_H
#define MQTTPROVIDER_H

#include <QObject>
#include <QHostAddress>

#include "typeutils.h"
#include "hardwareresource.h"

class MqttChannel;

class MqttProvider : public HardwareResource
{
    Q_OBJECT
public:
    explicit MqttProvider(QObject *parent = nullptr);

    virtual MqttChannel* createChannel(const DeviceId &deviceId, const QHostAddress &clientAddress) = 0;
    virtual void releaseChannel(MqttChannel *channel) = 0;
};

#endif // MQTTPROVIDER_H
