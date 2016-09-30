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
    Q_ENUMS(DeviceState)
    Q_ENUMS(DeviceStateReason)

public:
    enum DeviceState {
        DeviceStateUnknown = 0,
        DeviceStateUnmanaged = 10,
        DeviceStateUnavailable = 20,
        DeviceStateDisconnected = 30,
        DeviceStatePrepare = 40,
        DeviceStateConfig = 50,
        DeviceStateNeedAuth = 60,
        DeviceStateIpConfig = 70,
        DeviceStateIpCheck = 80,
        DeviceStateSecondaries = 90,
        DeviceStateActivated = 100,
        DeviceStateDeactivating = 110,
        DeviceStateFailed = 120
    };

    enum DeviceStateReason {
        DeviceStateReasonNone = 0,
        DeviceStateReasonUnknown = 1,
        DeviceStateReasonNowManaged = 2,
        DeviceStateReasonNowUnmanaged = 3,
        DeviceStateReasonConfigFailed = 4,
        DeviceStateReasonIpConfigUnavailable = 5,
        DeviceStateReasonIpConfigExpired = 6,
        DeviceStateReasonNoSecrets = 7,
        DeviceStateReasonSupplicantDisconnected = 8,
        DeviceStateReasonSupplicantConfigFailed = 9,
        DeviceStateReasonSupplicantFailed = 10,
        DeviceStateReasonSupplicantTimeout = 11,
        DeviceStateReasonPppStartFailed = 12,
        DeviceStateReasonPppDisconnected = 13,
        DeviceStateReasonPppFailed = 14,
        DeviceStateReasonDhcpStartFailed = 15,
        DeviceStateReasonDhcpError = 16,
        DeviceStateReasonDhcpFailed = 17,
        DeviceStateReasonSharedStartFailed = 18,
        DeviceStateReasonSharedFailed = 19,
        DeviceStateReasonAutoIpStartFailed = 20,
        DeviceStateReasonAutoIpError = 21,
        DeviceStateReasonAutoIpFailed = 22,
        DeviceStateReasonModemBusy = 23,
        DeviceStateReasonModemNoDialTone = 24,
        DeviceStateReasonModemNoCarrier = 25,
        DeviceStateReasonModemDialTimeout = 26,
        DeviceStateReasonModemDialFailed = 27,
        DeviceStateReasonModemInitFailed = 28,
        DeviceStateReasonGsmApnFailed = 29,
        DeviceStateReasonGsmRegistrationNotSearching = 30,
        DeviceStateReasonGsmRegistrationDenied = 31,
        DeviceStateReasonGsmRegistrationTimeout = 32,
        DeviceStateReasonGsmRegistrationFailed = 33,
        DeviceStateReasonGsmPinCheckFailed = 34,
        DeviceStateReasonFirmwareMissing = 35,
        DeviceStateReasonRemoved = 36,
        DeviceStateReasonSleeping = 37,
        DeviceStateReasonConnectionRemoved = 38,
        DeviceStateReasonUserRequest = 39,
        DeviceStateReasonCarrier = 40,
        DeviceStateReasonConnectionAssumed = 41,
        DeviceStateReasonSupplicantAvailable = 42,
        DeviceStateReasonModemNotFound = 43,
        DeviceStateReasonBtFailed = 44,
        DeviceStateReasonGsmSimNotInserted = 45,
        DeviceStateReasonGsmSimPinRequired = 46,
        DeviceStateReasonGsmSimPukRequired = 47,
        DeviceStateReasonGsmSimWrong = 48,
        DeviceStateReasonInfinibandMode = 49,
        DeviceStateReasonDependencyFailed = 50,
        DeviceStateReasonBR2684Failed = 51,
        DeviceStateReasonModemManagerUnavailable = 52,
        DeviceStateReasonSsidNotFound = 53,
        DeviceStateReasonSecondaryConnectionFailed = 54,
        DeviceStateReasonDcbFoecFailed = 55,
        DeviceStateReasonTeamdControlFailed = 56,
        DeviceStateReasonModemFailed = 57,
        DeviceStateReasonModemAvailable = 58,
        DeviceStateReasonSimPinIncorrect = 59,
        DeviceStateReasonNewActivision = 60,
        DeviceStateReasonParentChanged = 61,
        DeviceStateReasonParentManagedChanged = 62
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
    DeviceState deviceState() const;
    DeviceStateReason deviceStateReason() const;
    DeviceType deviceType() const;

    QDBusObjectPath activeConnection() const;
    QDBusObjectPath ip4Config() const;
    QList<QDBusObjectPath> availableConnections() const;

    static QString deviceTypeToString(const DeviceType &deviceType);
    static QString deviceStateToString(const DeviceState &deviceState);
    static QString deviceStateReasonToString(const DeviceStateReason &deviceStateReason);


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
    DeviceState m_deviceState;
    DeviceStateReason m_deviceStateReason;
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
