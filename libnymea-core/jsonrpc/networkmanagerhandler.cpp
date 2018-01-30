/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::NetworkManagerHandler
    \brief This subclass of \l{JsonHandler} processes the JSON requests for the \tt NetworkManager namespace of the JSON-RPC API.

    \ingroup json
    \inmodule core

    This \l{JsonHandler} will be created in the \l{JsonRPCServer} and used to handle JSON-RPC requests
    for the \tt {NetworkManager} namespace of the API.

    \sa NetworkManager, JsonHandler, JsonRPCServer
*/

/*! \fn void nymeaserver::NetworkManagerHandler::NetworkStatusChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the state of the \l{NetworkManager} has changed.
    The \a params contains the map for the notification.
*/

/*! \fn void nymeaserver::NetworkManagerHandler::WiredNetworkDeviceAdded(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{WiredNetworkDevice} has been added to the \l{NetworkManager}.
    The \a params contains the map for the notification.
*/

/*! \fn void nymeaserver::NetworkManagerHandler::WiredNetworkDeviceRemoved(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{WiredNetworkDevice} has been removed from the \l{NetworkManager}.
    The \a params contains the map for the notification.
*/

/*! \fn void nymeaserver::NetworkManagerHandler::WiredNetworkDeviceChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{WiredNetworkDevice} has changed in the \l{NetworkManager}.
    The \a params contains the map for the notification.
*/

/*! \fn void nymeaserver::NetworkManagerHandler::WirelessNetworkDeviceAdded(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{WirelessNetworkDevice} has been added to the \l{NetworkManager}.
    The \a params contains the map for the notification.
*/

/*! \fn void nymeaserver::NetworkManagerHandler::WirelessNetworkDeviceRemoved(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{WirelessNetworkDevice} has been removed from the \l{NetworkManager}.
    The \a params contains the map for the notification.
*/

/*! \fn void nymeaserver::NetworkManagerHandler::WirelessNetworkDeviceChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when a \l{WirelessNetworkDevice} has changed in the \l{NetworkManager}.
    The \a params contains the map for the notification.
*/


#include "nymeacore.h"
#include "jsontypes.h"
#include "loggingcategories.h"
#include "networkmanagerhandler.h"
#include "networkmanager/networkmanager.h"


