/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

/*!
    \page conrad.html
    \title Conrad

    \ingroup plugins
    \ingroup rf433

    This plugin allows to controll RF 433 MHz actors an receive remote signals from Conrad
    devices (\l{http://www.conrad.at}).

    Following devices are supported:

    \chapter Supported devices
        \section1 Actors
            \table
            \header
                \li Model
                \li Device Type
            \row
                \li
                \li
            \endtable

        \section1 Remotes
            \table
            \header
                \li Model
                \li Device Type
            \row
                \li
                \li
            \endtable
  */

#include "devicepluginconrad.h"

#include "devicemanager.h"
#include "plugin/device.h"
#include "types/paramtype.h"

#include <QDebug>
#include <QStringList>


DevicePluginConrad::DevicePluginConrad()
{
}

DeviceManager::HardwareResources DevicePluginConrad::requiredHardware() const
{
    return DeviceManager::HardwareResourceRadio433;
}

DeviceManager::DeviceError DevicePluginConrad::executeAction(Device *device, const Action &action)
{
    QList<int> rawData;
    QByteArray binCode;

    if(action.param("power").value().toBool()){
        binCode = "10010011";
    }else{
        binCode = "10100011";
    }

    // append ID
    QByteArray remoteId = "100101010110011000000001";
    QByteArray motionDetectorId = "100100100101101101101010";
    QByteArray wallSwitchId = "000001001101000010110110";
    QByteArray randomID     = "100010101010111010101010";

    binCode.append(wallSwitchId);



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
    if(transmitData(delay, rawData)){
        qDebug() << "action" << pluginName() << device->name() << "power: " << action.param("power").value().toBool();
        return DeviceManager::DeviceErrorNoError;
    }else{
        qDebug() << "could not transmitt" << pluginName() << device->name() << "power: " << action.param("power").value().toBool();
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }
}

void DevicePluginConrad::radioData(const QList<int> &rawData)
{
    // filter right here a wrong signal length
    if(rawData.length() != 65){
        return;
    }

    // qDebug() << rawData;

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

    qDebug() << "CONRAD: " << binCode.left(binCode.length() - 24) << "  ID = " << binCode.right(24);

    //    // FIXME: find a better way to get to the remote DeviceClass
    //    DeviceClass deviceClass = supportedDevices().first();
    //    foreach (const EventType &eventType, deviceClass.events()) {
    //        if (eventType.name() == buttonCode) {
    //            qDebug() << "emit event " << pluginName() << familyCode << eventType.name() << power;
    //            Event event = Event(eventType.id(), device->id(), params);
    //            emit emitEvent(event);
    //            return;
    //        }
    //    }
}
