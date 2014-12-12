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

#ifndef DEVICEPLUGINCOMMANDLAUNCHER_H
#define DEVICEPLUGINCOMMANDLAUNCHER_H

#include "plugin/deviceplugin.h"
#include <QProcess>
#include <QFileInfo>

class DevicePluginCommandLauncher : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "deviceplugincommandlauncher.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginCommandLauncher();

    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

    void deviceRemoved(Device *device) override;

private:
    QHash<QProcess*,Device*> m_scripts;
    QHash<QProcess*,Device*> m_applications;

    // Hashes for action execution4
    QHash<QProcess*,ActionId> m_startingScripts;
    QHash<QProcess*,ActionId> m_startingApplications;
    QHash<QProcess*,ActionId> m_killingScripts;
    QHash<QProcess*,ActionId> m_killingApplications;

private slots:
    void scriptStateChanged(QProcess::ProcessState state);
    void scriptFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void applicationStateChanged(QProcess::ProcessState state);
    void applicationFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // DEVICEPLUGINCOMMANDLAUNCHER_H
