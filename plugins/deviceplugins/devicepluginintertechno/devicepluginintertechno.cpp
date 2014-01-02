#include "devicepluginintertechno.h"

#include "device.h"
#include "devicemanager.h"
#include "radio433.h"

#include <QDebug>
#include <QStringList>

QUuid intertechnoRemote = QUuid("ab73ad2f-6594-45a3-9063-8f72d365c5e5");

DevicePluginIntertechno::DevicePluginIntertechno()
{
}

void DevicePluginIntertechno::init()
{
    connect(deviceManager()->radio433(), &Radio433::dataReceived, this, &DevicePluginIntertechno::dataReceived);
}

QList<DeviceClass> DevicePluginIntertechno::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassRfRemote(pluginId(), intertechnoRemote);
    deviceClassRfRemote.setName("Intertechno Remote");
    
    QVariantList deviceParams;
    QVariantMap channelParam;
    // family code = A-P
    channelParam.insert("name", "familycode");
    channelParam.insert("type", "string");
    
    deviceClassRfRemote.setParams(deviceParams);
    
    QList<TriggerType> buttonTriggers;
    
    QVariantList params;
    QVariantMap param;

    // on  = true
    // off = false
    param.insert("name", "power");
    param.insert("type", "bool");
    params.append(param);


    /*              1-16
     *        ________________
     *       | I | II|III| IV |
     *       |---|---|---|----|
     *    1  | 1 | 5 | 9 | 13 |
     *    2  | 2 | 6 | 10| 14 |
     *    3  | 3 | 7 | 11| 15 |
     *    4  | 4 | 8 | 12| 16 |
     *       |___|___|___|____|
     */
    param.insert("name", "button");
    param.insert("type", "int");
    params.append(param);

    TriggerType button1Trigger("785c1b30-a3f2-4696-af7c-d532acf3d6f7");
    button1Trigger.setName("1");
    button1Trigger.setParameters(params);
    buttonTriggers.append(button1Trigger);

    TriggerType button2Trigger("1d42c850-7b43-452f-b205-e1aac14eb3ee");
    button2Trigger.setName("2");
    button2Trigger.setParameters(params);
    buttonTriggers.append(button2Trigger);


    deviceClassRfRemote.setTriggers(buttonTriggers);
    
    ret.append(deviceClassRfRemote);
    return ret;
}

QString DevicePluginIntertechno::pluginName() const
{
    return "RF Remote Intertechno";
}

QUuid DevicePluginIntertechno::pluginId() const
{
    return QUuid("e998d934-0397-42c1-ad63-9141bcac8563");
}

void DevicePluginIntertechno::executeAction(Device *device, const Action &action)
{

}

