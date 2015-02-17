/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
#include "wemoswitch.h"

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

    void guhTimer() override;
    void upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList) override;
    void upnpNotifyReceived(const QByteArray &notifyData);

private:
    QHash<WemoSwitch*, Device*> m_wemoSwitches;
    bool verifyExistingDevices(UpnpDeviceDescriptor deviceDescriptor);

private slots:
    void wemoSwitchStateChanged();
    void setPowerFinished(const bool &succeeded, const ActionId &actionId);


public slots:


};

#endif // DEVICEPLUGINWEMO_H
