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
    \page conrad.html
    \title Conrad

    \ingroup plugins
    \ingroup rf433

    This plugin allows to controll RF 433 MHz actors an receive remote signals from \l{http://www.conrad.at}{Conrad}
    devices.

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

    \quotefile plugins/deviceplugins/conrad/devicepluginconrad.json
*/

#include "devicepluginconrad.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QStringList>


DevicePluginConrad::DevicePluginConrad()
{
}

DeviceManager::HardwareResources DevicePluginConrad::requiredHardware() const
{
    return DeviceManager::HardwareResourceRadio433;
}

DeviceManager::DeviceSetupStatus DevicePluginConrad::setupDevice(Device *device)
{
    if (device->deviceClassId() == conradShutterDeviceClassId)
        return DeviceManager::DeviceSetupStatusSuccess;

    return DeviceManager::DeviceSetupStatusFailure;
}

DeviceManager::DeviceError DevicePluginConrad::executeAction(Device *device, const Action &action)
{

    QList<int> rawData;
    QByteArray binCode;

    int repetitions = 10;

    if (action.actionTypeId() == upActionTypeId) {
        binCode = "10101000";
    } else if (action.actionTypeId() == downActionTypeId) {
        binCode = "10100000";
    } else if (action.actionTypeId() == syncActionTypeId) {
        binCode = "10100000";
        repetitions = 20;
    } else {
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    // append ID
    binCode.append("100101010110011000000001");

    //QByteArray remoteId = "100101010110011000000001";
    //    QByteArray motionDetectorId = "100100100101101101101010";
    //QByteArray wallSwitchId = "000001001101000010110110";
    //    QByteArray randomID     = "100010101010111010101010";



    // =======================================
    //create rawData timings list
    int delay = 650;

    // sync signal
    rawData.append(1);
    rawData.append(10);

    // add the code
    foreach (QChar c, binCode) {
        if(c == '0'){
            rawData.append(1);
            rawData.append(2);
        }
        if(c == '1'){
            rawData.append(2);
            rawData.append(1);
        }
    }

    // =======================================
    // send data to driver
    if(transmitData(delay, rawData, repetitions)){
        qCDebug(dcConrad) << "Transmitted successfully" << device->name() << action.actionTypeId();
        return DeviceManager::DeviceErrorNoError;
    }else{
        qCWarning(dcConrad) << "Could not transmitt" << pluginName() << device->name() << action.actionTypeId();
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }
}

void DevicePluginConrad::radioData(const QList<int> &rawData)
{
    // filter right here a wrong signal length
    if(rawData.length() != 65){
        return;
    }

    qCDebug(dcConrad) << rawData;

    int delay = rawData.first()/10;
    QByteArray binCode;

    // =======================================
    // average 650
    if(delay > 600 && delay < 750){
        // go trough all 64 timings (without sync signal)
        for(int i = 1; i <= 64; i+=2 ){
            int div;
            int divNext;

            // if short
            if(rawData.at(i) <= 900){
                div = 1;
            }else{
                div = 2;
            }
            // if long
            if(rawData.at(i+1) < 900){
                divNext = 1;
            }else{
                divNext = 2;
            }

            //              _
            // if we have  | |__ = 0 -> in 4 delays => 100
            //              __
            // if we have  |  |_ = 1 -> in 4 delays => 110

            if(div == 1 && divNext == 2){
                binCode.append('0');
            }else if(div == 2 && divNext == 1){
                binCode.append('1');
            }else{
                return;
            }
        }
    }else{
        return;
    }

    qCDebug(dcConrad) << binCode.left(binCode.length() - 24) << "  ID = " << binCode.right(24);
}
