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

DevicePluginGenericElements::DevicePluginGenericElements()
{
}

DeviceManager::HardwareResources DevicePluginGenericElements::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceSetupStatus DevicePluginGenericElements::setupDevice(Device *device)
{
    if (device->deviceClassId() == toggleButtonDeviceClassId) {
        device->setName(device->paramValue("name").toString() +" (Toggle Button)");
        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}

DeviceManager::DeviceError DevicePluginGenericElements::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == toggleButtonDeviceClassId
            && action.actionTypeId() == toggleButtonToggleActionTypeId) {
        bool currentStatus = device->stateValue(toggleButtonStatusStateTypeId).toBool();
        device->setStateValue(toggleButtonStatusStateTypeId, !currentStatus);
        return DeviceManager::DeviceErrorNoError;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}
