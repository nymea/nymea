/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
    explicit NetworkManagerHandler(NetworkManager *networkManager, QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *GetNetworkStatus(const QVariantMap &params);
    Q_INVOKABLE JsonReply *EnableNetworking(const QVariantMap &params);
    Q_INVOKABLE JsonReply *EnableWirelessNetworking(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetWirelessAccessPoints(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetNetworkDevices(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ScanWifiNetworks(const QVariantMap &params);
    Q_INVOKABLE JsonReply *ConnectWifiNetwork(const QVariantMap &params);
    Q_INVOKABLE JsonReply *DisconnectInterface(const QVariantMap &params);
    Q_INVOKABLE JsonReply *StartAccessPoint(const QVariantMap &params);

private:
    QVariantMap packNetworkManagerStatus();

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
    QVariantMap packWirelessAccessPoint(WirelessAccessPoint *wirelessAccessPoint);
    QVariantMap packWiredNetworkDevice(WiredNetworkDevice *networkDevice);
    QVariantMap packWirelessNetworkDevice(WirelessNetworkDevice *networkDevice);

    QVariantMap statusToReply(NetworkManager::NetworkManagerError status) const;

    NetworkManager* m_networkManager = nullptr;

};

}

#endif // NETWORKMANAGERHANDLER_H
