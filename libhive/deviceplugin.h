#ifndef DEVICEPLUGIN_H
#define DEVICEPLUGIN_H

#include "deviceclass.h"

#include <QObject>

class DevicePlugin: public QObject
{
    Q_OBJECT
public:
    DevicePlugin(QObject *parent = 0);
    virtual ~DevicePlugin();

    virtual QList<DeviceClass> supportedDevices() const = 0;

};

#endif
