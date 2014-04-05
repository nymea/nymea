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
    \page intertechno.html
    \title Intertechno

    \ingroup plugins
    \ingroup rf433

    This plugin allows to controll RF 433 MHz actors an receive remote signals from Intertechno
    devices (\l{http://www.intertechno.at}).

    Following devices are supported:

    \chapter Supported devices
        \section1 Actors
            \table
            \header
                \li Model
                \li Device Type
            \row
                \li IT-1500
                \li Socket Switch
            \row
                \li IT-3500L
                \li Socket Switch
            \row
                \li ITLR-3500
                \li Socket Switch
            \row
                \li ITLR-3500T
                \li Socket Switch
            \row
                \li IT-2300
                \li Multiple Socket Switch
            \row
                \li GRR-3500
                \li Outdoor Socket Switch
            \row
                \li IT-3500
                \li Socket Switch
            \row
                \li ITR-3500
                \li Socket Switch
            \row
                \li PA3-1000
                \li Socket Switch
            \row
                \li IT-300L
                \li Socket Dimmer
            \row
                \li ITLR-300
                \li Socket Dimmer
            \row
                \li IT-300
                \li Socket Dimmer
            \row
                \li ITR-300
                \li Socket Dimmer
            \row
                \li ITL-1000
                \li Switch
            \row
                \li ITL-230
                \li Switch
            \row
                \li CMR-1000
                \li Switch
            \row
                \li ITDL-1000
                \li Switch
            \row
                \li CMR-1224
                \li Switch
            \row
                \li ITL-3500
                \li Switch
            \row
                \li ITE-1000
                \li Switch
            \row
                \li ITL-300
                \li Dimmer
            \row
                \li ITL-210
                \li Dimmer
            \row
                \li ITL-150
                \li Dimmer
            \row
                \li ITL-250
                \li Dimmer (LED)
            \row
                \li CMR-300
                \li Dimmer
            \row
                \li ITE-300
                \li Dimmer
            \row
                \li CMR-500
                \li Jalousie Switch
            \row
                \li ITL-500
                \li Jalousie Switch
            \row
                \li ITL-1000
                \li Jalousie Switch
            \row
                \li LBUR-100
                \li Lamp Switch
            \endtable

        \section1 Remotes
            \table
            \header
                \li Model
                \li Device Type
            \row
                \li ITK-200
                \li Mini Remote
            \row
                \li ITS-150
                \li Remote
            \row
                \li ITZ-500
                \li Remote/Timer
            \row
                \li YWT-8500
                \li Wall Sender
            \row
                \li ITW-852
                \li Wall Sender
            \row
                \li ITM-100
                \li Magnet Sensor
            \row
                \li PIR-5000
                \li Moving Sensor
            \endtable
  */

#include "devicepluginintertechno.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "hardware/radio433.h"

#include <QDebug>
#include <QStringList>

VendorId intertechnoVendorId = VendorId("6a852bc2-34dd-4f4c-9ac9-dd4c32ddbcba");
DeviceClassId intertechnoRemote = DeviceClassId("ab73ad2f-6594-45a3-9063-8f72d365c5e5");
DeviceClassId intertechnoSwitch = DeviceClassId("324219e8-7c53-41b5-b314-c2900cd15252");

DevicePluginIntertechno::DevicePluginIntertechno()
{
}

QList<Vendor> DevicePluginIntertechno::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor intertechno(intertechnoVendorId, "Intertechno");
    return ret;
}

QList<DeviceClass> DevicePluginIntertechno::supportedDevices() const
{
    QList<DeviceClass> ret;

    // =======================================
    // Remote
    DeviceClass deviceClassIntertechnoRemote(pluginId(), intertechnoVendorId, intertechnoRemote);
    deviceClassIntertechnoRemote.setName("Intertechno Remote");
    
    QVariantList remoteParams;
    QVariantMap familyParam;
    // family code = A-P
    familyParam.insert("name", "familyCode");
    familyParam.insert("type", "string");
    remoteParams.append(familyParam);

    deviceClassIntertechnoRemote.setParams(remoteParams);
    
    QList<EventType> buttonEvents;
    
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

    EventType button1Event(EventTypeId("785c1b30-a3f2-4696-af7c-d532acf3d6f7"));
    button1Event.setName("1");
    button1Event.setParameters(paramsRemote);
    buttonEvents.append(button1Event);

    EventType button2Event(EventTypeId("1d42c850-7b43-452f-b205-e1aac14eb3ee"));
    button2Event.setName("2");
    button2Event.setParameters(paramsRemote);
    buttonEvents.append(button2Event);

    EventType button3Event(EventTypeId("77a4780e-2355-4a77-870d-2f675bf986ce"));
    button3Event.setName("3");
    button3Event.setParameters(paramsRemote);
    buttonEvents.append(button3Event);

    EventType button4Event(EventTypeId("bd6a8b4b-f946-4f3b-992f-e7cff10187b8"));
    button4Event.setName("4");
    button4Event.setParameters(paramsRemote);
    buttonEvents.append(button4Event);

    EventType button5Event(EventTypeId("0f20782e-0acc-45f1-8c42-5dc5f5b29f1b"));
    button5Event.setName("5");
    button5Event.setParameters(paramsRemote);
    buttonEvents.append(button5Event);

    EventType button6Event(EventTypeId("f7cb439a-0528-4905-9583-06b6bfeb3ba1"));
    button6Event.setName("6");
    button6Event.setParameters(paramsRemote);
    buttonEvents.append(button6Event);

    EventType button7Event(EventTypeId("a0b0d8d8-2b43-4897-98e0-05b6b408a950"));
    button7Event.setName("7");
    button7Event.setParameters(paramsRemote);
    buttonEvents.append(button7Event);

    EventType button8Event(EventTypeId("ae5833a2-bc43-4462-ae47-e45dac1fb0ce"));
    button8Event.setName("8");
    button8Event.setParameters(paramsRemote);
    buttonEvents.append(button8Event);

    EventType button9Event(EventTypeId("52c13828-d047-4256-b488-0bf84abbc87c"));
    button9Event.setName("9");
    button9Event.setParameters(paramsRemote);
    buttonEvents.append(button9Event);

    EventType button10Event(EventTypeId("22c5afbc-835e-47cc-8211-4429eb9d9fee"));
    button10Event.setName("10");
    button10Event.setParameters(paramsRemote);
    buttonEvents.append(button10Event);

    EventType button11Event(EventTypeId("6bec5cbc-8bfb-4c6c-8ac8-f8e7723fd5aa"));
    button11Event.setName("11");
    button11Event.setParameters(paramsRemote);
    buttonEvents.append(button11Event);
    
    EventType button12Event(EventTypeId("8b71edd2-8135-4c8b-bf44-380efadf1942"));
    button12Event.setName("12");
    button12Event.setParameters(paramsRemote);
    buttonEvents.append(button12Event);

    EventType button13Event(EventTypeId("192f36a4-1e58-41aa-9618-83d46e329a4b"));
    button13Event.setName("13");
    button13Event.setParameters(paramsRemote);
    buttonEvents.append(button13Event);

    EventType button14Event(EventTypeId("6c76de60-5e19-4a29-b027-e71e66caa2d6"));
    button14Event.setName("14");
    button14Event.setParameters(paramsRemote);
    buttonEvents.append(button14Event);

    EventType button15Event(EventTypeId("c2f56c10-1f81-4477-88fa-fc0f4a6383df"));
    button15Event.setName("15");
    button15Event.setParameters(paramsRemote);
    buttonEvents.append(button15Event);

    EventType button16Event(EventTypeId("5d2eb3f8-4cd4-4c71-9c0c-e0b685e168e4"));
    button16Event.setName("16");
    button16Event.setParameters(paramsRemote);
    buttonEvents.append(button16Event);

    deviceClassIntertechnoRemote.setEvents(buttonEvents);
    ret.append(deviceClassIntertechnoRemote);


    // =======================================
    // Switch
    DeviceClass deviceClassIntertechnoSwitch(pluginId(), intertechnoVendorId, intertechnoSwitch);
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

    ActionType switchActionPower(ActionTypeId("df19fb51-c3cd-4b95-8d88-ebbb535f4789"));
    switchActionPower.setName("power");
    switchActionPower.setParameters(paramsSwitch);
    switchActions.append(switchActionPower);

    deviceClassIntertechnoSwitch.setActions(switchActions);
    ret.append(deviceClassIntertechnoSwitch);

    return ret;
}

DeviceManager::HardwareResources DevicePluginIntertechno::requiredHardware() const
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

    // =======================================
    // generate bin from family code
    if(familyCode == "A"){
        binCode.append("00000000");
    }else if(familyCode == "B"){
        binCode.append("01000000");
    }else if(familyCode == "C"){
        binCode.append("00010000");
    }else if(familyCode == "D"){
        binCode.append("01010000");
    }else if(familyCode == "E"){
        binCode.append("00000100");
    }else if(familyCode == "F"){
        binCode.append("01000100");
    }else if(familyCode == "G"){
        binCode.append("01000000");
    }else if(familyCode == "H"){
        binCode.append("01010100");
    }else if(familyCode == "I"){
        binCode.append("00000001");
    }else if(familyCode == "J"){
        binCode.append("01000001");
    }else if(familyCode == "K"){
        binCode.append("00010001");
    }else if(familyCode == "L"){
        binCode.append("01010001");
    }else if(familyCode == "M"){
        binCode.append("00000101");
    }else if(familyCode == "N"){
        binCode.append("01000101");
    }else if(familyCode == "O"){
        binCode.append("00010101");
    }else if(familyCode == "P"){
        binCode.append("01010101");
    }else{
        return;
    }

    QString buttonCode = device->params().value("buttonCode").toString();

    // =======================================
    // generate bin from button code
    if(familyCode == "1"){
        binCode.append("00000000");
    }else if(familyCode == "2"){
        binCode.append("01000000");
    }else if(familyCode == "3"){
        binCode.append("00010000");
    }else if(familyCode == "4"){
        binCode.append("01010000");
    }else if(familyCode == "5"){
        binCode.append("00000100");
    }else if(familyCode == "6"){
        binCode.append("01000100");
    }else if(familyCode == "7"){
        binCode.append("01000000");
    }else if(familyCode == "8"){
        binCode.append("01010100");
    }else if(familyCode == "9"){
        binCode.append("00000001");
    }else if(familyCode == "10"){
        binCode.append("01000001");
    }else if(familyCode == "11"){
        binCode.append("00010001");
    }else if(familyCode == "12"){
        binCode.append("01010001");
    }else if(familyCode == "13"){
        binCode.append("00000101");
    }else if(familyCode == "14"){
        binCode.append("01000101");
    }else if(familyCode == "15"){
        binCode.append("00010101");
    }else if(familyCode == "16"){
        binCode.append("01010101");
    }else{
        return;
    }

    // =======================================
    // add fix nibble (0F)
    binCode.append("0001");

    // =======================================
    // add power nibble
    if(action.params().value("power").toBool()){
        binCode.append("0101");
    }else{
        binCode.append("0100");
    }
    //qDebug() << "bin code:" << binCode;
    // =======================================
    //create rawData timings list
    int delay = 350;

    // sync signal
    rawData.append(delay);
    rawData.append(delay*31);

    // add the code
    foreach (QChar c, binCode) {
        if(c == '0'){
            rawData.append(delay);
            rawData.append(delay*3);
        }else{
            rawData.append(delay*3);
            rawData.append(delay);
        }
    }

    // =======================================
    // send data to driver
    qDebug() << "transmit" << pluginName() << familyCode << buttonCode << action.params().value("power").toBool();
    transmitData(rawData);

}

void DevicePluginIntertechno::radioData(QList<int> rawData)
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
    
    // =======================================
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

    // =======================================
    // Check nibble 16-19, must be 0001
    if(binCode.mid(16,4) != "0001"){
        return;
    }

    // =======================================
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

    // =======================================
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

    // =======================================
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
    foreach (const EventType &eventType, deviceClass.events()) {
        if (eventType.name() == buttonCode) {
            qDebug() << "emit event " << pluginName() << familyCode << eventType.name() << power;
            Event event = Event(eventType.id(), device->id(), params);
            emit emitEvent(event);
            return;
        }
    }
}
