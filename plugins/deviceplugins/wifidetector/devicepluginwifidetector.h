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

#ifndef DEVICEPLUGINWIFIDETECTOR_H
#define DEVICEPLUGINWIFIDETECTOR_H

#include "plugin/deviceplugin.h"

#include <QProcess>

class DevicePluginWifiDetector : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginwifidetector.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginWifiDetector();

    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    DeviceManager::HardwareResources requiredHardware() const override;

    void guhTimer() override;

private:
    QList<QProcess *> m_discoveryProcesses;
    QList<DeviceDescriptor> m_deviceDescriptors;

private slots:
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void discoveryProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // DEVICEPLUGINWIFIDETECTOR_H
