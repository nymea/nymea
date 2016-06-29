#ifndef SENSORTAG_H
#define SENSORTAG_H

#ifdef BLUETOOTH_LE

#include <QHash>
#include <QSharedPointer>
#include <QLowEnergyService>
#include "bluetooth/bluetoothlowenergydevice.h"
#include "extern-plugininfo.h"

class SensorTag : public BluetoothLowEnergyDevice
{
    Q_OBJECT
public:
    explicit SensorTag(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent = 0);

signals:
    void valueChanged(StateTypeId state, double value);

private:
    QHash<QBluetoothUuid, QSharedPointer<QLowEnergyService>> m_services{
        {QBluetoothUuid(QUuid("f000aa00-0451-4000-b000-000000000000")), QSharedPointer<QLowEnergyService>()},
        {QBluetoothUuid(QUuid("f000aa20-0451-4000-b000-000000000000")), QSharedPointer<QLowEnergyService>()},
        {QBluetoothUuid(QUuid("f000aa40-0451-4000-b000-000000000000")), QSharedPointer<QLowEnergyService>()}
    };

    //QHash<QByteArray, ActionId> m_actions;

    //QQueue<CommandRequest> m_commandQueue;
    //CommandRequest m_currentRequest;
    //bool m_queueRunning;

    QVector<quint16> m_c;
    QVector<qint16> m_c2;

private slots:
    void setupServices();
    void onConnectionStatusChanged();

    // Service
    void onServiceStateChanged(const QLowEnergyService::ServiceState &state);
    void onServiceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    //void confirmedCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    //void confirmedDescriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value);
    void onServiceError(const QLowEnergyService::ServiceError &error);
};
#endif // BLUETOOTH_LE

#endif // SENSORTAG_H
