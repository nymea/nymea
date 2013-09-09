#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QUuid>

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(QObject *parent = 0);

    /* deviceType
     * radio
     * ...
     */

    bool saveDevice(QString deviceType, QUuid uuid, QVariantMap paramters);
    bool deleteDevice(QString deviceType, QUuid uuid);


signals:
    
public slots:

};

#endif // DEVICEMANAGER_H
