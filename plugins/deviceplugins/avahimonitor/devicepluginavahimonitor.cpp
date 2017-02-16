/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \page avahimonitor.html
    \title Avahi Monitor
    \brief Plugin to monitor zeroconf devices in the local network.

    \ingroup plugins
    \ingroup guh-plugins-maker

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/avahimonitor/devicepluginavahimonitor.json
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

void DevicePluginAvahiMonitor::init()
{
    connect(avahiServiceBrowser(), SIGNAL(serviceEntryAdded(AvahiServiceEntry)), this, SLOT(onServiceEntryAdded(AvahiServiceEntry)));
    connect(avahiServiceBrowser(), SIGNAL(serviceEntryRemoved(AvahiServiceEntry)), this, SLOT(onServiceEntryRemoved(AvahiServiceEntry)));
}

DeviceManager::DeviceSetupStatus DevicePluginAvahiMonitor::setupDevice(Device *device)
{
    qCDebug(dcAvahiMonitor()) << "Setup" << device->name() << device->params();

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
        ParamList params;
        params.append(Param(serviceParamTypeId, service.name()));
        params.append(Param(hostNameParamTypeId, service.hostName()));
        deviceDescriptor.setParams(params);
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
    foreach (Device *device, myDevices()) {
        if (device->paramValue(serviceParamTypeId).toString() == serviceEntry.name()) {
            device->setStateValue(onlineStateTypeId, true);
        }
    }
}

void DevicePluginAvahiMonitor::onServiceEntryRemoved(const AvahiServiceEntry &serviceEntry)
{
    foreach (Device *device, myDevices()) {
        if (device->paramValue(serviceParamTypeId).toString() == serviceEntry.name()) {
            device->setStateValue(onlineStateTypeId, false);
        }
    }
}
