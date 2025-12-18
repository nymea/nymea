// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef NETWORKMANAGERHANDLER_H
#define NETWORKMANAGERHANDLER_H

#include <QObject>

#include "jsonrpc/jsonhandler.h"
#include <networkmanager.h>

namespace nymeaserver {

class NetworkManagerHandler : public JsonHandler
{
    Q_OBJECT
public:
    enum WiredNetworkConnectionType { WiredNetworkConnectionTypeDHCP, WiredNetworkConnectionTypeManual, WiredNetworkConnectionTypeShared };
    Q_ENUM(WiredNetworkConnectionType)

    explicit NetworkManagerHandler(NetworkManager *networkManager, QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *GetNetworkStatus(const QVariantMap &params);
    Q_INVOKABLE JsonReply *EnableNetworking(const QVariantMap &params);
    Q_INVOKABLE JsonReply *EnableWirelessNetworking(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetWirelessAccessPoints(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetNetworkDevices(const QVariantMap &params);
    Q_INVOKABLE JsonReply *CreateWiredConnection(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ScanWifiNetworks(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ConnectWifiNetwork(const QVariantMap &params);
    Q_INVOKABLE JsonReply *DisconnectInterface(const QVariantMap &params);
    Q_INVOKABLE JsonReply *StartAccessPoint(const QVariantMap &params);

signals:
    // NetworkManager
    void NetworkStatusChanged(const QVariantMap &params);

    // NetworkDevices
    void WiredNetworkDeviceAdded(const QVariantMap &params);
    void WiredNetworkDeviceRemoved(const QVariantMap &params);
    void WiredNetworkDeviceChanged(const QVariantMap &params);

    void WirelessNetworkDeviceAdded(const QVariantMap &params);
    void WirelessNetworkDeviceRemoved(const QVariantMap &params);
    void WirelessNetworkDeviceChanged(const QVariantMap &params);

private slots:
    // NetworkManager
    void onNetworkManagerStatusChanged();

    // NetworkDevices
    void onWirelessNetworkDeviceAdded(WirelessNetworkDevice *networkDevice);
    void onWirelessNetworkDeviceRemoved(const QString &interface);
    void onWirelessNetworkDeviceChanged(WirelessNetworkDevice *networkDevice);

    void onWiredNetworkDeviceAdded(WiredNetworkDevice *networkDevice);
    void onWiredNetworkDeviceRemoved(const QString &interface);
    void onWiredNetworkDeviceChanged(WiredNetworkDevice *networkDevice);

private:
    QVariantMap packNetworkManagerStatus();
    QVariantMap packWirelessAccessPoint(WirelessAccessPoint *wirelessAccessPoint);
    QVariantMap packWiredNetworkDevice(WiredNetworkDevice *networkDevice);
    QVariantMap packWirelessNetworkDevice(WirelessNetworkDevice *networkDevice);

    QVariantMap statusToReply(NetworkManager::NetworkManagerError status) const;

    NetworkManager *m_networkManager = nullptr;
};

} // namespace nymeaserver

#endif // NETWORKMANAGERHANDLER_H
