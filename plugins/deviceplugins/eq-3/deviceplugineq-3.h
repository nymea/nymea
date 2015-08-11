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

#ifndef DEVICEPLUGINEQ3_H
#define DEVICEPLUGINEQ3_H

#include "plugin/deviceplugin.h"
#include "maxcubediscovery.h"

#include <QHostAddress>

class QNetworkReply;

class DevicePluginEQ3: public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "deviceplugineq-3.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginEQ3();

    DeviceManager::HardwareResources requiredHardware() const override;

    QList<ParamType> configurationDescription() const override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;

    void startMonitoringAutoDevices() override;

    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;
    void guhTimer() override;

private:
    QList<Param> m_config;
    MaxCubeDiscovery *m_cubeDiscovery;
    QHash<MaxCube*, Device*> m_cubes;

public slots:
    DeviceManager::DeviceError executeAction(Device *device, const Action &action);

private slots:
    void cubeConnectionStatusChanged(const bool &connected);
    void discoveryDone(const QList<MaxCube *> &cubeList);
    void commandActionFinished(const bool &succeeded, const ActionId &actionId);

    void wallThermostatFound();
    void radiatorThermostatFound();

    void updateCubeConfig();
    void wallThermostatDataUpdated();
    void radiatorThermostatDataUpdated();

};

#endif // DEVICEPLUGINEQ3_H
