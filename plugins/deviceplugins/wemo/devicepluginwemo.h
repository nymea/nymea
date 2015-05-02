/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICEPLUGINWEMO_H
#define DEVICEPLUGINWEMO_H

#include "plugin/deviceplugin.h"

class DevicePluginWemo : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginwemo.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginWemo();

    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

    void deviceRemoved(Device *device) override;
    void networkManagerReplyReady(QNetworkReply *reply) override;

    void guhTimer() override;
    void upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList) override;
    void upnpNotifyReceived(const QByteArray &notifyData);

private:
    QHash<QNetworkReply *, Device *> m_refreshReplies;
    QHash<QNetworkReply *, Device *> m_setPowerReplies;
    QHash<QNetworkReply *, ActionId> m_runningActionExecutions;
    bool verifyExistingDevices(UpnpDeviceDescriptor deviceDescriptor);

    void refresh(Device* device);
    bool setPower(Device *device, const bool &power, const ActionId &actionId);

    void processRefreshData(const QByteArray &data, Device *device);
    void processSetPowerData(const QByteArray &data, Device *device, const ActionId &actionId);

};

#endif // DEVICEPLUGINWEMO_H
