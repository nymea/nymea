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
