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
#include "wemodiscovery.h"

class DevicePluginWemo : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginwemo.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginWemo();

    QList<Vendor> supportedVendors() const override;
    QList<DeviceClass> supportedDevices() const override;

    QPair<DeviceManager::DeviceError, QString> discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    QPair<DeviceManager::DeviceSetupStatus, QString> setupDevice(Device *device) override;
    DeviceManager::HardwareResources requiredHardware() const override;
    QPair<DeviceManager::DeviceError, QString> executeAction(Device *device, const Action &action) override;

    void deviceRemoved(Device *device) override;

    QString pluginName() const override;
    PluginId pluginId() const override;

    void guhTimer() override;

    WemoDiscovery *m_discovery;
    QHash<WemoSwitch*, Device*> m_wemoSwitches;

private slots:
    void discoveryDone(QList<WemoSwitch *> deviceList);
    void wemoSwitchStateChanged();
public slots:


};

#endif // DEVICEPLUGINWEMO_H
