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
#include "hardware/radio433.h"
#include "types/paramtype.h"

#include <QDebug>
#include <QStringList>


DeviceClassId conradRemoteId = DeviceClassId("17cd2492-28ab-4827-ba6e-5ef35be23f1b");
EventTypeId conradRemoteButtonEventTypeId = EventTypeId("1f4050f5-4c90-4799-8d6d-e4069f3a2519");

DevicePluginConrad::DevicePluginConrad()
{
}

DeviceManager::HardwareResources DevicePluginConrad::requiredHardware() const
{
    return DeviceManager::HardwareResourceRadio433;
}

QPair<DeviceManager::DeviceError, QString> DevicePluginConrad::executeAction(Device *device, const Action &action)
{

    QList<int> rawData;
    QByteArray binCode;

    return report();
}

void DevicePluginConrad::radioData(QList<int> rawData)
{
    // filter right here a wrong signal length
    if(rawData.length() != 65){
        return;
    }

    int delay = rawData.first()/10;
    QByteArray binCode;
    
    // =======================================
    // average 314
    if(delay > 690 && delay < 750){
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

    qDebug() << "CONRAD plugin understands this protocol: " << binCode;





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
