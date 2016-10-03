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

#include "networkmanagerhandler.h"
#include "loggingcategories.h"
#include "guhcore.h"
#include "networkmanager/networkmanager.h"


namespace guhserver {

NetworkManagerHandler::NetworkManagerHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params;
    QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("GetWirelessAccessPoints", "Get the current list of wireless network access points.");
    setParams("GetWirelessAccessPoints", params);
    returns.insert("wirelessAccessPoints", QVariantList() << JsonTypes::wirelessAccessPointRef());
    returns.insert("o:networkError", QVariantList() << JsonTypes::wirelessAccessPointRef());
    setReturns("GetWirelessAccessPoints", returns);

    params.clear(); returns.clear();
    setDescription("GetNetworkStatus", "Get the current network manager status.");
    setParams("GetNetworkStatus", params);
    returns.insert("networkingEnabled", JsonTypes::basicTypeToString(QVariant::Bool));
    returns.insert("wirelessNetworkingEnabled", JsonTypes::basicTypeToString(QVariant::Bool));
    returns.insert("state", JsonTypes::networkManagerStateRef());
    setReturns("GetNetworkStatus", returns);

    params.clear(); returns.clear();
    setDescription("GetNetworkDevices", "Get the list of current network devices.");
    setParams("GetNetworkDevices", params);
    returns.insert("networkDevices",  QVariantList() << JsonTypes::networkDeviceRef());
    setReturns("GetNetworkDevices", returns);

    params.clear(); returns.clear();
    setDescription("ScanWifiNetworks", "Start a wifi scan for searching new networks.");
    setParams("ScanWifiNetworks", params);
    setReturns("ScanWifiNetworks", returns);

    params.clear(); returns.clear();
    setDescription("ConnectWifiNetwork", "Connect to the wifi network with the given ssid and password.");
    params.insert("ssid", JsonTypes::basicTypeToString(QVariant::String));
    params.insert("o:password", JsonTypes::basicTypeToString(QVariant::String));
    setParams("ConnectWifiNetwork", params);
    //returns.insert("networkDevices",  QVariantList() << JsonTypes::networkDeviceRef());
    setReturns("ConnectWifiNetwork", returns);
}

QString NetworkManagerHandler::name() const
{
    return "NetworkManager";
}

JsonReply *NetworkManagerHandler::GetNetworkStatus(const QVariantMap &params)
{
    Q_UNUSED(params);

    // TODO: check available
    // TODO: returns (networkManager error)

    QVariantMap returns;
    returns.insert("networkingEnabled", GuhCore::instance()->networkManager()->networkingEnabled());
    returns.insert("wirelessNetworkingEnabled", GuhCore::instance()->networkManager()->wirelessEnabled());
    returns.insert("state", JsonTypes::networkManagerStateToString(GuhCore::instance()->networkManager()->state()));
    return createReply(returns);
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
