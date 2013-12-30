#ifndef RFSWITCH_H
#define RFSWITCH_H

#include "deviceplugin.h"

class RfSwitch : public DevicePlugin
{
    Q_OBJECT
public:
    explicit RfSwitch(QObject *parent = 0);

    QList<DeviceClass> supportedDevices() const override;

signals:

public slots:

};

#endif // RFSWITCH_H
