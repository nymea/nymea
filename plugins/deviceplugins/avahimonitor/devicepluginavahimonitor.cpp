/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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
    \page wifidetector.html
    \title WiFi Detector
    \brief Plugin to monitor devices in the local network.

    \ingroup plugins
    \ingroup guh-plugins

    This plugin allows to find and monitor network devices in your local network by using the MAC address.

    \underline{NOTE}: the application \c nmap has to be installed and guh has to run as root.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/wifidetector/devicepluginwifidetector.json
*/


#include "devicepluginavahimonitor.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QStringList>
#include <QNetworkInterface>

DevicePluginAvahiMonitor::DevicePluginAvahiMonitor()
{
}

DeviceManager::DeviceSetupStatus DevicePluginAvahiMonitor::setupDevice(Device *device)
{
    qCDebug(dcAvahiMonitor()) << "Setup" << device->name() << device->params();

    connect(avahiServiceBrowser(), SIGNAL(serviceEntryAdded(AvahiServiceEntry)), this, SLOT(onServiceEntryAdded(AvahiServiceEntry)));
    connect(avahiServiceBrowser(), SIGNAL(serviceEntryRemoved(AvahiServiceEntry)), this, SLOT(onServiceEntryRemoved(AvahiServiceEntry)));

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginAvahiMonitor::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)

    if (deviceClassId != avahiDeviceClassId)
        return DeviceManager::DeviceErrorDeviceClassNotFound;

    QList<DeviceDescriptor> deviceDescriptors;
    foreach (const AvahiServiceEntry &service, avahiServiceBrowser()->serviceEntries()) {
        DeviceDescriptor deviceDescriptor(avahiDeviceClassId, service.name(), service.hostAddress().toString());
        deviceDescriptors.append(deviceDescriptor);
    }

    emit devicesDiscovered(avahiDeviceClassId, deviceDescriptors);

    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::HardwareResources DevicePluginAvahiMonitor::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

void DevicePluginAvahiMonitor::onServiceEntryAdded(const AvahiServiceEntry &serviceEntry)
{
    Q_UNUSED(serviceEntry)
}

void DevicePluginAvahiMonitor::onServiceEntryRemoved(const AvahiServiceEntry &serviceEntry)
{
    Q_UNUSED(serviceEntry)
}
