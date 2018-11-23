#ifndef MQTTCHANNEL_H
#define MQTTCHANNEL_H

#include <QObject>
#include <QHostAddress>

#include "libnymea.h"

class LIBNYMEA_EXPORT MqttChannel: public QObject
{
    Q_OBJECT
public:
    MqttChannel(QObject *parent = nullptr);
    virtual ~MqttChannel();

    virtual QString clientId() const = 0;
    virtual QString username() const = 0;
    virtual QString password() const = 0;
    virtual QHostAddress serverAddress() const = 0;
    virtual quint16 serverPort() const = 0;
    virtual QString topicPrefix() const = 0;

    virtual void publish(const QString &topic, const QByteArray &payload) = 0;

signals:
    void clientConnected(MqttChannel* channel);
    void clientDisconnected(MqttChannel* channel);
    void publishReceived(MqttChannel* channel, const QString &topic, const QByteArray &payload);
};

#endif // MQTTCHANNEL_H
