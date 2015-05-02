/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef DEVICEPLUGINMOCK_H
#define DEVICEPLUGINMOCK_H

#include "plugin/deviceplugin.h"

#include <QProcess>

class HttpDaemon;

class DevicePluginMock : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginmock.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginMock();
    ~DevicePluginMock();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;

    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

    void startMonitoringAutoDevices() override;

    QList<ParamType> configurationDescription() const override;

public slots:
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private slots:
    void setState(const StateTypeId &stateTypeId, const QVariant &value);
    void triggerEvent(const EventTypeId &id);
    void emitDevicesDiscovered();
    void emitDeviceSetupFinished();
    void emitActionExecuted();

private:
    QHash<Device*, HttpDaemon*> m_daemons;
    QList<Device*> m_asyncSetupDevices;
    QList<QPair<Action, Device*> > m_asyncActions;

    int m_discoveredDeviceCount;
};

#endif // DEVICEPLUGINMOCK_H
