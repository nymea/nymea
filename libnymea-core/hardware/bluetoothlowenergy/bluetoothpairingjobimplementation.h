#ifndef BLUETOOTHPAIRINGJOBIMPLEMENTATION_H
#define BLUETOOTHPAIRINGJOBIMPLEMENTATION_H

#include "hardware/bluetoothlowenergy/bluetoothlowenergymanager.h"
#include <QObject>

namespace nymeaserver
{

class NymeaBluetoothAgent;
class BluetoothLowEnergyManagerImplementation;

class BluetoothPairingJobImplementation : public BluetoothPairingJob
{
    Q_OBJECT
public:
    explicit BluetoothPairingJobImplementation(NymeaBluetoothAgent *agent, const QBluetoothAddress &address, QObject *parent = nullptr);

    bool isFinished() const override;
    bool success() const override;

    void passKeyEntered(const QString passKey) override;

private:
    friend BluetoothLowEnergyManagerImplementation;
    void finish(bool success);

private:
    bool m_finished = false;
    bool m_success = false;

    NymeaBluetoothAgent *m_agent = nullptr;
    QBluetoothAddress m_address;

};

}
#endif // BLUETOOTHPAIRINGJOBIMPLEMENTATION_H
