#include "devicemanager.h"
#include <QDebug>

DeviceManager::DeviceManager(QObject *parent) :
    QObject(parent)
{
}

bool DeviceManager::saveDevice(QString deviceType, QUuid uuid, QVariantMap paramters)
{
    QSettings settings("hive");
    settings.beginGroup(deviceType);
    settings.beginGroup(uuid.toString());
    QVariantMap::iterator i = paramters.begin();
    while (i!= paramters.end()){
        settings.setValue(i.key(),i.value());
        ++i;
    }
    settings.endGroup();
    settings.endGroup();
    return true;
}

bool DeviceManager::deleteDevice(QString deviceType,  QUuid uuid)
{
    QSettings settings("hive");
    settings.beginGroup(deviceType);

    // controll if we have a stored device with this uuid
    if(!settings.childGroups().contains(uuid.toString())){
        qDebug() << "no device with uuid" << uuid.toString() << "found.";
        return false;
    }
    settings.remove(uuid.toString());
    settings.endGroup();
    return true;
}



