#include "rfswitch.h"

#include "devicemanager.h"
#include "radio433.h"

#include <QDebug>

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

    DeviceClass deviceClassRfRemote(QUuid::createUuid());
    deviceClassRfRemote.setName("RF Remote");
    QList<TriggerType> buttonTriggers;

    QVariantList params;
    QVariantMap param;
    param.insert("name", "on");
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


    DeviceClass deviceClassRfSwitch(QUuid::createUuid());
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


}
