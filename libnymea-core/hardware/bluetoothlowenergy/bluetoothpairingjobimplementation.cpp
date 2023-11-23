#include "bluetoothpairingjobimplementation.h"

#include "nymeabluetoothagent.h"

namespace nymeaserver
{

BluetoothPairingJobImplementation::BluetoothPairingJobImplementation(NymeaBluetoothAgent *agent, const QBluetoothAddress &address, QObject *parent)
    : BluetoothPairingJob{address, parent},
      m_agent{agent},
      m_address{address}
{
    connect(m_agent, &NymeaBluetoothAgent::passKeyRequested, this, [address, this](const QBluetoothAddress &addr){
        if (address != addr) {
            // Not for us...
            return;
        }
        emit passKeyRequested();
    });
    connect(m_agent, &NymeaBluetoothAgent::displayPinCode, this, [address, this](const QBluetoothAddress &addr, const QString &pinCode){
        if (address != addr) {
            // Not for us...
            return;
        }
        emit displayPinCode(pinCode);
    });

}

bool BluetoothPairingJobImplementation::isFinished() const
{
    return m_finished;
}

bool BluetoothPairingJobImplementation::success() const
{
    return m_success;
}

void BluetoothPairingJobImplementation::passKeyEntered(const QString passKey)
{
    m_agent->passKeyEntered(m_address, passKey);
}

void BluetoothPairingJobImplementation::finish(bool success)
{
    m_finished = true;
    m_success = success;
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection, Q_ARG(bool, success));
}

}
