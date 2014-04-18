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
    \page elro.html
    \title Elro

    \ingroup plugins
    \ingroup rf433

    This plugin allows to controll RF 433 MHz actors an receive remote signals from Elro
    devices.

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

#include "devicepluginelro.h"

#include "devicemanager.h"
#include "plugin/device.h"
#include "hardware/radio433.h"

#include <QDebug>
#include <QStringList>

VendorId elroVendorId = VendorId("435a13a0-65ca-4f0c-94c1-e5873b258db5");
VendorId mumbiVendorId = VendorId("5f91c01c-0168-4bdf-a5ed-37cb6971b775");

DeviceClassId elroRemoteId = DeviceClassId("d85c1ef4-197c-4053-8e40-707aa671d302");
DeviceClassId elroSwitchId = DeviceClassId("308ae6e6-38b3-4b3a-a513-3199da2764f8");

DevicePluginElro::DevicePluginElro()
{
}

QList<Vendor> DevicePluginElro::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor elro(elroVendorId, "Electronic Roos");
    ret.append(elro);

    Vendor mumbi(mumbiVendorId, "Mumbi");
    ret.append(mumbi);

    return ret;
}

QList<DeviceClass> DevicePluginElro::supportedDevices() const
{
    // TODO: load list from config with static uuid
    QList<DeviceClass> ret;

    // =======================================
    // Remote
    DeviceClass deviceClassElroRemote(pluginId(), elroVendorId, elroRemoteId);
    deviceClassElroRemote.setName("Elro Remote");
    
    QList<ParamType> deviceParamsRemote;
    ParamType channelParam = ParamType("channel1", QVariant::Bool);
    deviceParamsRemote.append(channelParam);
    channelParam = ParamType("channel2", QVariant::Bool);
    deviceParamsRemote.append(channelParam);
    channelParam = ParamType("channel3", QVariant::Bool);
    deviceParamsRemote.append(channelParam);
    channelParam = ParamType("channel4", QVariant::Bool);
    deviceParamsRemote.append(channelParam);
    channelParam = ParamType("channel5", QVariant::Bool);
    deviceParamsRemote.append(channelParam);
    
    deviceClassElroRemote.setParams(deviceParamsRemote);
    
    QList<EventType> buttonEvents;
    
    QVariantList paramsRemote;
    QVariantMap param;
    param.insert("name", "power");
    param.insert("type", "bool");
    paramsRemote.append(param);
    
    EventType buttonAEvent(EventTypeId("9dd3f862-35f3-4b69-954e-fa3c8bd68e39"));
    buttonAEvent.setName("A");
    buttonAEvent.setParameters(paramsRemote);
    buttonEvents.append(buttonAEvent);
    
    EventType buttonBEvent(EventTypeId("733226eb-91ba-4e37-9d78-12c87eb5e763"));
    buttonBEvent.setName("B");
    buttonBEvent.setParameters(paramsRemote);
    buttonEvents.append(buttonBEvent);
    
    EventType buttonCEvent(EventTypeId("47aaeaec-485a-4775-a543-33f339fd28c8"));
    buttonCEvent.setName("C");
    buttonCEvent.setParameters(paramsRemote);
    buttonEvents.append(buttonCEvent);
    
    EventType buttonDEvent(EventTypeId("db3d484c-add9-44ab-80a4-a0664e0c87c8"));
    buttonDEvent.setName("D");
    buttonDEvent.setParameters(paramsRemote);
    buttonEvents.append(buttonDEvent);
    
    EventType buttonEEvent(EventTypeId("eb914aac-fb73-4ee2-9f1b-c34b2f6cc24a"));
    buttonEEvent.setName("E");
    buttonEEvent.setParameters(paramsRemote);
    buttonEvents.append(buttonEEvent);
    
    deviceClassElroRemote.setEvents(buttonEvents);
    ret.append(deviceClassElroRemote);

    // =======================================
    // Switch
    DeviceClass deviceClassElroSwitch(pluginId(), elroVendorId, elroSwitchId);
    deviceClassElroSwitch.setName("Elro Power Switch");
    
    QList<ParamType> deviceParamsSwitch;
    ParamType paramSwitch = ParamType("channel1", QVariant::Bool);
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch = ParamType("channel2", QVariant::Bool);
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch = ParamType("channel3", QVariant::Bool);
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch = ParamType("channel4", QVariant::Bool);
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch = ParamType("channel5", QVariant::Bool);
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch = ParamType("channel6", QVariant::Bool);
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch = ParamType("channel7", QVariant::Bool);
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch = ParamType("channel8", QVariant::Bool);
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch = ParamType("channel9", QVariant::Bool);
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch = ParamType("channel10", QVariant::Bool);
    deviceParamsSwitch.append(paramSwitch);

    deviceClassElroSwitch.setParams(deviceParamsSwitch);


    QVariantList actionParamsSwitch;
    QVariantMap actionParamSwitch;
    actionParamSwitch.insert("name", "power");
    actionParamSwitch.insert("type", "bool");
    actionParamsSwitch.append(actionParamSwitch);

    QList<ActionType> switchActions;

    ActionType powerAction(ActionTypeId("31c9758e-6567-4f89-85bb-29e1a7c55d44"));
    powerAction.setName("power");
    powerAction.setParameters(actionParamsSwitch);
    switchActions.append(powerAction);

    deviceClassElroSwitch.setActions(switchActions);
    ret.append(deviceClassElroSwitch);
    return ret;
}

