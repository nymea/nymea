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

    explicit WirelessNetworkManager(const QDBusObjectPath &objectPath, QObject *parent = 0);

    QDBusObjectPath objectPath() const;

    QString udi() const;
    QString macAddress() const;
    QString interfaceName() const;
    QString driver() const;
    QString driverVersion() const;

    bool connected() const;
    bool managed() const;
    NetworkDeviceState state() const;
    NetworkDeviceStateReason stateReason() const;

    void scanWirelessNetworks();

    QList<WirelessAccessPoint *> accessPoints();
    WirelessAccessPoint *getAccessPoint(const QString &ssid);
    WirelessAccessPoint *getAccessPoint(const QDBusObjectPath &objectPath);

    static QString deviceStateToString(const NetworkDeviceState &state);
    static QString deviceStateReasonToString(const NetworkDeviceStateReason &stateReason);

private:
    QDBusObjectPath m_objectPath;

    QString m_udi;
    QString m_macAddress;
    QString m_interfaceName;
    QString m_driver;
    QString m_driverVersion;

    // Can change
    bool m_connected;
    bool m_managed;
    NetworkDeviceState m_state;
    NetworkDeviceStateReason m_stateReason;

    QHash<QDBusObjectPath, WirelessAccessPoint *> m_accessPointsTable;

    void readAccessPoints();
    void readWirelessDeviceProperties();

    void setConnected(const bool &connected);
    void setState(const NetworkDeviceState &state);
    void setStateReason(const NetworkDeviceStateReason &stateReason);
    void setManaged(const bool &managed);

private slots:
    void deviceStateChanged(uint newState, uint oldState, uint reason);
    void accessPointAdded(const QDBusObjectPath &objectPath);
    void accessPointRemoved(const QDBusObjectPath &objectPath);
    void propertiesChanged(const QVariantMap &properties);

signals:
    void connectedChanged(const bool &connected);
    void managedChanged(const bool &managed);
    void stateChanged(const NetworkDeviceState &state);
};

QDebug operator<<(QDebug debug, WirelessNetworkManager *manager);

}

#endif // WIRELESSNETWORKMANAGER_H
