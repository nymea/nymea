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

#ifndef DEVICEPLUGINLGSMARTTV_H
#define DEVICEPLUGINLGSMARTTV_H

#include "plugin/deviceplugin.h"
#include "tvdiscovery.h"

class DevicePluginLgSmartTv : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginlgsmarttv.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginLgSmartTv();

    TvDiscovery *m_discovery;

    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

    void deviceRemoved(Device *device) override;

    void guhTimer() override;

    QHash<TvDevice*, Device*> m_tvList;

private slots:
    void discoveryDone(QList<TvDevice *> tvList);
    void pairingFinished(const bool &success);
    void sendingCommandFinished(const bool &success, const ActionId &actionId);
    void statusChanged();

public slots:


};

#endif // DEVICEPLUGINLGSMARTTV_H
