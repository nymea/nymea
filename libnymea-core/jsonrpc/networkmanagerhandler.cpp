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

#include "networkmanagerhandler.h"
#include "loggingcategories.h"

namespace nymeaserver {

/*! Constructs a new \l{NetworkManagerHandler} with the given \a parent. */
NetworkManagerHandler::NetworkManagerHandler(NetworkManager *networkManager, QObject *parent)
    : JsonHandler(parent)
    , m_networkManager(networkManager)
{
    // Enums
    registerEnum<NetworkManager::NetworkManagerError>();
    registerEnum<NetworkManager::NetworkManagerState>();
    registerEnum<NetworkDevice::NetworkDeviceState>();
    registerFlag<WirelessNetworkDevice::WirelessCapability, WirelessNetworkDevice::WirelessCapabilities>();
    registerEnum<WirelessNetworkDevice::WirelessMode>();
    registerEnum<WiredNetworkConnectionType>();

    // Objects
    QVariantMap wirelessAccessPoint;
    wirelessAccessPoint.insert("r:ssid", enumValueName(String));
    wirelessAccessPoint.insert("r:macAddress", enumValueName(String));
    wirelessAccessPoint.insert("r:frequency", enumValueName(Double));
    wirelessAccessPoint.insert("r:signalStrength", enumValueName(Int));
    wirelessAccessPoint.insert("r:protected", enumValueName(Bool));
    registerObject("WirelessAccessPoint", wirelessAccessPoint);

    QVariantMap wiredNetworkDevice;
    wiredNetworkDevice.insert("r:interface", enumValueName(String));
    wiredNetworkDevice.insert("r:macAddress", enumValueName(String));
    wiredNetworkDevice.insert("r:ipv4Addresses", enumValueName(StringList));
    wiredNetworkDevice.insert("r:ipv6Addresses", enumValueName(StringList));
    wiredNetworkDevice.insert("r:state", enumRef<NetworkDevice::NetworkDeviceState>());
    wiredNetworkDevice.insert("r:bitRate", enumValueName(String));
    wiredNetworkDevice.insert("r:pluggedIn", enumValueName(Bool));
    registerObject("WiredNetworkDevice", wiredNetworkDevice);

    QVariantMap wirelessNetworkDevice;
    wirelessNetworkDevice.insert("r:interface", enumValueName(String));
    wirelessNetworkDevice.insert("r:macAddress", enumValueName(String));
    wirelessNetworkDevice.insert("r:ipv4Addresses", enumValueName(StringList));
    wirelessNetworkDevice.insert("r:ipv6Addresses", enumValueName(StringList));
    wirelessNetworkDevice.insert("r:state", enumRef<NetworkDevice::NetworkDeviceState>());
    wirelessNetworkDevice.insert("r:capabilities", flagRef<WirelessNetworkDevice::WirelessCapabilities>());
    wirelessNetworkDevice.insert("r:bitRate", enumValueName(String));
    wirelessNetworkDevice.insert("r:mode", enumRef<WirelessNetworkDevice::WirelessMode>());
    wirelessNetworkDevice.insert("r:o:currentAccessPoint", objectRef<WirelessAccessPoint>());
    registerObject("WirelessNetworkDevice", wirelessNetworkDevice);

    // Methods
    QString description;
    QVariantMap params;
    QVariantMap returns;
    description = "Get the current network manager status.";
    QVariantMap status;
    status.insert("networkingEnabled", enumValueName(Bool));
    status.insert("wirelessNetworkingEnabled", enumValueName(Bool));
    status.insert("state", enumRef<NetworkManager::NetworkManagerState>());
    returns.insert("o:status", status);
    returns.insert("networkManagerError", enumRef<NetworkManager::NetworkManagerError>());
    registerMethod("GetNetworkStatus", description, params, returns);

    params.clear();
    returns.clear();
    description = "Enable or disable networking in the NetworkManager.";
    params.insert("enable", enumValueName(Bool));
    returns.insert("networkManagerError", enumRef<NetworkManager::NetworkManagerError>());
    registerMethod("EnableNetworking", description, params, returns);

    params.clear();
    returns.clear();
    description = "Enable or disable wireless networking in the NetworkManager.";
    params.insert("enable", enumValueName(Bool));
    returns.insert("networkManagerError", enumRef<NetworkManager::NetworkManagerError>());
    registerMethod("EnableWirelessNetworking", description, params, returns);

    params.clear();
    returns.clear();
    description = "Get the current list of wireless network access points for the given interface. The interface has to be a WirelessNetworkDevice.";
    params.insert("interface", enumValueName(String));
    returns.insert("o:wirelessAccessPoints", QVariantList() << objectRef("WirelessAccessPoint"));
    returns.insert("networkManagerError", enumRef<NetworkManager::NetworkManagerError>());
    registerMethod("GetWirelessAccessPoints", description, params, returns);

    params.clear();
    returns.clear();
    description = "Disconnect the given network interface. The interface will remain disconnected until the user connect it again.";
    params.insert("interface", enumValueName(String));
    returns.insert("networkManagerError", enumRef<NetworkManager::NetworkManagerError>());
    registerMethod("DisconnectInterface", description, params, returns);

    params.clear();
    returns.clear();
    description = "Get the list of current network devices.";
    returns.insert("networkManagerError", enumRef<NetworkManager::NetworkManagerError>());
    returns.insert("o:wiredNetworkDevices", QVariantList() << objectRef("WiredNetworkDevice"));
    returns.insert("o:wirelessNetworkDevices", QVariantList() << objectRef("WirelessNetworkDevice"));
    registerMethod("GetNetworkDevices", description, params, returns);

    params.clear();
    returns.clear();
    description = "Start a wifi scan for searching new networks.";
    params.insert("interface", enumValueName(String));
    returns.insert("networkManagerError", enumRef<NetworkManager::NetworkManagerError>());
    registerMethod("ScanWifiNetworks", description, params, returns);

    params.clear();
    returns.clear();
    description = "Create a wired connection.";
    params.insert("interface", enumValueName(String));
    params.insert("type", enumRef<WiredNetworkConnectionType>());
    params.insert("o:ip", enumValueName(String));
    params.insert("o:prefix", enumValueName(Uint));
    params.insert("o:gateway", enumValueName(String));
    params.insert("o:dns", enumValueName(String));
    returns.insert("networkManagerError", enumRef<NetworkManager::NetworkManagerError>());
    registerMethod("CreateWiredConnection", description, params, returns);

    params.clear();
    returns.clear();
    description = "Connect to the wifi network with the given ssid and password.";
    params.insert("interface", enumValueName(String));
    params.insert("ssid", enumValueName(String));
    params.insert("o:password", enumValueName(String));
    returns.insert("networkManagerError", enumRef<NetworkManager::NetworkManagerError>());
    registerMethod("ConnectWifiNetwork", description, params, returns);

    params.clear();
    returns.clear();
    description = "Start a WiFi Access point on the given interface with the given SSID and "
                  "password. Use DisconnectInterface to stop it again.";
    params.insert("interface", enumValueName(String));
    params.insert("ssid", enumValueName(String));
    params.insert("password", enumValueName(String));
    returns.insert("networkManagerError", enumRef<NetworkManager::NetworkManagerError>());
    registerMethod("StartAccessPoint", description, params, returns);

    // Notifications
    params.clear();
    returns.clear();
    description = "Emitted whenever a status of a NetworkManager changes.";
    params.insert("status", status);
    registerNotification("NetworkStatusChanged", description, params);

    params.clear();
    returns.clear();
    description = "Emitted whenever a new WirelessNetworkDevice was added.";
    params.insert("wirelessNetworkDevice", objectRef("WirelessNetworkDevice"));
    registerNotification("WirelessNetworkDeviceAdded", description, params);

    params.clear();
    returns.clear();
    description = "Emitted whenever a WirelessNetworkDevice was removed.";
    params.insert("interface", enumValueName(String));
    registerNotification("WirelessNetworkDeviceRemoved", description, params);

    params.clear();
    returns.clear();
    description = "Emitted whenever the given WirelessNetworkDevice has changed.";
    params.insert("wirelessNetworkDevice", objectRef("WirelessNetworkDevice"));
    registerNotification("WirelessNetworkDeviceChanged", description, params);

    params.clear();
    returns.clear();
    description = "Emitted whenever a new WiredNetworkDevice was added.";
    params.insert("wiredNetworkDevice", objectRef("WiredNetworkDevice"));
    registerNotification("WiredNetworkDeviceAdded", description, params);

    params.clear();
    returns.clear();
    description = "Emitted whenever a WiredNetworkDevice was removed.";
    params.insert("interface", enumValueName(String));
    registerNotification("WiredNetworkDeviceRemoved", description, params);

    params.clear();
    returns.clear();
    description = "Emitted whenever the given WiredNetworkDevice has changed.";
    params.insert("wiredNetworkDevice", objectRef("WiredNetworkDevice"));
    registerNotification("WiredNetworkDeviceChanged", description, params);

    connect(m_networkManager, &NetworkManager::stateChanged, this, &NetworkManagerHandler::onNetworkManagerStatusChanged);
    connect(m_networkManager, &NetworkManager::networkingEnabledChanged, this, &NetworkManagerHandler::onNetworkManagerStatusChanged);
    connect(m_networkManager, &NetworkManager::wirelessEnabledChanged, this, &NetworkManagerHandler::onNetworkManagerStatusChanged);

    connect(m_networkManager, &NetworkManager::wirelessDeviceAdded, this, &NetworkManagerHandler::onWirelessNetworkDeviceAdded);
    connect(m_networkManager, &NetworkManager::wirelessDeviceRemoved, this, &NetworkManagerHandler::onWirelessNetworkDeviceRemoved);
    connect(m_networkManager, &NetworkManager::wirelessDeviceChanged, this, &NetworkManagerHandler::onWirelessNetworkDeviceChanged);

    connect(m_networkManager, &NetworkManager::wiredDeviceAdded, this, &NetworkManagerHandler::onWiredNetworkDeviceAdded);
    connect(m_networkManager, &NetworkManager::wiredDeviceRemoved, this, &NetworkManagerHandler::onWiredNetworkDeviceRemoved);
    connect(m_networkManager, &NetworkManager::wiredDeviceChanged, this, &NetworkManagerHandler::onWiredNetworkDeviceChanged);
}

/*! Returns the name of the \l{NetworkManagerHandler}. In this case \b NetworkManager. */
QString NetworkManagerHandler::name() const
{
    return "NetworkManager";
}

JsonReply *NetworkManagerHandler::GetNetworkStatus(const QVariantMap &params)
{
    Q_UNUSED(params)

    // Check available
    if (!m_networkManager->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    // Pack network manager status
    QVariantMap returns = statusToReply(NetworkManager::NetworkManagerErrorNoError);
    returns.insert("status", packNetworkManagerStatus());
    return createReply(returns);
}

JsonReply *NetworkManagerHandler::EnableNetworking(const QVariantMap &params)
{
    if (!m_networkManager->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    bool enable = params.value("enable").toBool();

    if (!m_networkManager->enableNetworking(enable))
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorUnknownError));

    return createReply(statusToReply(NetworkManager::NetworkManagerErrorNoError));
}

JsonReply *NetworkManagerHandler::EnableWirelessNetworking(const QVariantMap &params)
{
    if (!m_networkManager->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    if (!m_networkManager->wirelessAvailable())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNotAvailable));

