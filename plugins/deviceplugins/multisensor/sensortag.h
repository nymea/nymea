#ifndef SENSORTAG_H
#define SENSORTAG_H

#ifdef BLUETOOTH_LE

#include <QHash>
#include <QSharedPointer>
#include <QLowEnergyService>
#include "bluetooth/bluetoothlowenergydevice.h"

class SensorTag : public BluetoothLowEnergyDevice
{
    Q_OBJECT
public:
    explicit SensorTag(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent = 0);

signals:
    void valueChanged(double value);

private:
    //QBluetoothUuid m_temperatureServiceUuid;
    //QPointer<QLowEnergyService> m_temperatureService;
    //QPointer<QLowEnergyService> m_temperatureService2;
    QHash<QBluetoothUuid, QSharedPointer<QLowEnergyService>> m_services{{QBluetoothUuid(QUuid("f000aa20-0451-4000-b000-000000000000")), QSharedPointer<QLowEnergyService>()}};

    //QBluetoothUuid m_temperatureCharacteristicUuid;
    //QLowEnergyCharacteristic m_temperatureCharacteristic;

    //QHash<QByteArray, ActionId> m_actions;

    //QQueue<CommandRequest> m_commandQueue;
    //CommandRequest m_currentRequest;
    //bool m_queueRunning;

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
