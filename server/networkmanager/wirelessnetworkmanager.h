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

#ifndef WIRELESSNETWORKMANAGER_H
#define WIRELESSNETWORKMANAGER_H

#include <QObject>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusContext>
#include <QDBusArgument>

#include "wirelessaccesspoint.h"

namespace guhserver {

class WirelessNetworkManager : public QObject
{
    Q_OBJECT
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

    explicit WirelessNetworkManager(const QDBusObjectPath &devicePath, QObject *parent = 0);

    QString udi() const;
    QString macAddress() const;
    QString interfaceName() const;
    QString driver() const;
    QString driverVersion() const;

    bool connected() const;
    bool managed() const;
    DeviceState state() const;
    DeviceStateReason stateReason() const;

    void scanWirelessNetworks();

    QList<WirelessAccessPoint *> accessPoints();
    WirelessAccessPoint * getAccessPoint(const QString &ssid);
    WirelessAccessPoint * getAccessPoint(const QDBusObjectPath &objectPath);

    static QString deviceStateToString(const DeviceState &state);
    static QString deviceStateReasonToString(const DeviceStateReason &stateReason);

private:
    QString m_path;

    QString m_udi;
    QString m_macAddress;
    QString m_interfaceName;
    QString m_driver;
    QString m_driverVersion;

    // Can change
    bool m_connected;
    bool m_managed;
    DeviceState m_state;
    DeviceStateReason m_stateReason;

    QHash<QDBusObjectPath, WirelessAccessPoint *> m_accessPointsTable;

    void readAccessPoints();
    void readWirelessDeviceProperties();

    void setConnected(const bool &connected);
    void setState(const DeviceState &state);
    void setStateReason(const DeviceStateReason &stateReason);
    void setManaged(const bool &managed);

private slots:
    void deviceStateChanged(uint newState, uint oldState, uint reason);
    void accessPointAdded(const QDBusObjectPath &objectPath);
    void accessPointRemoved(const QDBusObjectPath &objectPath);
    void propertiesChanged(const QVariantMap &properties);

signals:
    void connectedChanged(const bool &connected);
    void managedChanged(const bool &managed);
    void stateChanged(const DeviceState &state);
};

QDebug operator<<(QDebug debug, WirelessNetworkManager *manager);

}

#endif // WIRELESSNETWORKMANAGER_H