DeviceManager::HardwareResources DevicePluginElro::requiredHardware() const
{
    return DeviceManager::HardwareResourceRadio433;
}

QString DevicePluginElro::pluginName() const
{
    return QStringLiteral("Elro");
}

PluginId DevicePluginElro::pluginId() const
{
    return PluginId("2b267f81-d9ae-4f4f-89a0-7386b547cfd3");
}

DeviceManager::DeviceError DevicePluginElro::executeAction(Device *device, const Action &action)
{

    QList<int> rawData;
    QByteArray binCode;

    // =======================================
    // create the bincode
    // channels
    if(device->paramValue("channel1").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->paramValue("channel2").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->paramValue("channel3").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->paramValue("channel4").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->paramValue("channel5").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }

    // =======================================
    // Buttons
    if(device->paramValue("A").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->paramValue("B").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->paramValue("C").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->paramValue("D").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->paramValue("E").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    // Power
    if(action.param("power").value().toBool()){
        binCode.append("0001");
    }else{
        binCode.append("0100");
    }

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
    //qDebug() << "rawData" << rawData;
    qDebug() << "transmit" << pluginName() << action.param("power").value().toBool();
    transmitData(rawData);
    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginElro::radioData(QList<int> rawData)
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

            /*
             *       _
             *      | |___   = 0 -> in 4 delays => 1000
             *          _
             *      ___| |   = 1 -> in 4 delays => 0001
             */

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
    
    // get the button letter
    QString button;
    QByteArray buttonCode = binCode.mid(10,10);

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

    // get power status -> On = 0100, Off = 0001
    bool power;
    if(binCode.right(4).toInt(0,2) == 1){
        power = true;
    }else if(binCode.right(4).toInt(0,2) == 4){
        power = false;
    }else{
        return;
    }

    Device *device = 0;
    QList<Device*> deviceList = deviceManager()->findConfiguredDevices(elroRemoteId);
    foreach (Device *dev, deviceList) {
        if (dev->hasParam("channel1") && dev->paramValue("channel1").toBool() == group.at(0) &&
                dev->hasParam("channel2") && dev->paramValue("channel2").toBool() == group.at(1) &&
                dev->hasParam("channel3") && dev->paramValue("channel3").toBool() == group.at(2) &&
                dev->hasParam("channel4") && dev->paramValue("channel4").toBool() == group.at(3) &&
                dev->hasParam("channel5") && dev->paramValue("channel5").toBool() == group.at(4)
                ) {
            // Yippie! We found the device.
            device = dev;
            break;
        }
    }
    if (!device) {
        qWarning() << "couldn't find any configured device for elro:" << binCode.left(10) ;
        return;
    }

    QList<Param> params;
    Param powerParam("power", power);
    params.append(powerParam);

    // FIXME: find a better way to get to the remote DeviceClass
    DeviceClass deviceClass = supportedDevices().first();
    foreach (const EventType &eventType, deviceClass.events()) {
        if (eventType.name() == button) {
            qDebug() << "emit event " << pluginName() << group << eventType.name() << power;
            Event event = Event(eventType.id(), device->id(), params);
            emit emitEvent(event);
            return;
        }
    }
}
