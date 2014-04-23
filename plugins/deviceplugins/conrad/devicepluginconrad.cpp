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

#include <QDebug>
#include <QStringList>


VendorId conradVendorId = VendorId("986cf06f-3ef1-4271-b2a3-2cc277ebecb6");
DeviceClassId conradRemoteId = DeviceClassId("17cd2492-28ab-4827-ba6e-5ef35be23f1b");
EventTypeId conradRemoteButtonEventTypeId = EventTypeId("1f4050f5-4c90-4799-8d6d-e4069f3a2519");

DevicePluginConrad::DevicePluginConrad()
{
}

QList<Vendor> DevicePluginConrad::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor conrad(conradVendorId, "Conrad Electronic SE");
    ret.append(conrad);
    return ret;
}

QList<DeviceClass> DevicePluginConrad::supportedDevices() const
{
    // TODO: load list from config with static uuid
    QList<DeviceClass> ret;

    // =======================================
    // Remote
    DeviceClass deviceClassConradRemote(pluginId(), conradVendorId, conradRemoteId);
    deviceClassConradRemote.setName("Conrad Remote");

    // Params
    QList<ParamType> deviceParamsRemote;

    QVariantList deviceParamRemote;
    QVariantMap nameParam;
    nameParam.insert("name", "name");
    nameParam.insert("type", "string");
    deviceParamRemote.append(nameParam);

    // Events
    QList<EventType> buttonEvents;

    QVariantList paramsRemote;
    QVariantMap paramButton;
    paramButton.insert("name", "button");
    paramButton.insert("type", "int");
    paramsRemote.append(paramButton);

    QVariantMap paramGroup;
    paramGroup.insert("name", "group");
    paramGroup.insert("type", "int");
    paramsRemote.append(paramGroup);

    QVariantMap paramPower;
    paramPower.insert("name", "power");
    paramPower.insert("type", "bool");
    paramsRemote.append(paramPower);

    EventType buttonEvent(conradRemoteButtonEventTypeId);
    buttonEvent.setName("Button Pressed");
    buttonEvent.setParameters(paramsRemote);
    buttonEvents.append(buttonEvent);

    deviceClassConradRemote.setParams(deviceParamsRemote);
    deviceClassConradRemote.setEvents(buttonEvents);
    ret.append(deviceClassConradRemote);

    return ret;
}

DeviceManager::HardwareResources DevicePluginConrad::requiredHardware() const
{
    return DeviceManager::HardwareResourceRadio433;
}

QString DevicePluginConrad::pluginName() const
{
    return "Conrad";
}

PluginId DevicePluginConrad::pluginId() const
{
    return PluginId("1fd1a076-f229-4ec6-b501-48ddd15935e4");
}

DeviceManager::DeviceError DevicePluginConrad::executeAction(Device *device, const Action &action)
{

    QList<int> rawData;
    QByteArray binCode;

    return DeviceManager::DeviceErrorNoError;
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
