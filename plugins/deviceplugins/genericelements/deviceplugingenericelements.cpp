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

#include <QDebug>

DeviceClassId toggleButtonDeviceClassId = DeviceClassId("c0f511f9-70f5-499b-bd70-2c0e9ddd68c4");
ActionTypeId toggleButtonToggleActionTypeId = ActionTypeId("47bdc15a-b393-4dc2-801b-845420cdfda3");
StateTypeId toggleButtonStatusStateTypeId = StateTypeId("b5e90567-54aa-49bd-a78a-3c19fb38aaf5");

DeviceClassId buttonDeviceClassId = DeviceClassId("820b2f2d-0d92-48c8-8fd4-f94ce8fc4103");
ActionTypeId buttonPressActionTypeId = ActionTypeId("01f38af1-b2ab-4ec3-844e-ef52f0f229a9");
EventTypeId buttonPressedEventTypeId = EventTypeId("effdbc2d-e467-4b0b-80a9-9dda251bfa5c");

DeviceClassId onOffButtonDeviceClassId = DeviceClassId("430d188c-476d-4825-a9bd-86dfa3094b56");
ActionTypeId onOffButtonOnActionTypeId = ActionTypeId("892596d2-0863-4807-97da-469b9f7003f2");
ActionTypeId onOffButtonOffActionTypeId = ActionTypeId("a8d64050-0b58-4ccf-b052-77ce2b7368ad");
EventTypeId onOffButtonOnEventTypeId = EventTypeId("4eeba6a2-e4c7-4a2e-8360-2797d98114e6");
EventTypeId onOffButtonOffEventTypeId = EventTypeId("b636c5f3-2eb0-4682-96d4-88a4aa9d2c12");

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
