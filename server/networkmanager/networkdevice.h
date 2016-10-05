/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef NETWORKDEVICE_H
#define NETWORKDEVICE_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusContext>
#include <QDBusArgument>

#include "dbus-interfaces.h"

namespace guhserver {

class NetworkDevice : public QObject
{
    Q_OBJECT
    Q_ENUMS(DeviceType)
    Q_ENUMS(NetworkDeviceState)
    Q_ENUMS(NetworkDeviceStateReason)

public:
    enum NetworkDeviceState {
        NetworkDeviceStateUnknown = 0,
        NetworkDeviceStateUnmanaged = 10,
        NetworkDeviceStateUnavailable = 20,
        NetworkDeviceStateDisconnected = 30,
        NetworkDeviceStatePrepare = 40,
        NetworkDeviceStateConfig = 50,
        NetworkDeviceStateNeedAuth = 60,
        NetworkDeviceStateIpConfig = 70,
        NetworkDeviceStateIpCheck = 80,
        NetworkDeviceStateSecondaries = 90,
        NetworkDeviceStateActivated = 100,
        NetworkDeviceStateDeactivating = 110,
        NetworkDeviceStateFailed = 120
    };

    enum NetworkDeviceStateReason {
        NetworkDeviceStateReasonNone = 0,
        NetworkDeviceStateReasonUnknown = 1,
        NetworkDeviceStateReasonNowManaged = 2,
        NetworkDeviceStateReasonNowUnmanaged = 3,
        NetworkDeviceStateReasonConfigFailed = 4,
        NetworkDeviceStateReasonIpConfigUnavailable = 5,
        NetworkDeviceStateReasonIpConfigExpired = 6,
        NetworkDeviceStateReasonNoSecrets = 7,
        NetworkDeviceStateReasonSupplicantDisconnected = 8,
        NetworkDeviceStateReasonSupplicantConfigFailed = 9,
        NetworkDeviceStateReasonSupplicantFailed = 10,
        NetworkDeviceStateReasonSupplicantTimeout = 11,
        NetworkDeviceStateReasonPppStartFailed = 12,
        NetworkDeviceStateReasonPppDisconnected = 13,
        NetworkDeviceStateReasonPppFailed = 14,
        NetworkDeviceStateReasonDhcpStartFailed = 15,
        NetworkDeviceStateReasonDhcpError = 16,
        NetworkDeviceStateReasonDhcpFailed = 17,
        NetworkDeviceStateReasonSharedStartFailed = 18,
        NetworkDeviceStateReasonSharedFailed = 19,
        NetworkDeviceStateReasonAutoIpStartFailed = 20,
        NetworkDeviceStateReasonAutoIpError = 21,
        NetworkDeviceStateReasonAutoIpFailed = 22,
        NetworkDeviceStateReasonModemBusy = 23,
        NetworkDeviceStateReasonModemNoDialTone = 24,
        NetworkDeviceStateReasonModemNoCarrier = 25,
        NetworkDeviceStateReasonModemDialTimeout = 26,
        NetworkDeviceStateReasonModemDialFailed = 27,
        NetworkDeviceStateReasonModemInitFailed = 28,
        NetworkDeviceStateReasonGsmApnFailed = 29,
        NetworkDeviceStateReasonGsmRegistrationNotSearching = 30,
        NetworkDeviceStateReasonGsmRegistrationDenied = 31,
        NetworkDeviceStateReasonGsmRegistrationTimeout = 32,
        NetworkDeviceStateReasonGsmRegistrationFailed = 33,
        NetworkDeviceStateReasonGsmPinCheckFailed = 34,
        NetworkDeviceStateReasonFirmwareMissing = 35,
        NetworkDeviceStateReasonRemoved = 36,
        NetworkDeviceStateReasonSleeping = 37,
        NetworkDeviceStateReasonConnectionRemoved = 38,
        NetworkDeviceStateReasonUserRequest = 39,
        NetworkDeviceStateReasonCarrier = 40,
        NetworkDeviceStateReasonConnectionAssumed = 41,
        NetworkDeviceStateReasonSupplicantAvailable = 42,
        NetworkDeviceStateReasonModemNotFound = 43,
        NetworkDeviceStateReasonBtFailed = 44,
        NetworkDeviceStateReasonGsmSimNotInserted = 45,
        NetworkDeviceStateReasonGsmSimPinRequired = 46,
        NetworkDeviceStateReasonGsmSimPukRequired = 47,
        NetworkDeviceStateReasonGsmSimWrong = 48,
        NetworkDeviceStateReasonInfinibandMode = 49,
        NetworkDeviceStateReasonDependencyFailed = 50,
        NetworkDeviceStateReasonBR2684Failed = 51,
        NetworkDeviceStateReasonModemManagerUnavailable = 52,
        NetworkDeviceStateReasonSsidNotFound = 53,
        NetworkDeviceStateReasonSecondaryConnectionFailed = 54,
        NetworkDeviceStateReasonDcbFoecFailed = 55,
        NetworkDeviceStateReasonTeamdControlFailed = 56,
        NetworkDeviceStateReasonModemFailed = 57,
        NetworkDeviceStateReasonModemAvailable = 58,
        NetworkDeviceStateReasonSimPinIncorrect = 59,
        NetworkDeviceStateReasonNewActivision = 60,
        NetworkDeviceStateReasonParentChanged = 61,
        NetworkDeviceStateReasonParentManagedChanged = 62
    };

