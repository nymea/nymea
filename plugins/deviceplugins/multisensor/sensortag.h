#ifndef SENSORTAG_H
#define SENSORTAG_H

#ifdef BLUETOOTH_LE

#include "bluetooth/bluetoothlowenergydevice.h"

class SensorTag : public BluetoothLowEnergyDevice
{
    Q_OBJECT
public:
    explicit SensorTag(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent = 0);

};
#endif // BLUETOOTH_LE

#endif // SENSORTAG_H