namespace nymeaserver {

/*! Constructs a new \l{NetworkManagerHandler} with the given \a parent. */
NetworkManagerHandler::NetworkManagerHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params; QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("GetNetworkStatus", "Get the current network manager status.");
    setParams("GetNetworkStatus", params);
    QVariantMap status;
    status.insert("networkingEnabled", JsonTypes::basicTypeToString(QVariant::Bool));
    status.insert("wirelessNetworkingEnabled", JsonTypes::basicTypeToString(QVariant::Bool));
    status.insert("state", JsonTypes::networkManagerStateRef());
    returns.insert("o:status", status);
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorRef());
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
    setDescription("GetWirelessAccessPoints", "Get the current list of wireless network access points for the given interface. The interface has to be a WirelessNetworkDevice.");
    params.insert("interface", JsonTypes::basicTypeToString(QVariant::String));
    setParams("GetWirelessAccessPoints", params);
    returns.insert("o:wirelessAccessPoints", QVariantList() << JsonTypes::wirelessAccessPointRef());
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorRef());
    setReturns("GetWirelessAccessPoints", returns);

    params.clear(); returns.clear();
    setDescription("DisconnectInterface", "Disconnect the given network interface. The interface will remain disconnected until the user connect it again.");
    params.insert("interface", JsonTypes::basicTypeToString(QVariant::String));
    setParams("DisconnectInterface", params);
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorRef());
    setReturns("DisconnectInterface", returns);

    params.clear(); returns.clear();
    setDescription("GetNetworkDevices", "Get the list of current network devices.");
    setParams("GetNetworkDevices", params);
    returns.insert("wiredNetworkDevices",  QVariantList() << JsonTypes::wiredNetworkDeviceRef());
    returns.insert("wirelessNetworkDevices",  QVariantList() << JsonTypes::wirelessNetworkDeviceRef());
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorRef());
    setReturns("GetNetworkDevices", returns);

    params.clear(); returns.clear();
    setDescription("ScanWifiNetworks", "Start a wifi scan for searching new networks.");
    params.insert("interface", JsonTypes::basicTypeToString(QVariant::String));
    setParams("ScanWifiNetworks", params);
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorRef());
    setReturns("ScanWifiNetworks", returns);

    params.clear(); returns.clear();
    setDescription("ConnectWifiNetwork", "Connect to the wifi network with the given ssid and password.");
    params.insert("interface", JsonTypes::basicTypeToString(QVariant::String));
    params.insert("ssid", JsonTypes::basicTypeToString(QVariant::String));
    params.insert("o:password", JsonTypes::basicTypeToString(QVariant::String));
    setParams("ConnectWifiNetwork", params);
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorRef());
    setReturns("ConnectWifiNetwork", returns);

    // Notifications
    params.clear(); returns.clear();
    setDescription("NetworkStatusChanged", "Emitted whenever a status of a NetworkManager changes.");
    params.insert("status", status);
    setParams("NetworkStatusChanged", params);

    params.clear(); returns.clear();
    setDescription("WirelessNetworkDeviceAdded", "Emitted whenever a new WirelessNetworkDevice was added.");
    params.insert("wirelessNetworkDevice", JsonTypes::wirelessNetworkDeviceRef());
    setParams("WirelessNetworkDeviceAdded", params);

    params.clear(); returns.clear();
    setDescription("WirelessNetworkDeviceRemoved", "Emitted whenever a WirelessNetworkDevice was removed.");
    params.insert("interface", JsonTypes::basicTypeToString(QVariant::String));
    setParams("WirelessNetworkDeviceRemoved", params);

    params.clear(); returns.clear();
    setDescription("WirelessNetworkDeviceChanged", "Emitted whenever the given WirelessNetworkDevice has changed.");
    params.insert("wirelessNetworkDevice", JsonTypes::wirelessNetworkDeviceRef());
    setParams("WirelessNetworkDeviceChanged", params);

    params.clear(); returns.clear();
    setDescription("WiredNetworkDeviceAdded", "Emitted whenever a new WiredNetworkDevice was added.");
    params.insert("wiredNetworkDevice", JsonTypes::wiredNetworkDeviceRef());
    setParams("WiredNetworkDeviceAdded", params);

    params.clear(); returns.clear();
    setDescription("WiredNetworkDeviceRemoved", "Emitted whenever a WiredNetworkDevice was removed.");
    params.insert("interface", JsonTypes::basicTypeToString(QVariant::String));
    setParams("WiredNetworkDeviceRemoved", params);

    params.clear(); returns.clear();
    setDescription("WiredNetworkDeviceChanged", "Emitted whenever the given WiredNetworkDevice has changed.");
    params.insert("wiredNetworkDevice", JsonTypes::wiredNetworkDeviceRef());
    setParams("WiredNetworkDeviceChanged", params);

    connect(NymeaCore::instance()->networkManager(), &NetworkManager::stateChanged, this, &NetworkManagerHandler::onNetworkManagerStatusChanged);
    connect(NymeaCore::instance()->networkManager(), &NetworkManager::networkingEnabledChanged, this, &NetworkManagerHandler::onNetworkManagerStatusChanged);
    connect(NymeaCore::instance()->networkManager(), &NetworkManager::wirelessEnabledChanged, this, &NetworkManagerHandler::onNetworkManagerStatusChanged);

    connect(NymeaCore::instance()->networkManager(), &NetworkManager::wirelessDeviceAdded, this, &NetworkManagerHandler::onWirelessNetworkDeviceAdded);
    connect(NymeaCore::instance()->networkManager(), &NetworkManager::wirelessDeviceRemoved, this, &NetworkManagerHandler::onWirelessNetworkDeviceRemoved);
    connect(NymeaCore::instance()->networkManager(), &NetworkManager::wirelessDeviceChanged, this, &NetworkManagerHandler::onWirelessNetworkDeviceChanged);

    connect(NymeaCore::instance()->networkManager(), &NetworkManager::wiredDeviceAdded, this, &NetworkManagerHandler::onWiredNetworkDeviceAdded);
    connect(NymeaCore::instance()->networkManager(), &NetworkManager::wiredDeviceRemoved, this, &NetworkManagerHandler::onWiredNetworkDeviceRemoved);
    connect(NymeaCore::instance()->networkManager(), &NetworkManager::wiredDeviceChanged, this, &NetworkManagerHandler::onWiredNetworkDeviceChanged);
}

