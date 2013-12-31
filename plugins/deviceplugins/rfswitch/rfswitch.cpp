#include "rfswitch.h"

#include "device.h"
#include "devicemanager.h"
#include "radio433.h"

#include <QDebug>

QUuid mumbiRemote = QUuid("d85c1ef4-197c-4053-8e40-707aa671d302");
QUuid mumbiRfSwitch = QUuid("308ae6e6-38b3-4b3a-a513-3199da2764f8");

RfSwitch::RfSwitch()
{
}

void RfSwitch::init()
{
    connect(deviceManager()->radio433(), &Radio433::dataReceived, this, &RfSwitch::dataReceived);
}

QList<DeviceClass> RfSwitch::supportedDevices() const
{
    // TODO: load list from config with static uuid
    QList<DeviceClass> ret;

    DeviceClass deviceClassRfRemote(mumbiRemote);
    deviceClassRfRemote.setName("Mumbi Remote");
    
    QVariantList deviceParams;
    QVariantMap channelParam;
    channelParam.insert("name", "channel");
    channelParam.insert("type", "string");
    channelParam.insert("name", "channel2");
    channelParam.insert("type", "bool");
    channelParam.insert("name", "channel3");
    channelParam.insert("type", "bool");
    channelParam.insert("name", "channel4");
    channelParam.insert("type", "bool");
    channelParam.insert("name", "channel5");
    channelParam.insert("type", "bool");
    deviceParams.append(channelParam);
    
    deviceClassRfRemote.setParams(deviceParams);
    
    QList<TriggerType> buttonTriggers;
    
    QVariantList params;
    QVariantMap param;
    param.insert("name", "power");
    param.insert("type", "bool");
    params.append(param);
    
    TriggerType buttonATrigger(QUuid::createUuid());
    buttonATrigger.setName("Button A");
    buttonATrigger.setParameters(params);
    buttonTriggers.append(buttonATrigger);
    
    TriggerType buttonBTrigger(QUuid::createUuid());
    buttonBTrigger.setName("Button B");
    buttonBTrigger.setParameters(params);
    buttonTriggers.append(buttonBTrigger);
    
    TriggerType buttonCTrigger(QUuid::createUuid());
    buttonCTrigger.setName("Button C");
    buttonCTrigger.setParameters(params);
    buttonTriggers.append(buttonCTrigger);
    
    TriggerType buttonDTrigger(QUuid::createUuid());
    buttonDTrigger.setName("Button D");
    buttonDTrigger.setParameters(params);
    buttonTriggers.append(buttonDTrigger);
    
    TriggerType buttonETrigger(QUuid::createUuid());
    buttonETrigger.setName("Button E");
    buttonETrigger.setParameters(params);
    buttonTriggers.append(buttonETrigger);
    
    deviceClassRfRemote.setTriggers(buttonTriggers);
    
    ret.append(deviceClassRfRemote);


    DeviceClass deviceClassRfSwitch(mumbiRfSwitch);
    deviceClassRfSwitch.setName("Mumbi Power Switch");
    ret.append(deviceClassRfSwitch);
    
    return ret;
}

QString RfSwitch::pluginName() const
{
    return "RF Switch";
}

void RfSwitch::dataReceived(QList<int> rawData)
{    
    // filter right here a wrong signal length
    if(rawData.length() != 49){
        return;
    }
    
    int delay = rawData.first()/31;
    QByteArray binCode;
    
    // new Remote -> average 314
    if(delay > 300 && delay < 400){
        // go trough all 48 timings (without sync signal)
        for(int i = 1; i <= 48; i+=2 ){
            int div;
            int divNext;
            
            // if short
            if(rawData.at(i) < 700){
                div = 1;
            }else{
                div = 3;
            }
            // if long
            if(rawData.at(i+1) < 700){
                divNext = 1;
            }else{
                divNext = 3;
            }
            
            //              _
            // if we have  | |___ = 0 -> in 4 delays => 1000
            //                 _
            // if we have  ___| | = 1 -> in 4 delays => 0001
            
            if(div == 1 && divNext == 3){
                binCode.append('0');
            }else if(div == 3 && divNext == 1){
                binCode.append('1');
            }else{
                return;
            }
        }
    }
    qDebug() << "bincode" << binCode;


    // get the channel of the remote signal (5 channels, true=1, false=0)
    QList<bool> group;
    for(int i = 1; i < 10; i+=2){
        if(binCode.at(i-1) == '0' && binCode.at(i) == '1'){
            group << false;
        }else if(binCode.at(i-1) == '0' && binCode.at(i) == '0'){
            group << true;
        }else {
            return;
        }
    }
    //qDebug() << "group" << group;
    
    // get the button letter
    QString button;
    QByteArray buttonCode = binCode.mid(10,10);
    qDebug() << "Buttoncode -> " << buttonCode;

    if(buttonCode == "0001010101"){
        button = "A";
    }else if(buttonCode == "0100010101"){
        button = "B";
    }else if(buttonCode == "0101000101"){
        button = "C";
    }else if(buttonCode == "0101010001"){
        button = "D";
    }else if(buttonCode == "0101010100"){
        button = "E";
    }else{
        return;
    }
        
    //    // TODO: Lets assume we received group "1000"
    //    QList<bool> group;
    //    group << true << false << false << false << false;
    
    Device *device = 0;
    QList<Device*> deviceList = deviceManager()->findConfiguredDevices(mumbiRemote);
    foreach (Device *dev, deviceList) {
        if (dev->params().contains("channel1") && dev->params().value("channel1").toBool() == group.at(0) &&
                dev->params().contains("channel2") && dev->params().value("channel2").toBool() == group.at(1) &&
                dev->params().contains("channel3") && dev->params().value("channel3").toBool() == group.at(2) &&
                dev->params().contains("channel4") && dev->params().value("channel4").toBool() == group.at(3) &&
                dev->params().contains("channel5") && dev->params().value("channel5").toBool() == group.at(4)
                ) {
            // Yippie! We found the device.
            device = dev;
            break;
        }
    }
    if (!device) {
        //qWarning() << "couldn't find any configured device for data:" << rawData;
        return;
    }
    
    // TODO: Lets assume we received button "A" "on"
    //   QString button = "A";
    bool power = true;
    
    QVariantMap params;
    params.insert("button", button);
    params.insert("power", power);
    foreach (const Trigger &trigger, device->triggers()) {
        if (trigger.name() == button) {
            emit emitTrigger(trigger.id(), params);
            return;
        }
    }
}
