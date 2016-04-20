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

/*!
    \page lirc.html
    \title LIRC
    \brief Plugin for the LIRC infrared daemon.

    \ingroup plugins
    \ingroup guh-plugins

    This plugin allows to interact with \l{http://www.lirc.org/}{LIRC} daemon and controll commonly used remote controls.
    If lircd (LIRC daemon) is configured on your system, guh will connect to the lirc daemon and all configured remote
    controls of lircd will appear in guh.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/lircd/devicepluginlircd.json
*/

#include "devicepluginlircd.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"
#include "lircdclient.h"

#include <QDebug>
#include <QStringList>

DeviceClassId lircdDeviceClassId = DeviceClassId("5c2bc4cd-ba6c-4052-b6cd-1db83323ea22");
EventTypeId LircKeypressEventTypeId = EventTypeId("8711471a-fa0e-410b-b174-dfc3d2aeffb1");

DevicePluginLircd::DevicePluginLircd()
{
    m_lircClient = new LircClient(this);

    //m_lircClient->connect();
    connect(m_lircClient, &LircClient::buttonPressed, this, &DevicePluginLircd::buttonPressed);
}

DeviceManager::HardwareResources DevicePluginLircd::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

void DevicePluginLircd::buttonPressed(const QString &remoteName, const QString &buttonName, int repeat)
{
    Device *remote = nullptr;
    QList<Device*> configuredRemotes = deviceManager()->findConfiguredDevices(lircdDeviceClassId);
    foreach (Device *device, configuredRemotes) {
        if (device->paramValue("remoteName").toString() == remoteName) {
            remote = device;
            break;
        }
    }
    if (!remote) {
        qCWarning(dcLircd) << "Unhandled remote" << remoteName << buttonName;
        return;
    }

    qCDebug(dcLircd) << "found remote" << remoteName << supportedDevices().first().eventTypes().count();
    ParamList params;
    Param buttonParam("button", buttonName);
    params.append(buttonParam);
    Param repeatParam("repeat", repeat);
    params.append(repeatParam);
    Event event(LircKeypressEventTypeId, remote->id(), params);
    emitEvent(event);
}

//QVariantMap DevicePluginLircd::configuration() const
//{
//    return m_config;
//}

//void DevicePluginLircd::setConfiguration(const QVariantMap &configuration)
//{
//    m_config = configuration;
//}
