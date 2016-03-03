/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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
    \page genericelements.html
    \title Generic elements

    \ingroup plugins
    \ingroup services

    The generic elements plugin allows you create virtual buttons, which can be connected with a rule. This gives you
    the possibility to execute multiple \l{Action}{Actions} with one signal. Without a rule this generic elements are
    useless.

    \chapter Toggle Button
    With the "Toggle Button" \l{DeviceClass} you can create a button with one \l{Action} \unicode{0x2192} toggle. In the \tt state \l{State} you can find out,
    what happens if the button will be pressed. The states can be true or false.

    \chapter Button
    With the "Button" \l{DeviceClass} you can create a button with one \l{Action} \unicode{0x2192} press. This button just creates one \l{Event}.

    \chapter ON/OFF Button
    With the "ON/OFF Button" \l{DeviceClass} you create a button pair with the \l{Action}{Actions} \unicode{0x2192} ON and OFF.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \note If a \l{StateType} has the parameter \tt{"writable": {...}}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/genericelements/deviceplugingenericelements.json
*/


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
        return DeviceManager::DeviceSetupStatusSuccess;
    }
    // Button
    if (device->deviceClassId() == buttonDeviceClassId) {
        return DeviceManager::DeviceSetupStatusSuccess;
    }
    // ON/OFF Button
    if (device->deviceClassId() == onOffButtonDeviceClassId) {
        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}

DeviceManager::DeviceError DevicePluginGenericElements::executeAction(Device *device, const Action &action)
{
    // Toggle Button
    if (device->deviceClassId() == toggleButtonDeviceClassId ) {
        if (action.actionTypeId() == stateActionTypeId) {
            device->setStateValue(stateStateTypeId, !device->stateValue(stateStateTypeId).toBool());
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
