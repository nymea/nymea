#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "deviceclass.h"

#include <QObject>

class Device;
class DevicePlugin;
class Radio433;

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(QObject *parent = 0);

    QList<DeviceClass> supportedDevices();

    QList<Device*> devices() const;

    Radio433 *radio433() const;

signals:

private slots:
    void loadPlugins();

private:
    QList<DeviceClass> m_supportedDevices;
    QList<Device*> m_devices;
    QList<DevicePlugin*> m_devicePlugins;

    Radio433* m_radio433;
};

#endif // DEVICEMANAGER_H
