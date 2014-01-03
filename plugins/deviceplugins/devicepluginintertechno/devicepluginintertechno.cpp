#include "devicepluginintertechno.h"

#include "device.h"
#include "devicemanager.h"
#include "radio433.h"

#include <QDebug>
#include <QStringList>

QUuid intertechnoRemote = QUuid("ab73ad2f-6594-45a3-9063-8f72d365c5e5");
QUuid intertechnoSwitch = QUuid("324219e8-7c53-41b5-b314-c2900cd15252");

DevicePluginIntertechno::DevicePluginIntertechno()
{
}

QList<DeviceClass> DevicePluginIntertechno::supportedDevices() const
{
    QList<DeviceClass> ret;

    // Remote
    DeviceClass deviceClassIntertechnoRemote(pluginId(), intertechnoRemote);
    deviceClassIntertechnoRemote.setName("Intertechno Remote");
    
    QVariantList remoteParams;
    QVariantMap familyParam;
    // family code = A-P
    familyParam.insert("name", "familyCode");
    familyParam.insert("type", "string");
    remoteParams.append(familyParam);

    deviceClassIntertechnoRemote.setParams(remoteParams);
    
    QList<TriggerType> buttonTriggers;
    
    QVariantList paramsRemote;
    QVariantMap paramRemote;

    // on  = true
    // off = false
    paramRemote.insert("name", "power");
    paramRemote.insert("type", "bool");
    paramsRemote.append(paramRemote);

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

    TriggerType button1Trigger("785c1b30-a3f2-4696-af7c-d532acf3d6f7");
    button1Trigger.setName("1");
    button1Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button1Trigger);

    TriggerType button2Trigger("1d42c850-7b43-452f-b205-e1aac14eb3ee");
    button2Trigger.setName("2");
    button2Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button2Trigger);

    TriggerType button3Trigger("77a4780e-2355-4a77-870d-2f675bf986ce");
    button3Trigger.setName("3");
    button3Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button3Trigger);

    TriggerType button4Trigger("bd6a8b4b-f946-4f3b-992f-e7cff10187b8");
    button4Trigger.setName("4");
    button4Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button4Trigger);

    TriggerType button5Trigger("0f20782e-0acc-45f1-8c42-5dc5f5b29f1b");
    button5Trigger.setName("5");
    button5Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button5Trigger);

    TriggerType button6Trigger("f7cb439a-0528-4905-9583-06b6bfeb3ba1");
    button6Trigger.setName("6");
    button6Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button6Trigger);

    TriggerType button7Trigger("a0b0d8d8-2b43-4897-98e0-05b6b408a950");
    button7Trigger.setName("7");
    button7Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button7Trigger);

    TriggerType button8Trigger("ae5833a2-bc43-4462-ae47-e45dac1fb0ce");
    button8Trigger.setName("8");
    button8Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button8Trigger);

    TriggerType button9Trigger("52c13828-d047-4256-b488-0bf84abbc87c");
    button9Trigger.setName("9");
    button9Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button9Trigger);

    TriggerType button10Trigger("22c5afbc-835e-47cc-8211-4429eb9d9fee");
    button10Trigger.setName("10");
    button10Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button10Trigger);

    TriggerType button11Trigger("6bec5cbc-8bfb-4c6c-8ac8-f8e7723fd5aa");
    button11Trigger.setName("11");
    button11Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button11Trigger);
    
    TriggerType button12Trigger("8b71edd2-8135-4c8b-bf44-380efadf1942");
    button12Trigger.setName("12");
    button12Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button12Trigger);

    TriggerType button13Trigger("192f36a4-1e58-41aa-9618-83d46e329a4b");
    button13Trigger.setName("13");
    button13Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button13Trigger);

    TriggerType button14Trigger("6c76de60-5e19-4a29-b027-e71e66caa2d6");
    button14Trigger.setName("14");
    button14Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button14Trigger);

    TriggerType button15Trigger("c2f56c10-1f81-4477-88fa-fc0f4a6383df");
    button15Trigger.setName("15");
    button15Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button15Trigger);

    TriggerType button16Trigger("5d2eb3f8-4cd4-4c71-9c0c-e0b685e168e4");
    button16Trigger.setName("16");
    button16Trigger.setParameters(paramsRemote);
    buttonTriggers.append(button16Trigger);

    deviceClassIntertechnoRemote.setTriggers(buttonTriggers);
    ret.append(deviceClassIntertechnoRemote);


    // Switch
    DeviceClass deviceClassIntertechnoSwitch(pluginId(), intertechnoSwitch);
    deviceClassIntertechnoSwitch.setName("Intertechno Switch");

    QVariantList switchDeviceParams;
    QVariantMap buttonParam;
    // button code = 1-16
    buttonParam.insert("name", "buttonCode");
    buttonParam.insert("type", "int");

    switchDeviceParams.append(familyParam);
    switchDeviceParams.append(buttonParam);

    deviceClassIntertechnoSwitch.setParams(switchDeviceParams);

    QList<ActionType> switchActions;

    QVariantList paramsSwitch;
    QVariantMap paramSwitch;

    // on  = true
    // off = false
    paramSwitch.insert("name", "power");
    paramSwitch.insert("type", "bool");
    paramsSwitch.append(paramSwitch);

    ActionType switchActionPower(QUuid("df19fb51-c3cd-4b95-8d88-ebbb535f4789"));
    switchActionPower.setName("power");
    switchActionPower.setParameters(paramsSwitch);
    switchActions.append(switchActionPower);

    deviceClassIntertechnoSwitch.setActions(switchActions);
    ret.append(deviceClassIntertechnoSwitch);

    return ret;
}

DeviceManager::HardwareResource DevicePluginIntertechno::requiredHardware() const
{
    return DeviceManager::HardwareResourceRadio433;
}

QString DevicePluginIntertechno::pluginName() const
{
    return "Intertechno";
}

QUuid DevicePluginIntertechno::pluginId() const
{
    return QUuid("e998d934-0397-42c1-ad63-9141bcac8563");
}

void DevicePluginIntertechno::executeAction(Device *device, const Action &action)
{

    QList<int> rawData;
    QByteArray binCode;

    QString familyCode = device->params().value("familyCode").toString();

    if(familyCode == "A"){
        binCode.append("00000000");
    }else if(familyCode == "B"){
        binCode.append("01000000");
    }






}

void DevicePluginIntertechno::receiveData(QList<int> rawData)
{    
    // filter right here a wrong signal length
    if(rawData.length() != 49){
        return;
    }
    
    QList<Device*> deviceList = deviceManager()->findConfiguredDevices(intertechnoRemote);
    if(deviceList.isEmpty()){
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

    //qDebug() << "family code = " << familyCode << "button code =" << buttonCode << power;

    // ===================================================
    Device *device = 0;
    foreach (Device *dev, deviceList) {
        if (dev->params().contains("familyCode") && dev->params().value("familyCode").toString() == familyCode) {
            // Yippie! We found the device.
            device = dev;
            break;
        }
    }
    if (!device) {
        qWarning() << "couldn't find any configured device for intertech familyCode:" << familyCode;
        return;
    }

    QVariantMap params;
    params.insert("power", power);

    // FIXME: find a better way to get to the remote DeviceClass
    DeviceClass deviceClass = supportedDevices().first();
    foreach (const TriggerType &triggerType, deviceClass.triggers()) {
        if (triggerType.name() == buttonCode) {
            qDebug() << "emit trigger " << pluginName() << familyCode << triggerType.name() << power;
            Trigger trigger = Trigger(triggerType.id(), device->id(), params);
            emit emitTrigger(trigger);
            return;
        }
    }
}
