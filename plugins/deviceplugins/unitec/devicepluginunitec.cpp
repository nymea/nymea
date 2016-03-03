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
    \page unitec.html
    \title Unitec

    \ingroup plugins
    \ingroup rf433

    This plugin allows to controll RF 433 MHz actors an receive remote signals from \l{http://www.unitec-elektro.de}{Unitec}
    devices.

    The unitec socket units have a learn function. If you plug in the switch, a red light will start to blink. This means
    the socket is in the learning mode. Now you can add a Unitec switch (48111) to guh with your desired Channel (A,B,C or D).
    In order to pair the socket you just have to press the power ON, and the switch has to be in the pairing mode.
    If the pairing was successfull, the switch will turn on. If the switches will be removed from the socket or there will
    be a power breakdown, the switch has to be re-paired. The device can not remember the teached channel.

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

    \quotefile plugins/deviceplugins/unitec/devicepluginunitec.json
*/

#include "devicepluginunitec.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QStringList>

DevicePluginUnitec::DevicePluginUnitec()
{
}

DeviceManager::HardwareResources DevicePluginUnitec::requiredHardware() const
{
    return DeviceManager::HardwareResourceRadio433;
}

DeviceManager::DeviceSetupStatus DevicePluginUnitec::setupDevice(Device *device)
{
    if (device->deviceClassId() != switchDeviceClassId) {
        return DeviceManager::DeviceSetupStatusFailure;
    }

    foreach (Device* d, myDevices()) {
        if (d->paramValue("Channel").toString() == device->paramValue("Channel").toString()) {
            qCWarning(dcUnitec) << "Unitec switch with channel " << device->paramValue("Channel").toString() << "already added.";
            return DeviceManager::DeviceSetupStatusFailure;
        }
    }

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::DeviceError DevicePluginUnitec::executeAction(Device *device, const Action &action)
{   
    QList<int> rawData;
    QByteArray binCode;

    if (action.actionTypeId() != powerActionTypeId) {
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    // Bin codes for buttons
    if (device->paramValue("Channel").toString() == "A" && action.param("power").value().toBool() == true) {
        binCode.append("111011000100111010111111");
    } else if (device->paramValue("Channel").toString() == "A" && action.param("power").value().toBool() == false) {
        binCode.append("111001100110100001011111");
    } else if (device->paramValue("Channel").toString() == "B" && action.param("power").value().toBool() == true) {
        binCode.append("111011000100111010111011");
    } else if (device->paramValue("Channel").toString() == "B" && action.param("power").value().toBool() == false) {
        binCode.append("111000111001100111101011");
    } else if (device->paramValue("Channel").toString() == "C" && action.param("power").value().toBool() == true) {
        binCode.append("111000000011011111000011");
    } else if (device->paramValue("Channel").toString() == "C" && action.param("power").value().toBool() == false) {
        binCode.append("111001100110100001010011");
    } else if (device->paramValue("Channel").toString() == "D" && action.param("power").value().toBool() == true) {
        binCode.append("111001100110100001011101");
    } else if (device->paramValue("Channel").toString() == "D" && action.param("power").value().toBool() == false) {
        binCode.append("111000000011011111001101");
    }

    // =======================================
    //create rawData timings list
    int delay = 500;

    // add sync code
    rawData.append(6);
    rawData.append(14);

    // add the code
    foreach (QChar c, binCode) {
        if(c == '0'){
            rawData.append(2);
            rawData.append(1);
        }else{
            rawData.append(1);
            rawData.append(2);
        }
    }

    // =======================================
    // send data to hardware resource
    if(transmitData(delay, rawData)){
        qCDebug(dcUnitec) << "transmitted" << pluginName() << device->name() << "power: " << action.param("power").value().toBool();
        return DeviceManager::DeviceErrorNoError;
    }else{
        qCWarning(dcUnitec) << "could not transmitt" << pluginName() << device->name() << "power: " << action.param("power").value().toBool();
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }
}
