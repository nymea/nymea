/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef AVEABULB_H
#define AVEABULB_H

#include <QObject>
#include <QQueue>

#include "typeutils.h"
#include "bluetooth/bluetoothlowenergydevice.h"
#include "commandrequest.h"

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

    QBluetoothUuid m_imageSeviceUuid;
    QLowEnergyService *m_imageService;

    QBluetoothUuid m_imageCharacteristicUuid;
    QLowEnergyCharacteristic m_imageCharacteristic;

    QBluetoothUuid m_colorCharacteristicUuid;
    QLowEnergyCharacteristic m_colorCharacteristic;

    bool m_isAvailable;

    QHash<QByteArray, ActionId> m_actions;

    QQueue<CommandRequest> m_commandQueue;
    CommandRequest m_currentRequest;
    bool m_queueRunning;

    QByteArray m_brigthness;
    QByteArray m_liveliness;

private slots:
    void serviceScanFinished();
    void onConnectionStatusChanged();

    // Color service
    void serviceStateChanged(const QLowEnergyService::ServiceState &state);
    void serviceCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void confirmedCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void confirmedDescriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value);
    void serviceError(const QLowEnergyService::ServiceError &error);

    void enqueueCommand(QLowEnergyService *service, const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void sendNextCommand();

public slots:
    bool actionPowerOff(ActionId actionId);
    bool setWhite(ActionId actionId);
    bool setRed(ActionId actionId);
    bool setGreen(ActionId actionId);
    bool setBlue(ActionId actionId);
    bool setYellow(ActionId actionId);
    bool setPurple(ActionId actionId);
    bool setOrange(ActionId actionId);

    bool setCalmProvence(ActionId actionId);
    bool setCozyFlames(ActionId actionId);
    bool setCherryBlossom(ActionId actionId);
    bool setMountainBreeze(ActionId actionId);
    bool setNorthernGlow(ActionId actionId);
    bool setFairyWoods(ActionId actionId);
    bool setMagicHour(ActionId actionId);

};

#endif // AVEABULB_H
