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

#include "deviceplugingenericelements.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>

DevicePluginGenericElements::DevicePluginGenericElements()
{
}

DeviceManager::HardwareResources DevicePluginGenericElements::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceSetupStatus DevicePluginGenericElements::setupDevice(Device *device)
{
    // Toggle Button
    if (device->deviceClassId() == toggleButtonDeviceClassId) {
        device->setName(device->paramValue("name").toString() +" (Toggle Button)");
        return DeviceManager::DeviceSetupStatusSuccess;
    }
    // Button
    if (device->deviceClassId() == buttonDeviceClassId) {
        device->setName(device->paramValue("name").toString() +" (Button)");
        return DeviceManager::DeviceSetupStatusSuccess;
    }
    // ON/OFF Button
    if (device->deviceClassId() == onOffButtonDeviceClassId) {
        device->setName(device->paramValue("name").toString() +" (ON/OFF Button)");
        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}

DeviceManager::DeviceError DevicePluginGenericElements::executeAction(Device *device, const Action &action)
{
    // Toggle Button
    if (device->deviceClassId() == toggleButtonDeviceClassId ) {
        if (action.actionTypeId() == toggleButtonToggleActionTypeId) {
            device->setStateValue(toggleButtonStatusStateTypeId, !device->stateValue(toggleButtonStatusStateTypeId).toBool());
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    // Button
    if (device->deviceClassId() == buttonDeviceClassId ) {
        if (action.actionTypeId() == buttonPressActionTypeId) {
            emit emitEvent(Event(buttonPressedEventTypeId, device->id()));
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    // ON/OFF Button
    if (device->deviceClassId() == onOffButtonDeviceClassId ) {
        if (action.actionTypeId() == onOffButtonOnActionTypeId) {
            emit emitEvent(Event(onOffButtonOnEventTypeId, device->id()));
            return DeviceManager::DeviceErrorNoError;
        }
        if (action.actionTypeId() == onOffButtonOffActionTypeId) {
            emit emitEvent(Event(onOffButtonOffEventTypeId, device->id()));
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}