/*! Returns the name of the \l{NetworkManagerHandler}. In this case \b NetworkManager. */
QString NetworkManagerHandler::name() const
{
    return "NetworkManager";
}

JsonReply *NetworkManagerHandler::GetNetworkStatus(const QVariantMap &params)
{
    Q_UNUSED(params);

    // Check available
    if (!NymeaCore::instance()->networkManager()->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    // Pack network manager status
    QVariantMap returns;
    returns.insert("status", packNetworkManagerStatus());
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorToString(NetworkManager::NetworkManagerErrorNoError));
    return createReply(returns);
}

JsonReply *NetworkManagerHandler::EnableNetworking(const QVariantMap &params)
{
    if (!NymeaCore::instance()->networkManager()->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    bool enable = params.value("enable").toBool();

    if (!NymeaCore::instance()->networkManager()->enableNetworking(enable))
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorUnknownError));

    return createReply(statusToReply(NetworkManager::NetworkManagerErrorNoError));
}

JsonReply *NetworkManagerHandler::EnableWirelessNetworking(const QVariantMap &params)
{
    if (!NymeaCore::instance()->networkManager()->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    if (!NymeaCore::instance()->networkManager()->wifiAvailable())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNotAvailable));

    if (!NymeaCore::instance()->networkManager()->enableWireless(params.value("enable").toBool()))
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorUnknownError));

    return createReply(statusToReply(NetworkManager::NetworkManagerErrorNoError));
}

JsonReply *NetworkManagerHandler::GetWirelessAccessPoints(const QVariantMap &params)
{
    if (!NymeaCore::instance()->networkManager()->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    if (!NymeaCore::instance()->networkManager()->wifiAvailable())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNotAvailable));

    if (!NymeaCore::instance()->networkManager()->networkingEnabled())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkingDisabled));

    if (!NymeaCore::instance()->networkManager()->wirelessEnabled())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNetworkingDisabled));

    QString interface = params.value("interface").toString();

    if (!NymeaCore::instance()->networkManager()->getNetworkDevice(interface))
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkInterfaceNotFound));

    foreach (WirelessNetworkDevice *networkDevice, NymeaCore::instance()->networkManager()->wirelessNetworkDevices()) {
        if (networkDevice->interface() == interface) {
            QVariantList wirelessAccessPoints;
            foreach (WirelessAccessPoint *wirelessAccessPoint, networkDevice->accessPoints())
                wirelessAccessPoints.append(JsonTypes::packWirelessAccessPoint(wirelessAccessPoint));

            QVariantMap returns;
            returns.insert("wirelessAccessPoints", wirelessAccessPoints);
            returns.insert("networkManagerError", JsonTypes::networkManagerErrorToString(NetworkManager::NetworkManagerErrorNoError));
            return createReply(returns);

        }
    }

    return createReply(statusToReply(NetworkManager::NetworkManagerErrorInvalidNetworkDeviceType));
}

JsonReply *NetworkManagerHandler::GetNetworkDevices(const QVariantMap &params)
{
    Q_UNUSED(params);

    if (!NymeaCore::instance()->networkManager()->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    QVariantList wirelessNetworkDevices;
    foreach (WirelessNetworkDevice *networkDevice, NymeaCore::instance()->networkManager()->wirelessNetworkDevices())
        wirelessNetworkDevices.append(JsonTypes::packWirelessNetworkDevice(networkDevice));

    QVariantList wiredNetworkDevices;
    foreach (WiredNetworkDevice *networkDevice, NymeaCore::instance()->networkManager()->wiredNetworkDevices())
        wiredNetworkDevices.append(JsonTypes::packWiredNetworkDevice(networkDevice));

    QVariantMap returns;
    returns.insert("wirelessNetworkDevices", wirelessNetworkDevices);
    returns.insert("wiredNetworkDevices", wiredNetworkDevices);
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorToString(NetworkManager::NetworkManagerErrorNoError));
    return createReply(returns);
}

