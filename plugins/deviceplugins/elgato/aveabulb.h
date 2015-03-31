#ifndef AVEABULB_H
#define AVEABULB_H

#include <QObject>
#include "typeutils.h"
#include "bluetooth/bluetoothlowenergydevice.h"

class AveaBulb : public BluetoothLowEnergyDevice
{
    Q_OBJECT
public:
    explicit AveaBulb(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType, QObject *parent = 0);

    bool isAvailable();

signals:
    void availableChanged();
    void actionExecutionFinished(const ActionId &actionId, const bool &success);

private:
    QBluetoothUuid m_colorSeviceUuid;
    QLowEnergyService *m_colorService;

    QBluetoothUuid m_colorCharacteristicUuid;
    QLowEnergyCharacteristic m_colorCharacteristic;

    bool m_isAvailable;

    QHash<QByteArray, ActionId> m_actions;

private slots:
    void serviceScanFinished();
    void onConnectionStatusChanged();

    // Color service
    void serviceStateChanged(const QLowEnergyService::ServiceState &state);
    void serviceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void confirmedCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void serviceError(const QLowEnergyService::ServiceError &error);

    bool enableNotification();
    bool testMethod();

public slots:
    // actions
    bool actionPowerOff(ActionId actionId);
    bool setWhite(ActionId actionId);

};

#endif // AVEABULB_H
