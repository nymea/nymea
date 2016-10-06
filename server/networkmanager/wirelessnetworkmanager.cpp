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

#include "wirelessnetworkmanager.h"

#include "dbus-interfaces.h"
#include "loggingcategories.h"

#include <QMetaEnum>
#include <QUuid>

namespace guhserver {

WirelessNetworkManager::WirelessNetworkManager(const QDBusObjectPath &objectPath, QObject *parent) :
    NetworkDevice(objectPath, parent)
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.isConnected()) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkManager: System DBus not connected";
        return;
    }

    m_wirelessInterface = new QDBusInterface(serviceString, this->objectPath().path(), wirelessInterfaceString, systemBus, this);
    if(!m_wirelessInterface->isValid()) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkManager: Invalid wireless dbus interface";
        return;
    }

    QDBusConnection::systemBus().connect(serviceString, this->objectPath().path(), wirelessInterfaceString, "AccessPointAdded", this, SLOT(accessPointAdded(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(serviceString, this->objectPath().path(), wirelessInterfaceString, "AccessPointRemoved", this, SLOT(accessPointRemoved(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(serviceString, this->objectPath().path(), wirelessInterfaceString, "PropertiesChanged", this, SLOT(propertiesChanged(QVariantMap)));

    m_macAddress = m_wirelessInterface->property("HwAddress").toString();
    m_bitrate = m_wirelessInterface->property("Bitrate").toInt() / 1000;

    qCDebug(dcNetworkManager()) << this;

    readAccessPoints();
}

QString WirelessNetworkManager::macAddress() const
{
    return m_macAddress;
}

int WirelessNetworkManager::bitrate() const
{
    return m_bitrate;
}

void WirelessNetworkManager::scanWirelessNetworks()
{
    QDBusMessage query = m_wirelessInterface->call("RequestScan", QVariantMap());
    if(query.type() != QDBusMessage::ReplyMessage) {
        qCWarning(dcNetworkManager()) << "Scan error:" << query.errorName() << query.errorMessage();
        return;
    }
}

QList<WirelessAccessPoint *> WirelessNetworkManager::accessPoints()
{
    return m_accessPointsTable.values();
}

WirelessAccessPoint *WirelessNetworkManager::getAccessPoint(const QString &ssid)
{
    foreach (WirelessAccessPoint *accessPoint, m_accessPointsTable.values()) {
        if (accessPoint->ssid() == ssid)
            return accessPoint;
    }
    return Q_NULLPTR;
}

WirelessAccessPoint *WirelessNetworkManager::getAccessPoint(const QDBusObjectPath &objectPath)
{
    return m_accessPointsTable.value(objectPath);
}

void WirelessNetworkManager::readAccessPoints()
{
    QDBusMessage query = m_wirelessInterface->call("GetAccessPoints");
    if(query.type() != QDBusMessage::ReplyMessage) {
        qCWarning(dcNetworkManager()) << query.errorName() << query.errorMessage();
        return;
    }

    if (query.arguments().isEmpty())
        return;

    const QDBusArgument &argument = query.arguments().at(0).value<QDBusArgument>();
    argument.beginArray();
    while(!argument.atEnd()) {
        QDBusObjectPath accessPointObjectPath = qdbus_cast<QDBusObjectPath>(argument);
        accessPointAdded(accessPointObjectPath);
    }
    argument.endArray();
}

void WirelessNetworkManager::accessPointAdded(const QDBusObjectPath &objectPath)
{
    QDBusInterface accessPointInterface(serviceString, objectPath.path(), accessPointInterfaceString, QDBusConnection::systemBus());
    if(!accessPointInterface.isValid()) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkManager: Invalid access point dbus interface";
        return;
    }

    if (m_accessPointsTable.keys().contains(objectPath)) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkManager: Access point already added" << objectPath.path();
        return;
    }

    WirelessAccessPoint *accessPoint = new WirelessAccessPoint(objectPath, this);
    //qCDebug(dcNetworkManager()) << "WirelessNetworkManager: [+]" << accessPoint;

    // Add access point
    m_accessPointsTable.insert(objectPath, accessPoint);
}

void WirelessNetworkManager::accessPointRemoved(const QDBusObjectPath &objectPath)
{
    if (!m_accessPointsTable.keys().contains(objectPath))
        return;

    // Remove access point
    WirelessAccessPoint *accessPoint = m_accessPointsTable.take(objectPath);
    //qCDebug(dcNetworkManager()) << "WirelessNetworkManager: [-]" << accessPoint;
    accessPoint->deleteLater();
}

void WirelessNetworkManager::propertiesChanged(const QVariantMap &properties)
{
    qCDebug(dcNetworkManager()) << "WirelessNetworkManager: Property changed" << properties;
}

QDebug operator<<(QDebug debug, WirelessNetworkManager *manager)
{
    debug.nospace() << "WirelessDevice(" << manager->interface() << ", ";
    debug.nospace() << manager->macAddress() <<  ", ";
    debug.nospace() << manager->bitrate() <<  " [Mb/s], ";
    debug.nospace() << manager->deviceStateString() <<  ") ";
    return debug;
}

}
