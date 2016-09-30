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

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusContext>
#include <QDBusArgument>

#include "wirelessnetworkmanager.h"
#include "dbus-interfaces.h"
#include "networkdevice.h"
#include "networksettings.h"

// Docs: https://developer.gnome.org/NetworkManager/unstable/spec.html

namespace guhserver {

class NetworkManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(NetworkManagerState)
    Q_ENUMS(NetworkManagerConnectivityState)

public:
    enum NetworkManagerState {
        NetworkManagerStateUnknown = 0,
        NetworkManagerStateAsleep = 10,
        NetworkManagerStateDisconnected = 20,
        NetworkManagerStateDisconnecting = 30,
        NetworkManagerStateConnecting = 40,
        NetworkManagerStateConnectedLocal = 50,
        NetworkManagerStateConnectedSite = 60,
        NetworkManagerStateConnectedGlobal = 70
    };

    enum NetworkManagerConnectivityState {
        NetworkManagerConnectivityStateUnknown = 1,
        NetworkManagerConnectivityStateNone = 2,
        NetworkManagerConnectivityStatePortal = 3,
        NetworkManagerConnectivityStateLimited = 4,
        NetworkManagerConnectivityStateFull = 5
    };

    explicit NetworkManager(QObject *parent = 0);

    static bool available();
    static QString networkManagerStateToString(const NetworkManagerState &state);
    static QString networkManagerConnectivityStateToString(const NetworkManagerConnectivityState &state);

    QList<NetworkDevice *> networkDevices() const;

    WirelessNetworkManager *wirelessNetworkManager() const;

    // Properties
    QString version() const;
    NetworkManagerState state() const;
    NetworkManagerConnectivityState connectivityState() const;

    // Networking
    bool networkingEnabled() const;
    bool enableNetworking();
    bool disableNetworking();

    // Wireless Networking
    bool wirelessEnabled() const;
    bool enableWireless();
    bool disableWireless();

private:
    QDBusInterface *m_networkManagerInterface;
    QHash<QDBusObjectPath, NetworkDevice *> m_networkDevices;

    NetworkSettings *m_networkSettings;
    WirelessNetworkManager *m_wirelessNetworkManager;

    QString m_version;

    NetworkManagerState m_state;
    NetworkManagerConnectivityState m_connectivityState;
    bool m_networkingEnabled;
    bool m_wirelessEnabled;

    void loadDevices();

    void setVersion(const QString &version);
    void setNetworkingEnabled(const bool &enabled);
    void setWirelessEnabled(const bool &enabled);
    void setConnectivityState(const NetworkManagerConnectivityState &connectivityState);
    void setState(const NetworkManagerState &state);

signals:
    void versionChanged();
    void networkingEnabledChanged();
    void wirelessEnabledChanged();
    void stateChanged();
    void connectivityStateChanged();

private slots:
    void onDeviceAdded(const QDBusObjectPath &deviceObjectPath);
    void onDeviceRemoved(const QDBusObjectPath &deviceObjectPath);
    void onPropertiesChanged(const QVariantMap &properties);

};

}

#endif // NETWORKMANAGER_H
