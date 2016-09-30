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

#include "networkmanager.h"
#include "loggingcategories.h"

#include <QMetaEnum>

namespace guhserver {

NetworkManager::NetworkManager(QObject *parent) :
    QObject(parent),
    m_wirelessNetworkManager(0)
{
    // Create interface
    m_networkManagerInterface = new QDBusInterface(serviceString, pathString, serviceString, QDBusConnection::systemBus(), this);
    if(!m_networkManagerInterface->isValid()) {
        qCWarning(dcNetworkManager()) << "Invalid DBus network manager interface";
        return;
    }

    // Read properties
    m_version = m_networkManagerInterface->property("Version").toString();
    m_state = NetworkManagerState(m_networkManagerInterface->property("State").toUInt());
    m_connectivityState = NetworkManagerConnectivityState(m_networkManagerInterface->property("Connectivity").toUInt());
    m_networkingEnabled = m_networkManagerInterface->property("NetworkingEnabled").toBool();
    m_wirelessEnabled = m_networkManagerInterface->property("WirelessEnabled").toBool();

    qCDebug(dcNetworkManager()) << "Networkmanager version" << m_version;

    // Get network devices
    QDBusMessage query = m_networkManagerInterface->call("GetDevices");
    if(query.type() != QDBusMessage::ReplyMessage) {
        qCWarning(dcNetworkManager()) << query.errorName() << query.errorMessage();
        return;
    }

    const QDBusArgument &argument = query.arguments().at(0).value<QDBusArgument>();
    argument.beginArray();
    while(!argument.atEnd()) {
        QDBusObjectPath deviceObjectPath = qdbus_cast<QDBusObjectPath>(argument);
        onDeviceAdded(deviceObjectPath);
    }
    argument.endArray();

    // Connect signals
    QDBusConnection::systemBus().connect(serviceString, pathString, serviceString, "StateChanged", this, SLOT(onStateChanged(uint)));
    QDBusConnection::systemBus().connect(serviceString, pathString, serviceString, "DeviceAdded", this, SLOT(onDeviceAdded(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(serviceString, pathString, serviceString, "DeviceRemoved", this, SLOT(onDeviceRemoved(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(serviceString, pathString, serviceString, "PropertiesChanged", this, SLOT(onPropertiesChanged(QVariantMap)));

    m_networkSettings = new NetworkSettings(this);

}

bool NetworkManager::available()
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.isConnected()) {
        qCWarning(dcNetworkManager()) << "System DBus not connected";
        return false;
    }

    QDBusInterface networkInterface(serviceString, pathString, serviceString, QDBusConnection::systemBus());
    if(!networkInterface.isValid()) {
        qCWarning(dcNetworkManager()) << "Invalid DBus network manager interface";
        return false;
    }

    return true;
}

QString NetworkManager::networkManagerStateToString(const NetworkManager::NetworkManagerState &state)
{
    QMetaObject metaObject = NetworkManager::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("NetworkManagerState").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    return QString(metaEnum.valueToKey(state)).remove("NetworkManagerState");
}

QString NetworkManager::networkManagerConnectivityStateToString(const NetworkManager::NetworkManagerConnectivityState &state)
{
    QMetaObject metaObject = NetworkManager::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("NetworkManagerConnectivityState").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    return QString(metaEnum.valueToKey(state)).remove("NetworkManagerConnectivityState");
}

QList<NetworkDevice *> NetworkManager::networkDevices() const
{
    return m_networkDevices.values();
}

QString NetworkManager::version() const
{
    return m_version;
}

NetworkManager::NetworkManagerState NetworkManager::state() const
{
    return m_state;
}

NetworkManager::NetworkManagerConnectivityState NetworkManager::connectivityState() const
{
    return m_connectivityState;
}

bool NetworkManager::networkingEnabled() const
{
    return m_networkingEnabled;
}

bool NetworkManager::enableNetworking()
{
    if (m_networkingEnabled)
        return true;

    QDBusMessage query = m_networkManagerInterface->call("Enable", true);
    if(query.type() != QDBusMessage::ReplyMessage) {
        qCWarning(dcNetworkManager()) << query.errorName() << query.errorMessage();
        return false;
    }
    return true;
}

bool NetworkManager::disableNetworking()
{
    if (!m_networkingEnabled)
        return true;

    QDBusMessage query = m_networkManagerInterface->call("Enable", false);
    if(query.type() != QDBusMessage::ReplyMessage) {
        qCWarning(dcNetworkManager()) << query.errorName() << query.errorMessage();
        return false;
    }
    return true;
}

void NetworkManager::setNetworkingEnabled(const bool &enabled)
{
    qCDebug(dcNetworkManager()) << "Networking" << (enabled ? "enabled" : "disabled");
    m_networkingEnabled = enabled;
    emit networkingEnabledChanged();
}

bool NetworkManager::wirelessEnabled() const
{
    return m_wirelessEnabled;
}

bool NetworkManager::enableWireless()
{
    if (m_wirelessEnabled)
        return true;

    return m_networkManagerInterface->setProperty("WirelessEnabled", true);
}

void NetworkManager::setVersion(const QString &version)
{
    m_version = version;
    emit versionChanged();
}

void NetworkManager::setWirelessEnabled(const bool &enabled)
{
    qCDebug(dcNetworkManager()) << "Wireless networking" << (enabled ? "enabled" : "disabled");
    m_wirelessEnabled = enabled;
    emit wirelessEnabledChanged();
}

void NetworkManager::setConnectivityState(const NetworkManager::NetworkManagerConnectivityState &connectivityState)
{
    qCDebug(dcNetworkManager()) << "Connectivity state changed:" << networkManagerConnectivityStateToString(connectivityState);
    m_connectivityState = connectivityState;
    emit connectivityStateChanged();
}

void NetworkManager::setState(const NetworkManager::NetworkManagerState &state)
{
    qCDebug(dcNetworkManager()) << "State changed:" << networkManagerStateToString(state);
    m_state = state;
    emit stateChanged();
}

void NetworkManager::onDeviceAdded(const QDBusObjectPath &deviceObjectPath)
{
    if (m_networkDevices.keys().contains(deviceObjectPath)) {
        qCWarning(dcNetworkManager()) << "Device" << deviceObjectPath.path() << "already added.";
        return;
    }

    NetworkDevice *networkDevice = new NetworkDevice(deviceObjectPath, this);
    qCDebug(dcNetworkManager()) << "[+]" << networkDevice;

    if (!m_wirelessNetworkManager && networkDevice->deviceType() == NetworkDevice::DeviceTypeWifi)
        m_wirelessNetworkManager = new WirelessNetworkManager(networkDevice->objectPath(), this);

    m_networkDevices.insert(deviceObjectPath, networkDevice);
}

void NetworkManager::onDeviceRemoved(const QDBusObjectPath &deviceObjectPath)
{
    if (!m_networkDevices.keys().contains(deviceObjectPath)) {
        qCWarning(dcNetworkManager()) << "Unknown network device removed:" << deviceObjectPath.path();
        return;
    }

    NetworkDevice *networkDevice = m_networkDevices.take(deviceObjectPath);
    qCDebug(dcNetworkManager()) << "[-]" << networkDevice;
    networkDevice->deleteLater();
}

void NetworkManager::onPropertiesChanged(const QVariantMap &properties)
{
    //qCDebug(dcNetworkManager()) << "Network manager properties changed" << properties;

    if (properties.contains("Version"))
        setVersion(properties.value("Version").toString());

    if (properties.contains("State"))
        setState(NetworkManagerState(properties.value("State").toUInt()));

    if (properties.contains("Connectivity"))
        setConnectivityState(NetworkManagerConnectivityState(properties.value("Connectivity").toUInt()));

    if (properties.contains("NetworkingEnabled"))
        setNetworkingEnabled(properties.value("NetworkingEnabled").toBool());

    if (properties.contains("WirelessEnabled"))
        setWirelessEnabled(properties.value("WirelessEnabled").toBool());

}

}