    if (!m_networkManager->enableWireless(params.value("enable").toBool()))
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorUnknownError));

    return createReply(statusToReply(NetworkManager::NetworkManagerErrorNoError));
}

JsonReply *NetworkManagerHandler::GetWirelessAccessPoints(const QVariantMap &params)
{
    if (!m_networkManager->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    if (!m_networkManager->wirelessAvailable())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNotAvailable));

    if (!m_networkManager->networkingEnabled())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkingDisabled));

    if (!m_networkManager->wirelessEnabled())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNetworkingDisabled));

    QString interface = params.value("interface").toString();

    if (!m_networkManager->getNetworkDevice(interface))
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkInterfaceNotFound));

    foreach (WirelessNetworkDevice *networkDevice, m_networkManager->wirelessNetworkDevices()) {
        if (networkDevice->interface() == interface) {
            QVariantList wirelessAccessPoints;
            foreach (WirelessAccessPoint *wirelessAccessPoint, networkDevice->accessPoints())
                wirelessAccessPoints.append(packWirelessAccessPoint(wirelessAccessPoint));

            QVariantMap returns = statusToReply(NetworkManager::NetworkManagerErrorNoError);
            returns.insert("wirelessAccessPoints", wirelessAccessPoints);
            return createReply(returns);
        }
    }

    return createReply(statusToReply(NetworkManager::NetworkManagerErrorInvalidNetworkDeviceType));
}

