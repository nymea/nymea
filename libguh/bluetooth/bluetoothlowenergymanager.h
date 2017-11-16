#ifndef BLUETOOTHLOWENERGYMANAGER_H
#define BLUETOOTHLOWENERGYMANAGER_H

#include <QObject>
#include <QBluetoothLocalDevice>

#include "hardwareresource.h"
#include "bluetoothscanner.h"

class BluetoothLowEnergyManager : public HardwareResource
{
    Q_OBJECT

    friend class HardwareManager;

public:

private:
    explicit BluetoothLowEnergyManager(QObject *parent = nullptr);
    QList<BluetoothScanner *> m_bluetoothScanners;

signals:

public slots:
    bool enable();
    bool disable();

};

#endif // BLUETOOTHLOWENERGYMANAGER_H
