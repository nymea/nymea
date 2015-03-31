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

#ifndef DEVICEPLUGINELGATO_H
#define DEVICEPLUGINELGATO_H

#include "plugin/deviceplugin.h"
#include "bluetooth/bluetoothlowenergydevice.h"
#include "aveabulb.h"

class DevicePluginElgato : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginelgato.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginElgato();

    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

    void bluetoothDiscoveryFinished(const QList<QBluetoothDeviceInfo> &deviceInfos);
    void deviceRemoved(Device *device) override;

private:
    QHash<AveaBulb *, Device *> m_bulbs;
    bool verifyExistingDevices(const QBluetoothDeviceInfo &deviceInfo);

private slots:
    void bulbAvailableChanged();
    void actionFinished(const ActionId actionId, const bool &success);
};

#endif // DEVICEPLUGINELGATO_H