JsonReply *NetworkManagerHandler::GetNetworkDevices(const QVariantMap &params)
{
    Q_UNUSED(params)

    if (!m_networkManager->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    QVariantList wirelessNetworkDevices;
    foreach (WirelessNetworkDevice *networkDevice, m_networkManager->wirelessNetworkDevices())
        wirelessNetworkDevices.append(packWirelessNetworkDevice(networkDevice));

    QVariantList wiredNetworkDevices;
    foreach (WiredNetworkDevice *networkDevice, m_networkManager->wiredNetworkDevices())
        wiredNetworkDevices.append(packWiredNetworkDevice(networkDevice));

    QVariantMap returns = statusToReply(NetworkManager::NetworkManagerErrorNoError);
    returns.insert("wirelessNetworkDevices", wirelessNetworkDevices);
    returns.insert("wiredNetworkDevices", wiredNetworkDevices);
    return createReply(returns);
}

JsonReply *NetworkManagerHandler::CreateWiredConnection(const QVariantMap &params)
{
    if (!m_networkManager->available()) {
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));
    }
    QString interface = params.value("interface").toString();
    QMetaEnum typeEnum = QMetaEnum::fromType<WiredNetworkConnectionType>();
    WiredNetworkConnectionType type = static_cast<WiredNetworkConnectionType>(typeEnum.keyToValue(params.value("type").toByteArray()));

    switch (type) {
    case WiredNetworkConnectionTypeManual: {
        QHostAddress ip = QHostAddress(params.value("ip").toString());
        quint8 prefix = params.value("prefix").toInt();
        QHostAddress gateway = QHostAddress(params.value("gateway").toString());
        QHostAddress dns = QHostAddress(params.value("dns").toString());
        NetworkManager::NetworkManagerError status = m_networkManager->createWiredManualConnection(interface, ip, prefix, gateway, dns);
        return createReply(statusToReply(status));
    }
    case WiredNetworkConnectionTypeDHCP: {
        NetworkManager::NetworkManagerError status = m_networkManager->createWiredAutoConnection(interface);
        return createReply(statusToReply(status));
    }
    case WiredNetworkConnectionTypeShared: {
        QHostAddress ip = QHostAddress(params.value("ip").toString());
        quint8 prefix = params.value("prefix").toInt();
        NetworkManager::NetworkManagerError status = m_networkManager->createSharedConnection(interface, ip, prefix);
        return createReply(statusToReply(status));
    }
    }
    return createReply(statusToReply(NetworkManager::NetworkManagerErrorInvalidConfiguration));
}

