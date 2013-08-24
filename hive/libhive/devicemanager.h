#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QSettings>
#include <QStringList>

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(QObject *parent = 0);
    enum DeviceType {
        Sensor,
        Actor
    };
    enum DeviceName {
        Light,
        Switch

    };



signals:
    
public slots:
    void saveDeviceValue(QString deviceType, QString deviceName, QString key, QVariant value);
    void deleteDeviceValue(QString deviceType, QString deviceName, QString key);
    void deleteDevice(QString deviceType, QString deviceName);
    QStringList getDevices(QString deviceType);
    QStringList getDeviceKeys(QString deviceType, QString deviceName);

};

#endif // DEVICEMANAGER_H