void DevicePluginIntertechno::dataReceived(QList<int> rawData)
{    
    // filter right here a wrong signal length
    if(rawData.length() != 49){
        return;
    }
    
    int delay = rawData.first()/31;
    QByteArray binCode;
    
    // average 314
    if(delay > 300 && delay < 400){
        // go trough all 48 timings (without sync signal)
        for(int i = 1; i <= 48; i+=2 ){
            int div;
            int divNext;
            
            // if short
            if(rawData.at(i) <= 700){
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
    }else{
        return;
    }

    // Check nibble 16-19, must be 0001
    if(binCode.mid(16,4) != "0001"){
        return;
    }

    // Get family code
    QString familyCode;
    bool ok;
    QByteArray familyCodeBin = binCode.left(8);
    int famiyCodeInt = familyCodeBin.toInt(&ok,2);

    if(!ok){
        return;
    }

    switch (famiyCodeInt) {
    case 0b00000000:
        familyCode = "A";
        break;
    case 0b01000000:
        familyCode = "B";
        break;
    case 0b00010000:
        familyCode = "C";
        break;
    case 0b01010000:
        familyCode = "D";
        break;
    case 0b00000100:
        familyCode = "E";
        break;
    case 0b01000100:
        familyCode = "F";
        break;
    case 0b00010100:
        familyCode = "G";
        break;
    case 0b01010100:
        familyCode = "H";
        break;
    case 0b00000001:
        familyCode = "I";
        break;
    case 0b01000001:
        familyCode = "J";
        break;
    case 0b00010001:
        familyCode = "K";
        break;
    case 0b01010001:
        familyCode = "L";
        break;
    case 0b00000101:
        familyCode = "M";
        break;
    case 0b01000101:
        familyCode = "N";
        break;
    case 0b00010101:
        familyCode = "O";
        break;
    case 0b01010101:
        familyCode = "P";
        break;
    default:
        return;
    }

    // Get button code
    QString buttonCode;
    QByteArray buttonCodeBin = binCode.mid(8,8);
    int buttonCodeInt = buttonCodeBin.toInt(&ok,2);

    if(!ok){
        return;
    }

    switch (buttonCodeInt) {
    case 0b00000000:
        buttonCode = "1";
        break;
    case 0b01000000:
        buttonCode = "2";
        break;
    case 0b00010000:
        buttonCode = "3";
        break;
    case 0b01010000:
        buttonCode = "4";
        break;
    case 0b00000100:
        buttonCode = "5";
        break;
    case 0b01000100:
        buttonCode = "6";
        break;
    case 0b00010100:
        buttonCode = "7";
        break;
    case 0b01010100:
        buttonCode = "8";
        break;
    case 0b00000001:
        buttonCode = "9";
        break;
    case 0b01000001:
        buttonCode = "10";
        break;
    case 0b00010001:
        buttonCode = "11";
        break;
    case 0b01010001:
        buttonCode = "12";
        break;
    case 0b00000101:
        buttonCode = "13";
        break;
    case 0b01000101:
        buttonCode = "14";
        break;
    case 0b00010101:
        buttonCode = "15";
        break;
    case 0b01010101:
        buttonCode = "16";
        break;
    default:
        return;
    }

    // get power status -> On = 0100, Off = 0001
    bool power;
    if(binCode.right(4).toInt(0,2) == 5){
        power = true;
    }else if(binCode.right(4).toInt(0,2) == 4){
        power = false;
    }else{
        return;
    }

    qDebug() << "family code = " << familyCode << "button code =" << buttonCode << power;
    return;



    //    // get the channel of the remote signal (5 channels, true=1, false=0)
    //    QList<bool> group;
    //    for(int i = 1; i < 10; i+=2){
    //        if(binCode.at(i-1) == '0' && binCode.at(i) == '1'){
    //            group << false;
    //        }else if(binCode.at(i-1) == '0' && binCode.at(i) == '0'){
    //            group << true;
    //        }else {
    //            return;
    //        }
    //    }
    
    //    // get the button letter
    //    QString button;
    //    QByteArray buttonCode = binCode.mid(10,10);

    //    if(buttonCode == "0001010101"){
    //        button = "A";
    //    }else if(buttonCode == "0100010101"){
    //        button = "B";
    //    }else if(buttonCode == "0101000101"){
    //        button = "C";
    //    }else if(buttonCode == "0101010001"){
    //        button = "D";
    //    }else if(buttonCode == "0101010100"){
    //        button = "E";
    //    }else{
    //        return;
    //    }

    //    // get power status -> On = 0100, Off = 0001
    //    bool power;
    //    if(binCode.right(4).toInt(0,2) == 1){
    //        power = true;
    //    }else if(binCode.right(4).toInt(0,2) == 4){
    //        power = false;
    //    }else{
    //        return;
    //    }

    //    Device *device = 0;
    //    QList<Device*> deviceList = deviceManager()->findConfiguredDevices(intertechnoRemote);
    //    foreach (Device *dev, deviceList) {
    //        if (dev->params().contains("channel1") && dev->params().value("channel1").toBool() == group.at(0) &&
    //                dev->params().contains("channel2") && dev->params().value("channel2").toBool() == group.at(1) &&
    //                dev->params().contains("channel3") && dev->params().value("channel3").toBool() == group.at(2) &&
    //                dev->params().contains("channel4") && dev->params().value("channel4").toBool() == group.at(3) &&
    //                dev->params().contains("channel5") && dev->params().value("channel5").toBool() == group.at(4)
    //                ) {
    //            // Yippie! We found the device.
    //            device = dev;
    //            break;
    //        }
    //    }
    //    if (!device) {
    //        qWarning() << "couldn't find any configured device for mumbi:" << binCode.left(10) ;
    //        return;
    //    }
    
    //    QVariantMap params;
    //    params.insert("button", button);
    //    params.insert("power", power);
    //    foreach (const Trigger &trigger, device->triggers()) {
    //        //qDebug() << "got trigger" << trigger.name();
    //        if (trigger.name() == button) {
    //            emit emitTrigger(trigger.id(), params);
    //            return;
    //        }
    //    }
}