JsonReply *NetworkManagerHandler::ScanWifiNetworks(const QVariantMap &params)
{
    Q_UNUSED(params)

    if (!m_networkManager->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    if (!m_networkManager->wirelessAvailable())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNotAvailable));

    if (!m_networkManager->networkingEnabled())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkingDisabled));

    if (!m_networkManager->wirelessEnabled())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNetworkingDisabled));

    QString interface = params.value("interface").toString();

    if (!m_networkManager->getNetworkDevice(interface))
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkInterfaceNotFound));

    foreach (WirelessNetworkDevice *networkDevice, m_networkManager->wirelessNetworkDevices()) {
        if (networkDevice->interface() == interface) {
            networkDevice->scanWirelessNetworks();
            return createReply(statusToReply(NetworkManager::NetworkManagerErrorNoError));
        }
    }

    return createReply(statusToReply(NetworkManager::NetworkManagerErrorInvalidNetworkDeviceType));
}

JsonReply *NetworkManagerHandler::ConnectWifiNetwork(const QVariantMap &params)
{
    if (!m_networkManager->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    if (!m_networkManager->wirelessAvailable())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNotAvailable));

    if (!m_networkManager->networkingEnabled())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkingDisabled));

    if (!m_networkManager->wirelessEnabled())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorWirelessNetworkingDisabled));

    QString ssid = params.value("ssid").toString();
    QString password = params.value("password").toString();
    QString interface = params.value("interface").toString();

    return createReply(statusToReply(m_networkManager->connectWifi(interface, ssid, password)));
}

JsonReply *NetworkManagerHandler::DisconnectInterface(const QVariantMap &params)
{
    if (!m_networkManager->available())
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));

    QString interface = params.value("interface").toString();
    NetworkDevice *networkDevice = m_networkManager->getNetworkDevice(interface);
    if (!networkDevice)
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkInterfaceNotFound));

    networkDevice->disconnectDevice();
    return createReply(statusToReply(NetworkManager::NetworkManagerErrorNoError));
}

JsonReply *NetworkManagerHandler::StartAccessPoint(const QVariantMap &params)
{
    if (!m_networkManager->available()) {
        return createReply(statusToReply(NetworkManager::NetworkManagerErrorNetworkManagerNotAvailable));
    }

    QString interface = params.value("interface").toString();
    QString ssid = params.value("ssid").toString();
    QString password = params.value("password").toString();
    NetworkManager::NetworkManagerError status = m_networkManager->startAccessPoint(interface, ssid, password);
    return createReply(statusToReply(status));
}

