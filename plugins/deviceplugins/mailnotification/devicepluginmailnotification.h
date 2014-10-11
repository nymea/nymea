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

#ifndef DEVICEPLUGINMAILNOTIFICATION_H
#define DEVICEPLUGINMAILNOTIFICATION_H

#include "plugin/deviceplugin.h"
#include "smtpclient.h"

class DevicePluginMailNotification : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginmailnotification.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginMailNotification();
    ~DevicePluginMailNotification();

    QPair<DeviceManager::DeviceSetupStatus, QString> setupDevice(Device *device) override;
    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private:
    SmtpClient *m_smtpClient;

private slots:

public slots:


};

#endif // DEVICEPLUGINMAILNOTIFICATION_H