JsonReply *NetworkManagerHandler::ScanWifiNetworks(const QVariantMap &params)
{
    Q_UNUSED(params);

    if (!NymeaCore::instance()->networkManager()->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    if (!NymeaCore::instance()->networkManager()->wifiAvailable())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNotAvailable));

    if (!NymeaCore::instance()->networkManager()->networkingEnabled())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkingDisabled));

    if (!NymeaCore::instance()->networkManager()->wirelessEnabled())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNetworkingDisabled));


    QString interface = params.value("interface").toString();

    if (!NymeaCore::instance()->networkManager()->getNetworkDevice(interface))
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkInterfaceNotFound));

    foreach (WirelessNetworkDevice *networkDevice, NymeaCore::instance()->networkManager()->wirelessNetworkDevices()) {
        if (networkDevice->interface() == interface) {
            networkDevice->scanWirelessNetworks();
            return createReply(statusToReply(NetworkManager::NetworkManagerErrorNoError));
        }
    }

    return createReply(statusToReply(NetworkManager::NetworkManagerErrorInvalidNetworkDeviceType));
}

JsonReply *NetworkManagerHandler::ConnectWifiNetwork(const QVariantMap &params)
{
    if (!NymeaCore::instance()->networkManager()->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    if (!NymeaCore::instance()->networkManager()->wifiAvailable())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNotAvailable));

    if (!NymeaCore::instance()->networkManager()->networkingEnabled())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkingDisabled));

    if (!NymeaCore::instance()->networkManager()->wirelessEnabled())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNetworkingDisabled));


    QString ssid = params.value("ssid").toString();
    QString password = params.value("password").toString();
    QString interface = params.value("interface").toString();

    return createReply(statusToReply(NymeaCore::instance()->networkManager()->connectWifi(interface, ssid, password)));
}

JsonReply *NetworkManagerHandler::DisconnectInterface(const QVariantMap &params)
{
    if (!NymeaCore::instance()->networkManager()->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    QString interface = params.value("interface").toString();
    NetworkDevice *networkDevice = NymeaCore::instance()->networkManager()->getNetworkDevice(interface);
    if (!networkDevice)
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkInterfaceNotFound));

    networkDevice->disconnectDevice();
    return createReply(statusToReply(NetworkManager::NetworkManagerErrorNoError));
}

QVariantMap NetworkManagerHandler::packNetworkManagerStatus()
{
    QVariantMap status;
    status.insert("networkingEnabled", NymeaCore::instance()->networkManager()->networkingEnabled());
    status.insert("wirelessNetworkingEnabled", NymeaCore::instance()->networkManager()->wirelessEnabled());
    status.insert("state", NymeaCore::instance()->networkManager()->stateString());
    return status;
}

void NetworkManagerHandler::onNetworkManagerStatusChanged()
{
    emit NetworkStatusChanged(packNetworkManagerStatus());
}

void NetworkManagerHandler::onWirelessNetworkDeviceAdded(WirelessNetworkDevice *networkDevice)
{
    QVariantMap notification;
    notification.insert("wirelessNetworkDevice", JsonTypes::packWirelessNetworkDevice(networkDevice));
    emit WirelessNetworkDeviceAdded(notification);
}

void NetworkManagerHandler::onWirelessNetworkDeviceRemoved(const QString &interface)
{
    QVariantMap notification;
    notification.insert("interface", interface);
    emit WirelessNetworkDeviceRemoved(notification);
}

void NetworkManagerHandler::onWirelessNetworkDeviceChanged(WirelessNetworkDevice *networkDevice)
{
    QVariantMap notification;
    notification.insert("wirelessNetworkDevice", JsonTypes::packWirelessNetworkDevice(networkDevice));
    emit WirelessNetworkDeviceChanged(notification);
}

void NetworkManagerHandler::onWiredNetworkDeviceAdded(WiredNetworkDevice *networkDevice)
{
    QVariantMap notification;
    notification.insert("wiredNetworkDevice", JsonTypes::packWiredNetworkDevice(networkDevice));
    emit WiredNetworkDeviceAdded(notification);
}

void NetworkManagerHandler::onWiredNetworkDeviceRemoved(const QString &interface)
{
    QVariantMap notification;
    notification.insert("interface", interface);
    emit WiredNetworkDeviceRemoved(notification);
}

void NetworkManagerHandler::onWiredNetworkDeviceChanged(WiredNetworkDevice *networkDevice)
{
    QVariantMap notification;
    notification.insert("wiredNetworkDevice", JsonTypes::packWiredNetworkDevice(networkDevice));
    emit WiredNetworkDeviceChanged(notification);
}

}
