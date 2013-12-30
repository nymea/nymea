#include "rfswitch.h"

RfSwitch::RfSwitch(QObject *parent) :
    DevicePlugin(parent)
{
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