    enum DeviceType {
        DeviceTypeUnknown = 0,
        DeviceTypeEthernet = 1,
        DeviceTypeWifi = 2,
        DeviceTypeBluetooth = 5,
        DeviceTypeOlpcMesh = 6,
        DeviceTypeWiMax = 7,
        DeviceTypeModem = 8,
        DeviceTypeInfiniBand = 9,
        DeviceTypeBond = 10,
        DeviceTypeVLan = 11,
        DeviceTypeAdsl = 12,
        DeviceTypeBridge = 13,
        DeviceTypeGeneric = 14,
        DeviceTypeTeam = 15,
        DeviceTypeTun = 16,
        DeviceTypeIpTunnel = 17,
        DeviceTypeMacVLan = 18,
        DeviceTypeVXLan = 19,
        DeviceTypeVEth = 20,
    };

    explicit NetworkDevice(const QDBusObjectPath &objectPath, QObject *parent = 0);

    QDBusObjectPath objectPath() const;

    QString udi() const;
    QString interface() const;
    QString ipInterface() const;
    QString driver() const;
    QString driverVersion() const;
    QString firmwareVersion() const;
    QString physicalPortId() const;
    uint mtu() const;
    uint metered() const;

    NetworkDeviceState deviceState() const;
    QString deviceStateString() const;
    NetworkDeviceStateReason deviceStateReason() const;
    DeviceType deviceType() const;

    QDBusObjectPath activeConnection() const;
    QDBusObjectPath ip4Config() const;
    QList<QDBusObjectPath> availableConnections() const;

    static QString deviceTypeToString(const DeviceType &deviceType);
    static QString deviceStateToString(const NetworkDeviceState &deviceState);
    static QString deviceStateReasonToString(const NetworkDeviceStateReason &deviceStateReason);
private:
    QDBusObjectPath m_objectPath;

    // Device properties
    QString m_udi;
    QString m_interface;
    QString m_ipInterface;
    QString m_driver;
    QString m_driverVersion;
    QString m_firmwareVersion;
    QString m_physicalPortId;
    uint m_mtu;
    uint m_metered;
    NetworkDeviceState m_deviceState;
    NetworkDeviceStateReason m_deviceStateReason;
    DeviceType m_deviceType;

    QDBusObjectPath m_activeConnection;
    QDBusObjectPath m_ip4Config;
    QDBusObjectPath m_ip6Config;

    QList<QDBusObjectPath> m_availableConnections;

private slots:
    void onStateChanged(uint newState, uint oldState, uint reason);

signals:
    void deviceStateChanged();

};

QDebug operator<<(QDebug debug, NetworkDevice *device);

}

#endif // NETWORKDEVICE_H
