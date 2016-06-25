#ifdef BLUETOOTH_LE

#include "sensortag.h"

SensorTag::SensorTag(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent) :
    BluetoothLowEnergyDevice(deviceInfo, addressType, parent)
{
}

#endif // BLUETOOTH_LE
