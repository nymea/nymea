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

#ifndef BLUETOOTHLOWENERGYDEVICE_H
#define BLUETOOTHLOWENERGYDEVICE_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QBluetoothServiceInfo>
#include <QLowEnergyController>

class BluetoothLowEnergyDevice : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothLowEnergyDevice(const QBluetoothDeviceInfo &deviceInfo, const QLowEnergyController::RemoteAddressType &addressType = QLowEnergyController::PublicAddress, QObject *parent = 0);

    QString name() const;
    QBluetoothAddress address() const;

    void connectDevice();
    void reconnectDevice();
    void disconnectDevice();

    bool isConnected() const;
    bool isDiscovered() const;

protected:
    QLowEnergyController *controller() const;

private:
    QBluetoothDeviceInfo m_deviceInfo;
    QLowEnergyController *m_controller;

    bool m_connected;
    bool m_discovered;

signals:
    void connectionStatusChanged();
    void servicesDiscoveryFinished();

private slots:
    void connected();
    void disconnected();
    void deviceError(const QLowEnergyController::Error &error);
};

#endif // BLUETOOTHLOWENERGYDEVICE_H
