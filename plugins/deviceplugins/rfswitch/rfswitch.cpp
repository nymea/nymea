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
    deviceClassRfRemote.setName("RF Remote");

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
    deviceClassRfSwitch.setName("RF Switch");
    ret.append(deviceClassRfSwitch);

    return ret;
}

QString RfSwitch::pluginName() const
{
    return "RF Switch";
}

void RfSwitch::dataReceived(QList<int> rawData)
{
    qDebug() << "data received from Radio433" << rawData;

    // TODO: Lets assume we found a device of class "deviceClassRfRemote"
    DeviceClass deviceClassRfRemote = supportedDevices().first();

    // TODO: Lets assume we received group "1000"
    QList<bool> group;
    group << true << false << false << false << false;

    Device *device = 0;
    QList<Device*> deviceList = deviceManager()->findConfiguredDevices(deviceClassRfRemote.id());
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
        qWarning() << "couldn't find any configured device for data:" << rawData;
        return;
    }

    // TODO: Lets assume we received button "A" "on"
    QString button = "A";
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
