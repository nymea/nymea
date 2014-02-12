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

#include "device.h"
#include "devicemanager.h"
#include "radio433.h"

#include <QDebug>
#include <QStringList>


QUuid conradRemoteId = QUuid("17cd2492-28ab-4827-ba6e-5ef35be23f1b");


DevicePluginConrad::DevicePluginConrad()
{
}

QList<DeviceClass> DevicePluginConrad::supportedDevices() const
{
    // TODO: load list from config with static uuid
    QList<DeviceClass> ret;

    // =======================================
    // Remote
    DeviceClass deviceClassConradRemote(pluginId(), conradRemoteId);
    deviceClassConradRemote.setName("Conrad Remote");

    QVariantList deviceParamsRemote;
    QVariantMap channelParam;
//    channelParam.insert("name", "channel1");
//    channelParam.insert("type", "bool");
//    deviceParamsRemote.append(channelParam);
//    channelParam.insert("name", "channel2");
//    channelParam.insert("type", "bool");
//    deviceParamsRemote.append(channelParam);
//    channelParam.insert("name", "channel3");
//    channelParam.insert("type", "bool");
//    deviceParamsRemote.append(channelParam);
//    channelParam.insert("name", "channel4");
//    channelParam.insert("type", "bool");
//    deviceParamsRemote.append(channelParam);
//    channelParam.insert("name", "channel5");
//    channelParam.insert("type", "bool");
//    deviceParamsRemote.append(channelParam);

    deviceClassConradRemote.setParams(deviceParamsRemote);
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

QUuid DevicePluginConrad::pluginId() const
{
    return QUuid("1fd1a076-f229-4ec6-b501-48ddd15935e4");
}

void DevicePluginConrad::executeAction(Device *device, const Action &action)
{

    QList<int> rawData;
    QByteArray binCode;


}

void DevicePluginConrad::radioData(QList<int> rawData)
{
    qDebug() << "!!!!!!!!!!!! called conrad radioData";
    // filter right here a wrong signal length
    if(rawData.length() != 65){
        return;
    }
    
//    QList<Device*> deviceList = deviceManager()->findConfiguredDevices(intertechnoRemote);
//    if(deviceList.isEmpty()){
//        return;
//    }

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
            if(rawData.at(i) <= 800){
                div = 1;
            }else{
                div = 2;
            }
            // if long
            if(rawData.at(i+1) < 800){
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

    // =======================================

    qDebug() << "-----------------------------------------------------------";
    qDebug() << "got conrad device...";
    qDebug() << binCode;

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