QVariantMap NetworkManagerHandler::packNetworkManagerStatus()
{
    QVariantMap status;
    status.insert("networkingEnabled", m_networkManager->networkingEnabled());
    status.insert("wirelessNetworkingEnabled", m_networkManager->wirelessEnabled());
    status.insert("state", m_networkManager->stateString());
    return status;
}

void NetworkManagerHandler::onNetworkManagerStatusChanged()
{
    QVariantMap notification;
    notification.insert("status", packNetworkManagerStatus());
    emit NetworkStatusChanged(notification);
}

void NetworkManagerHandler::onWirelessNetworkDeviceAdded(WirelessNetworkDevice *networkDevice)
{
    QVariantMap notification;
    notification.insert("wirelessNetworkDevice", packWirelessNetworkDevice(networkDevice));
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
    notification.insert("wirelessNetworkDevice", packWirelessNetworkDevice(networkDevice));
    emit WirelessNetworkDeviceChanged(notification);
}

void NetworkManagerHandler::onWiredNetworkDeviceAdded(WiredNetworkDevice *networkDevice)
{
    QVariantMap notification;
    notification.insert("wiredNetworkDevice", packWiredNetworkDevice(networkDevice));
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
    notification.insert("wiredNetworkDevice", packWiredNetworkDevice(networkDevice));
    emit WiredNetworkDeviceChanged(notification);
}

QVariantMap NetworkManagerHandler::packWirelessAccessPoint(WirelessAccessPoint *wirelessAccessPoint)
{
    QVariantMap wirelessAccessPointVariant;
    wirelessAccessPointVariant.insert("ssid", wirelessAccessPoint->ssid());
    wirelessAccessPointVariant.insert("macAddress", wirelessAccessPoint->macAddress());
    wirelessAccessPointVariant.insert("frequency", wirelessAccessPoint->frequency());
    wirelessAccessPointVariant.insert("signalStrength", wirelessAccessPoint->signalStrength());
    wirelessAccessPointVariant.insert("protected", wirelessAccessPoint->isProtected());
    return wirelessAccessPointVariant;
}

QVariantMap NetworkManagerHandler::packWiredNetworkDevice(WiredNetworkDevice *networkDevice)
{
    QVariantMap networkDeviceVariant;
    networkDeviceVariant.insert("interface", networkDevice->interface());
    networkDeviceVariant.insert("macAddress", networkDevice->macAddress());
    networkDeviceVariant.insert("ipv4Addresses", networkDevice->ipv4Addresses());
    networkDeviceVariant.insert("ipv6Addresses", networkDevice->ipv6Addresses());
    networkDeviceVariant.insert("state", networkDevice->deviceStateString());
    networkDeviceVariant.insert("bitRate", QString("%1 [Mb/s]").arg(QString::number(networkDevice->bitRate())));
    networkDeviceVariant.insert("pluggedIn", networkDevice->pluggedIn());
    return networkDeviceVariant;
}

QVariantMap NetworkManagerHandler::packWirelessNetworkDevice(WirelessNetworkDevice *networkDevice)
{
    QVariantMap networkDeviceVariant;
    networkDeviceVariant.insert("interface", networkDevice->interface());
    networkDeviceVariant.insert("macAddress", networkDevice->macAddress());
    networkDeviceVariant.insert("ipv4Addresses", networkDevice->ipv4Addresses());
    networkDeviceVariant.insert("ipv6Addresses", networkDevice->ipv6Addresses());
    networkDeviceVariant.insert("state", networkDevice->deviceStateString());
    networkDeviceVariant.insert("capabilities", flagValueNames(networkDevice->wirelessCapabilities()));
    networkDeviceVariant.insert("mode", enumValueName(networkDevice->wirelessMode()));
    networkDeviceVariant.insert("bitRate", QString("%1 [Mb/s]").arg(QString::number(networkDevice->bitRate())));
    if (networkDevice->activeAccessPoint())
        networkDeviceVariant.insert("currentAccessPoint", packWirelessAccessPoint(networkDevice->activeAccessPoint()));

    return networkDeviceVariant;
}

QVariantMap NetworkManagerHandler::statusToReply(NetworkManager::NetworkManagerError status) const
{
    QVariantMap returns;
    returns.insert("networkManagerError", enumValueName<NetworkManager::NetworkManagerError>(status));
    return returns;
}

} // namespace nymeaserver
