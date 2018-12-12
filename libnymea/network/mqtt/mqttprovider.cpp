/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class MqttProvider
    \brief Provides means of communicating to devices via MQTT

    \ingroup hardware
    \inmodule libnymea

    The MQTT provider class offers 2 different ways to establish a MQTT connection to a MQTT enabled IoT device:
    For one it supports establishing a secure channel between the device and the plugin, using nymea's internal MQTT
    broker. The second approach is to provide a generic MQTT client object connected to nymea's internal MQTT broker.

    Note, if you whish to establish a MQTT connection to an external MQTT broker, you can just create your own MqttClient
    and connect to wherever you need to. The MqttProvider hardware resource only manages connections to the nymea internal
    MQTT broker.

    If using MqttProvider hardware resource, the MQTT broker will be automatically configured with appropriate policies.

    \chapter Example 1

    Establishing a secure MQTT directly between plugin and the MQTT device:

    \tt devicepluginexample.h

    \code
    #include "network/mqtt/mqttprovider.h"
    #include "network/mqtt/mqttchannel.h"

    class DevicePluginExample : public DevicePlugin
    {
    ...
    public:
        DeviceManager::DeviceSetupStatus setupDevice(Device* device) override;
        void deviceRemoved(Device* device) override;

    private slots:
        void onDevicePublishReceived(MqttChannel* channel, const QString &topic, const QByteArray &payload);

    ...

    };

    \endcode

    \tt devicepluginexample.cpp

    \code
        DeviceManager::DeviceSetupStatus DevicePluginExample::setupDevice(Device *device) {
            MqttChannel *channel = hardware()->mqttProvider()->createChannel(device->id(), "192.168.0.100");

            // Provision credentials to device:
            QString clientId = channel->clientId();
            QString username = channel->username();
            QString password = channel->password();
            hardwareManager()->networkManager()->post("https://192.168.0.100/setup?clientId=" + username + "&username=" + username);

            // connect to publishes in this channel
            connect(channel, &MqttChannel::publishReceived, this, onDevicePublishReceived);

            // publish to the channel
            channel->publish(channel->prefix() + "/my/test/topic", "Hello world");
        }

        void DevicePluginExample::deviceRemoved(Device* device) {
            hardware()->mqttProvider()->releaseChannel(channel);
        }

        void DevicePluginExample::onDevicePublishReceibed(MqttChannel *channel, const QString &topic, const QByteArray &payload) {
            qCDebug(dcExample()) << "Publish received from:" << channel->clientId() << topic << payload;
        }
    \endcode

    \chapter Example 2

    Obtaining a MQTT client connected to the nymea internal MQTT broker:

    \tt devicepluginexample.h

    \code
    #include "network/mqtt/mqttprovider.h"
    #include "<nymea-mqtt/mqttclient.h>

    class DevicePluginExample : public DevicePlugin
    {
    ...
    public:
        DeviceManager::DeviceSetupStatus setupDevice(Device* device) override;
        void deviceRemoved(Device* device) override;

    private slots:
        void onPublishReceived(const QString &topic, const QByteArray &payload, bool retained);

    ...

    };

    \endcode

    \tt devicepluginexample.cpp

    \code
        DeviceManager::DeviceSetupStatus DevicePluginExample::setupDevice(Device *device) {
            MqttClient *client = hardware()->mqttProvider()->createInternalClient(device->id());

            // subscribe to topics
            client->subscribe("/my/test/topic", Mqtt::QuS1);

            // connect to publishes in this channel
            connect(channel, &MqttClient::publishReceived, this, onPublishReceived);

            // publish to the broker
            client->publish("/my/test/topic", "Hello world");
        }

        void DevicePluginExample::deviceRemoved(Device* device) {
            hardware()->mqttProvider()->releaseChannel(channel);
        }

        void DevicePluginExample::onPublishRecei(const QString &topic, const QByteArray &payload, bool retained) {
            qCDebug(dcExample()) << "Publish received from:" << channel->clientId() << topic << payload << retained;
        }
    \endcode


    \sa HardwareResource, HardwareManager::mqttProvider()
*/

/*! \fn MqttChannel *MqttProvider::createChannel(const DeviceId &deviceId, const QHostAddress &clientAddress);
    Creates a new MQTT channel on the internal broker. The returned channel will have the required details for the
    client device to connect to the broker. A temporaray clientId/user/password combination will be created and
    clients connecting to the broker with those credentials will have access to subscribe and post to # within the
    given topic prefix.

    \sa releaseChannel(MqttChannel *channel)
*/

/*! \fn void MqttProvider::releaseChannel(MqttChannel *channel)
    Releases the given MQTT channel. This means the broker will disconnect any clients connected on this channel
    and remove the policy from the broker. The channel object will be destroyed and must not be accessed any more
    after calling this function.

    \sa createChannel(const DeviceId &deviceId, const QHostAddress &clientAddress)
*/

/*! \fn MqttClient *MqttProvider::createInternalClient(const DeviceId &deviceId);
    Creates a temporary policy on the internal MQTT broker to allow this client connecting. If that is successful
    it will create a new MqttClient object and connect it to the broker.

    The client will not be subscribed to any topics yet. See the nymea-mqtt documentation for information on the MqttClient API.

    The client will be owned by the MqttProvider and cleaned up when the system goes down, however, the user is free to
    delete the client at any point. The policy attached to it will be removed from the MQTT broker.
*/


#include "mqttprovider.h"
#include "mqttchannel.h"

MqttProvider::MqttProvider(QObject *parent) : HardwareResource("MQTT", parent)
{

}
