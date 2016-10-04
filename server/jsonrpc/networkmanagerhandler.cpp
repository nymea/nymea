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

#include "guhcore.h"
#include "jsontypes.h"
#include "loggingcategories.h"
#include "networkmanagerhandler.h"
#include "networkmanager/networkmanager.h"


namespace guhserver {

NetworkManagerHandler::NetworkManagerHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params;
    QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("GetNetworkStatus", "Get the current network manager status.");
    setParams("GetNetworkStatus", params);
    QVariantMap status;
    status.insert("networkingEnabled", JsonTypes::basicTypeToString(QVariant::Bool));
    status.insert("wirelessNetworkingEnabled", JsonTypes::basicTypeToString(QVariant::Bool));
    status.insert("state", JsonTypes::networkManagerStateRef());
    returns.insert("o:status", status);
    returns.insert("o:networkManagerError", JsonTypes::networkManagerErrorRef());
    setReturns("GetNetworkStatus", returns);

    params.clear(); returns.clear();
    setDescription("EnableNetworking", "Enable or disable networking in the NetworkManager.");
    params.insert("enable", JsonTypes::basicTypeToString(QVariant::Bool));
    setParams("EnableNetworking", params);
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorRef());
    setReturns("EnableNetworking", returns);

    params.clear(); returns.clear();
    setDescription("EnableWirelessNetworking", "Enable or disable wireless networking in the NetworkManager.");
    params.insert("enable", JsonTypes::basicTypeToString(QVariant::Bool));
    setParams("EnableWirelessNetworking", params);
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorRef());
    setReturns("EnableWirelessNetworking", returns);

    params.clear(); returns.clear();
    setDescription("GetWirelessAccessPoints", "Get the current list of wireless network access points.");
    setParams("GetWirelessAccessPoints", params);
    returns.insert("o:wirelessAccessPoints", QVariantList() << JsonTypes::wirelessAccessPointRef());
    returns.insert("o:networkManagerError", JsonTypes::networkManagerErrorRef());
    setReturns("GetWirelessAccessPoints", returns);

    params.clear(); returns.clear();
    setDescription("GetNetworkDevices", "Get the list of current network devices.");
    setParams("GetNetworkDevices", params);
    returns.insert("o:networkDevices",  QVariantList() << JsonTypes::networkDeviceRef());
    returns.insert("o:networkManagerError", JsonTypes::networkManagerErrorRef());
    setReturns("GetNetworkDevices", returns);

    params.clear(); returns.clear();
    setDescription("ScanWifiNetworks", "Start a wifi scan for searching new networks.");
    setParams("ScanWifiNetworks", params);
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorRef());
    setReturns("ScanWifiNetworks", returns);

    params.clear(); returns.clear();
    setDescription("ConnectWifiNetwork", "Connect to the wifi network with the given ssid and password.");
    params.insert("ssid", JsonTypes::basicTypeToString(QVariant::String));
    params.insert("o:password", JsonTypes::basicTypeToString(QVariant::String));
    setParams("ConnectWifiNetwork", params);
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorRef());
    setReturns("ConnectWifiNetwork", returns);
}

QString NetworkManagerHandler::name() const
{
    return "NetworkManager";
}

JsonReply *NetworkManagerHandler::GetNetworkStatus(const QVariantMap &params)
{
    Q_UNUSED(params);

    // Check available
    if (!GuhCore::instance()->networkManager()->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    // Pack network manager status
    QVariantMap returns; QVariantMap status;
    status.insert("networkingEnabled", GuhCore::instance()->networkManager()->networkingEnabled());
    status.insert("wirelessNetworkingEnabled", GuhCore::instance()->networkManager()->wirelessEnabled());
    status.insert("state", JsonTypes::networkManagerStateToString(GuhCore::instance()->networkManager()->state()));
    returns.insert("status", status);
    return createReply(returns);
}

JsonReply *NetworkManagerHandler::EnableNetworking(const QVariantMap &params)
{
    if (!GuhCore::instance()->networkManager()->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    bool enable = params.value("enable").toBool();

    if (!GuhCore::instance()->networkManager()->enableNetworking(enable))
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorUnknownError));

    return createReply(statusToReply(NetworkManager::NetworkManagerErrorNoError));
}

JsonReply *NetworkManagerHandler::EnableWirelessNetworking(const QVariantMap &params)
{
    if (!GuhCore::instance()->networkManager()->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    bool enable = params.value("enable").toBool();

    if (!GuhCore::instance()->networkManager()->enableWireless(enable))
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorUnknownError));

    return createReply(statusToReply(NetworkManager::NetworkManagerErrorNoError));
}

JsonReply *NetworkManagerHandler::GetWirelessAccessPoints(const QVariantMap &params)
{
    Q_UNUSED(params);

    QVariantList wirelessAccessPoints;

    // TODO: check available
    // TODO: returns (networkManager error)

    foreach (WirelessAccessPoint *wirelessAccessPoint, GuhCore::instance()->networkManager()->wirelessNetworkManager()->accessPoints())
        wirelessAccessPoints.append(JsonTypes::packWirelessAccessPoint(wirelessAccessPoint));

    QVariantMap returns;
    returns.insert("wirelessAccessPoints", wirelessAccessPoints);
    return createReply(returns);
}

JsonReply *NetworkManagerHandler::GetNetworkDevices(const QVariantMap &params)
{
    Q_UNUSED(params);

    QVariantList networkDevices;

    // TODO: check available
    // TODO: returns (networkManager error)

    foreach (NetworkDevice *networkDevice, GuhCore::instance()->networkManager()->networkDevices())
        networkDevices.append(JsonTypes::packNetworkDevice(networkDevice));

    QVariantMap returns;
    returns.insert("networkDevices", networkDevices);
    return createReply(returns);
}

JsonReply *NetworkManagerHandler::ScanWifiNetworks(const QVariantMap &params)
{
    Q_UNUSED(params);

    // TODO: check available
    // TODO: returns (networkManager error)

    GuhCore::instance()->networkManager()->wirelessNetworkManager()->scanWirelessNetworks();

    QVariantMap returns;
    return createReply(returns);
}

JsonReply *NetworkManagerHandler::ConnectWifiNetwork(const QVariantMap &params)
{
    QString ssid = params.value("ssid").toString();
    QString password = params.value("password").toString();

    // TODO: check available
    // TODO: returns (networkManager error)

    GuhCore::instance()->networkManager()->connectWifi(ssid, password);

    QVariantMap returns;
    return createReply(returns);
}

}
