/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
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

#ifndef CLOUDNOTIFICATIONS_H
#define CLOUDNOTIFICATIONS_H

#include "devices/deviceplugin.h"
#include "awsconnector.h"

class CloudNotifications : public DevicePlugin
{
    Q_OBJECT

//    Q_PLUGIN_METADATA(IID "io.nymea.DevicePlugin" FILE "deviceplugincloudnotifications.json")
    Q_INTERFACES(DevicePlugin)

public:
    CloudNotifications(AWSConnector *awsConnector, QObject* parent = nullptr);

    PluginMetadata metaData() const;

    Device::DeviceSetupStatus setupDevice(Device *device) override;
    void startMonitoringAutoDevices() override;
    Device::DeviceError executeAction(Device *device, const Action &action) override;

private slots:
    void pushNotificationEndpointsUpdated(const QList<AWSConnector::PushNotificationsEndpoint> &endpoints);
    void pushNotificationEndpointAdded(const AWSConnector::PushNotificationsEndpoint &endpoint);
    void pushNotificationSent(int id, int status);

private:
    AWSConnector *m_awsConnector = nullptr;
    QHash<int, ActionId> m_pendingPushNotifications;
};

#endif // CLOUDNOTIFICATIONS_H
