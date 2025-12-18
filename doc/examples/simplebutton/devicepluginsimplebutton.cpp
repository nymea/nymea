// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "devicepluginsimplebutton.h"
#include "plugininfo.h"

DevicePluginSimpleButton::DevicePluginSimpleButton() {}

void DevicePluginSimpleButton::init()
{
    // Initialize/create objects
}

void DevicePluginSimpleButton::startMonitoringAutoDevices()
{
    // Start seaching for devices which can be discovered and added automatically
}

void DevicePluginSimpleButton::postSetupDevice(Device *device)
{
    qCDebug(dcSimpleButton()) << "Post setup device" << device->name() << device->params();

    // This method will be called once the setup for device is finished
}

void DevicePluginSimpleButton::deviceRemoved(Device *device)
{
    qCDebug(dcSimpleButton()) << "Remove device" << device->name() << device->params();

    // Clean up all data related to this device
}

DeviceManager::DeviceSetupStatus DevicePluginSimpleButton::setupDevice(Device *device)
{
    qCDebug(dcSimpleButton()) << "Setup device" << device->name() << device->params();

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginSimpleButton::executeAction(Device *device, const Action &action)
{
    qCDebug(dcSimpleButton()) << "Executing action for device" << device->name() << action.actionTypeId().toString() << action.params();

    // Check the device class
    if (device->deviceClassId() == simplebuttonDeviceClassId) {
        // Check the action type
        if (action.actionTypeId() == simplebuttonPressActionTypeId) {
            // Emit the pressed event on button press
            qCDebug(dcSimpleButton()) << "Emit event pressed for simple button" << device->name();
            emitEvent(Event(simplebuttonPressedEventTypeId, device->id()));
            return DeviceManager::DeviceErrorNoError;
        }

        // Unhandled action type
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    // Unhandled device type
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}
