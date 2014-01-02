#include "devicepluginelro.h"

#include "device.h"
#include "devicemanager.h"
#include "radio433.h"

#include <QDebug>
#include <QStringList>

QUuid elroRemoteId = QUuid("d85c1ef4-197c-4053-8e40-707aa671d302");
QUuid elroSwitchId = QUuid("308ae6e6-38b3-4b3a-a513-3199da2764f8");

DevicePluginElro::DevicePluginElro()
{
}

QList<DeviceClass> DevicePluginElro::supportedDevices() const
{
    // TODO: load list from config with static uuid
    QList<DeviceClass> ret;

    // Remote
    DeviceClass deviceClassElroRemote(pluginId(), elroRemoteId);
    deviceClassElroRemote.setName("Elro Remote");
    
    QVariantList deviceParamsRemote;
    QVariantMap channelParam;
    channelParam.insert("name", "channel1");
    channelParam.insert("type", "bool");
    deviceParamsRemote.append(channelParam);
    channelParam.insert("name", "channel2");
    channelParam.insert("type", "bool");
    deviceParamsRemote.append(channelParam);
    channelParam.insert("name", "channel3");
    channelParam.insert("type", "bool");
    deviceParamsRemote.append(channelParam);
    channelParam.insert("name", "channel4");
    channelParam.insert("type", "bool");
    deviceParamsRemote.append(channelParam);
    channelParam.insert("name", "channel5");
    channelParam.insert("type", "bool");
    deviceParamsRemote.append(channelParam);
    
    deviceClassElroRemote.setParams(deviceParamsRemote);
    
    QList<TriggerType> buttonTriggers;
    
    QVariantList paramsRemote;
    QVariantMap param;
    param.insert("name", "power");
    param.insert("type", "bool");
    paramsRemote.append(param);
    
    TriggerType buttonATrigger(QUuid("9dd3f862-35f3-4b69-954e-fa3c8bd68e39"));
    buttonATrigger.setName("A");
    buttonATrigger.setParameters(paramsRemote);
    buttonTriggers.append(buttonATrigger);
    
    TriggerType buttonBTrigger(QUuid("733226eb-91ba-4e37-9d78-12c87eb5e763"));
    buttonBTrigger.setName("B");
    buttonBTrigger.setParameters(paramsRemote);
    buttonTriggers.append(buttonBTrigger);
    
    TriggerType buttonCTrigger(QUuid("47aaeaec-485a-4775-a543-33f339fd28c8"));
    buttonCTrigger.setName("C");
    buttonCTrigger.setParameters(paramsRemote);
    buttonTriggers.append(buttonCTrigger);
    
    TriggerType buttonDTrigger(QUuid("db3d484c-add9-44ab-80a4-a0664e0c87c8"));
    buttonDTrigger.setName("D");
    buttonDTrigger.setParameters(paramsRemote);
    buttonTriggers.append(buttonDTrigger);
    
    TriggerType buttonETrigger(QUuid("eb914aac-fb73-4ee2-9f1b-c34b2f6cc24a"));
    buttonETrigger.setName("E");
    buttonETrigger.setParameters(paramsRemote);
    buttonTriggers.append(buttonETrigger);
    
    deviceClassElroRemote.setTriggers(buttonTriggers);
    ret.append(deviceClassElroRemote);

    // Switch
    DeviceClass deviceClassElroSwitch(pluginId(), elroSwitchId);
    deviceClassElroSwitch.setName("Elro Power Switch");
    
    QVariantList deviceParamsSwitch;
    QVariantMap paramSwitch;
    paramSwitch.insert("name", "channel1");
    paramSwitch.insert("type", "bool");
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch.insert("name", "channel2");
    paramSwitch.insert("type", "bool");
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch.insert("name", "channel3");
    paramSwitch.insert("type", "bool");
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch.insert("name", "channel4");
    paramSwitch.insert("type", "bool");
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch.insert("name", "channel5");
    paramSwitch.insert("type", "bool");
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch.insert("name", "A");
    paramSwitch.insert("type", "bool");
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch.insert("name", "B");
    paramSwitch.insert("type", "bool");
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch.insert("name", "C");
    paramSwitch.insert("type", "bool");
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch.insert("name", "D");
    paramSwitch.insert("type", "bool");
    deviceParamsSwitch.append(paramSwitch);
    paramSwitch.insert("name", "E");
    paramSwitch.insert("type", "bool");
    deviceParamsSwitch.append(paramSwitch);

    deviceClassElroSwitch.setParams(deviceParamsSwitch);


    QVariantList actionParamsSwitch;
    QVariantMap actionParamSwitch;
    actionParamSwitch.insert("name", "power");
    actionParamSwitch.insert("type", "bool");
    actionParamsSwitch.append(actionParamSwitch);

    QList<ActionType> switchActions;

    ActionType powerAction(QUuid("31c9758e-6567-4f89-85bb-29e1a7c55d44"));
    powerAction.setName("power");
    powerAction.setParameters(actionParamsSwitch);
    switchActions.append(powerAction);

    deviceClassElroSwitch.setActions(switchActions);
    ret.append(deviceClassElroSwitch);
    return ret;
}

DeviceManager::HardwareResource DevicePluginElro::requiredHardware() const
{
    return DeviceManager::HardwareResourceRadio433;
}

QString DevicePluginElro::pluginName() const
{
    return QStringLiteral("Elro");
}

QUuid DevicePluginElro::pluginId() const
{
    return QUuid("2b267f81-d9ae-4f4f-89a0-7386b547cfd3");
}

void DevicePluginElro::executeAction(Device *device, const Action &action)
{

    QList<int> rawData;
    QByteArray binCode;

    qDebug() << "rawData" << rawData;
    // =======================================
    // create the bincode
    // channels
    if(device->params().value("channel1").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->params().value("channel2").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->params().value("channel3").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->params().value("channel4").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->params().value("channel5").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }

    // Buttons
    if(device->params().value("A").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->params().value("B").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->params().value("C").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->params().value("D").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    if(device->params().value("E").toBool()){
        binCode.append("00");
    }else{
        binCode.append("01");
    }
    // Power
    if(action.params().first().toBool()){
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
    qDebug() << "rawData" << rawData;
    transmitData(rawData);
}

void DevicePluginElro::receiveData(QList<int> rawData)
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
        qWarning() << "couldn't find any configured device for elro:" << binCode.left(10) ;
        return;
    }

    QVariantMap params;
    params.insert("power", power);

    // FIXME: find a better way to get to the remote DeviceClass
    DeviceClass deviceClass = supportedDevices().first();
    foreach (const TriggerType &triggerType, deviceClass.triggers()) {
        if (triggerType.name() == button) {
            Trigger trigger = Trigger(triggerType.id(), params);
            emit emitTrigger(trigger);
            return;
        }
    }
}
