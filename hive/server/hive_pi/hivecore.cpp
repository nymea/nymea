#include "hivecore.h"

HiveCore::HiveCore(QObject *parent) :
    QObject(parent)
{
    m_server = new Server(this);
    m_server->startServer();

    DeviceManager deviceManager;
    deviceManager.saveDeviceValue("sensor","light","A-ON",1361);
    deviceManager.saveDeviceValue("sensor","light","A-OFF",1364);
    deviceManager.saveDeviceValue("sensor","light","B-ON",4433);
    deviceManager.saveDeviceValue("sensor","light","B-OFF",4436);
    deviceManager.saveDeviceValue("sensor","light","C-ON",5393);
    deviceManager.saveDeviceValue("sensor","light","C-OFF",5204);
    deviceManager.saveDeviceValue("sensor","light","D-ON",5393);
    deviceManager.saveDeviceValue("sensor","light","D-OFF",5396);
    deviceManager.saveDeviceValue("sensor","weatherStation","temperature",20);
    deviceManager.saveDeviceValue("sensor","weatherStation","humidity",20);


    deviceManager.saveDeviceValue("actor","window","open",true);
    deviceManager.saveDeviceValue("actor","door","open",false);

    qDebug() << "get sensors" << deviceManager.getDevices("sensor");
    qDebug() << "get actors" << deviceManager.getDevices("actor");
    qDebug() << "get light keys" << deviceManager.getDeviceKeys("sensor","light");
    qDebug() << "get weatherStation keys" << deviceManager.getDeviceKeys("sensor","weatherStation");

    deviceManager.deleteDeviceValue("sensor","light","C-OFF");
    qDebug() << "get light keys" << deviceManager.getDeviceKeys("sensor","light");
    deviceManager.deleteDevice("sensor","weatherStation");
    qDebug() << "get sensors" << deviceManager.getDevices("sensor");


}
