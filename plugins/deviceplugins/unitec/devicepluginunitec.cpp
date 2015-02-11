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

/*!
    \page unitec.html
    \title Unitec

    \ingroup plugins
    \ingroup rf433

    This plugin allows to controll RF 433 MHz actors an receive remote signals from \l{www.unitec-elektro.de}{Unitec}
    devices.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \quotefile plugins/deviceplugins/elro/devicepluginelro.json
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

DeviceManager::DeviceError DevicePluginUnitec::executeAction(Device *device, const Action &action)
{   
    QList<int> rawData;
    QByteArray binCode;


    // sync signal (20 * 1)
    binCode.append("11111111111111111111");

    // Button ID
    if (device->paramValue("Channel").toString() == "A") {
        binCode.append("000");
    } else if (device->paramValue("Channel").toString() == "B") {
        binCode.append("100");
    } else if (device->paramValue("Channel").toString() == "C") {
        binCode.append("010");
    } else if (device->paramValue("Channel").toString() == "D") {
        binCode.append("110");
    } else if (device->paramValue("Channel").toString() == "ALL") {
        binCode.append("001");
    }

    // power state
    if (action.param("power").value().toBool() == true) {
        binCode.append("11");
    } else {
        binCode.append("01");
    }


    // =======================================
    //create rawData timings list
    int delay = 250;

    // add the code
    foreach (QChar c, binCode) {
        if(c == '0'){
            rawData.append(3);
            rawData.append(1);
        }else{
            rawData.append(1);
            rawData.append(3);
        }
    }

    qDebug() << binCode;
    qDebug() << rawData;

    // =======================================
    // send data to hardware resource
    if(transmitData(delay, rawData)){
        qDebug() << "transmitted" << pluginName() << device->name() << "power: " << action.param("power").value().toBool();
        return DeviceManager::DeviceErrorNoError;
    }else{
        qDebug() << "could not transmitt" << pluginName() << device->name() << "power: " << action.param("power").value().toBool();
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }
}
