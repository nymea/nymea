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
