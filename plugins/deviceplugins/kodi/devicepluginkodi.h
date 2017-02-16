/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef DEVICEPLUGINKODI_H
#define DEVICEPLUGINKODI_H

#include "plugin/deviceplugin.h"
#include "kodi.h"

#include <QHash>
#include <QDebug>
#include <QTcpSocket>

class DevicePluginKodi : public DevicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginkodi.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginKodi();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;
    void guhTimer() override;

    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    void upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList) override;

    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private:
    QHash<Kodi *, Device *> m_kodis;
    QList<Kodi *> m_asyncSetups;


private slots:
    void onConnectionChanged();
    void onStateChanged();
    void onActionExecuted(const ActionId &actionId, const bool &success);
    void versionDataReceived(const QVariantMap &data);
    void onSetupFinished(const QVariantMap &data);

    void onPlayerPlay();
    void onPlayerPause();
    void onPlayerStop();
};

#endif // DEVICEPLUGINKODI_H
