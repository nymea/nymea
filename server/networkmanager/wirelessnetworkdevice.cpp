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

#include "wirelessnetworkdevice.h"

#include "dbus-interfaces.h"
#include "loggingcategories.h"

#include <QMetaEnum>
#include <QUuid>

namespace guhserver {

WirelessNetworkDevice::WirelessNetworkDevice(const QDBusObjectPath &objectPath, QObject *parent) :
    NetworkDevice(objectPath, parent),
    m_activeAccessPoint(Q_NULLPTR)
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.isConnected()) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkDevice: System DBus not connected";
        return;
    }

    m_wirelessInterface = new QDBusInterface(serviceString, this->objectPath().path(), wirelessInterfaceString, systemBus, this);
    if (!m_wirelessInterface->isValid()) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkDevice: Invalid wireless dbus interface";
        return;
    }

    QDBusConnection::systemBus().connect(serviceString, this->objectPath().path(), wirelessInterfaceString, "AccessPointAdded", this, SLOT(accessPointAdded(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(serviceString, this->objectPath().path(), wirelessInterfaceString, "AccessPointRemoved", this, SLOT(accessPointRemoved(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(serviceString, this->objectPath().path(), wirelessInterfaceString, "PropertiesChanged", this, SLOT(propertiesChanged(QVariantMap)));

    readAccessPoints();

    setMacAddress(m_wirelessInterface->property("HwAddress").toString());
    setBitrate(m_wirelessInterface->property("Bitrate").toInt());
    setActiveAccessPoint(qdbus_cast<QDBusObjectPath>(m_wirelessInterface->property("ActiveAccessPoint")));
}

QString WirelessNetworkDevice::macAddress() const
{
    return m_macAddress;
}

int WirelessNetworkDevice::bitRate() const
{
    return m_bitRate;
}

WirelessAccessPoint *WirelessNetworkDevice::activeAccessPoint()
{
    return m_activeAccessPoint;
}

void WirelessNetworkDevice::scanWirelessNetworks()
{
    qCDebug(dcNetworkManager()) << this << "Request scan";
    QDBusMessage query = m_wirelessInterface->call("RequestScan", QVariantMap());
    if (query.type() != QDBusMessage::ReplyMessage) {
        qCWarning(dcNetworkManager()) << "Scan error:" << query.errorName() << query.errorMessage();
        return;
    }
}

QList<WirelessAccessPoint *> WirelessNetworkDevice::accessPoints()
{
    return m_accessPointsTable.values();
}

WirelessAccessPoint *WirelessNetworkDevice::getAccessPoint(const QString &ssid)
{
    foreach (WirelessAccessPoint *accessPoint, m_accessPointsTable.values()) {
        if (accessPoint->ssid() == ssid)
            return accessPoint;
    }
    return Q_NULLPTR;
}

WirelessAccessPoint *WirelessNetworkDevice::getAccessPoint(const QDBusObjectPath &objectPath)
{
    return m_accessPointsTable.value(objectPath);
}

void WirelessNetworkDevice::readAccessPoints()
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
    while (!argument.atEnd()) {
        QDBusObjectPath accessPointObjectPath = qdbus_cast<QDBusObjectPath>(argument);
        accessPointAdded(accessPointObjectPath);
    }
    argument.endArray();
}

void WirelessNetworkDevice::setMacAddress(const QString &macAddress)
{
    m_macAddress = macAddress;
}

void WirelessNetworkDevice::setBitrate(const int &bitRate)
{
    if (m_bitRate != bitRate / 1000) {
        m_bitRate = bitRate / 1000;
        emit deviceChanged();
    }
}

void WirelessNetworkDevice::setActiveAccessPoint(const QDBusObjectPath &activeAccessPointObjectPath)
{
    if (m_activeAccessPointObjectPath != activeAccessPointObjectPath) {
        m_activeAccessPointObjectPath = activeAccessPointObjectPath;
        if (m_accessPointsTable.contains(m_activeAccessPointObjectPath)) {
            if (m_activeAccessPoint)
                disconnect(m_activeAccessPoint, &WirelessAccessPoint::signalStrengthChanged, this, &WirelessNetworkDevice::deviceChanged);

            // Set new access point object
            m_activeAccessPoint = m_accessPointsTable.value(activeAccessPointObjectPath);
            // Update the device when the signalstrength changed
            connect(m_activeAccessPoint, &WirelessAccessPoint::signalStrengthChanged, this, &WirelessNetworkDevice::deviceChanged);
        } else {
            m_activeAccessPoint = Q_NULLPTR;
        }
        emit deviceChanged();
    }
}

void WirelessNetworkDevice::accessPointAdded(const QDBusObjectPath &objectPath)
{
    QDBusInterface accessPointInterface(serviceString, objectPath.path(), accessPointInterfaceString, QDBusConnection::systemBus());
    if (!accessPointInterface.isValid()) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkDevice: Invalid access point dbus interface";
        return;
    }

    if (m_accessPointsTable.keys().contains(objectPath)) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkDevice: Access point already added" << objectPath.path();
        return;
    }

    WirelessAccessPoint *accessPoint = new WirelessAccessPoint(objectPath, this);
    //qCDebug(dcNetworkManager()) << "WirelessNetworkDevice: [+]" << accessPoint;
    m_accessPointsTable.insert(objectPath, accessPoint);
}

void WirelessNetworkDevice::accessPointRemoved(const QDBusObjectPath &objectPath)
{
    if (!m_accessPointsTable.keys().contains(objectPath))
        return;

    WirelessAccessPoint *accessPoint = m_accessPointsTable.take(objectPath);
    if (accessPoint == m_activeAccessPoint)
        m_activeAccessPoint = Q_NULLPTR;

    //qCDebug(dcNetworkManager()) << "WirelessNetworkDevice: [-]" << accessPoint;
    accessPoint->deleteLater();
}

void WirelessNetworkDevice::propertiesChanged(const QVariantMap &properties)
{
    //qCDebug(dcNetworkManager()) << "WirelessNetworkDevice: Property changed" << properties;

    if (properties.contains("Bitrate"))
        setBitrate(properties.value("Bitrate").toInt());

    if (properties.contains("ActiveAccessPoint"))
        setActiveAccessPoint(qdbus_cast<QDBusObjectPath>(properties.value("ActiveAccessPoint")));
}

QDebug operator<<(QDebug debug, WirelessNetworkDevice *manager)
{
    debug.nospace() << "WirelessNetworkDevice(" << manager->interface() << ", ";
    debug.nospace() << manager->macAddress() <<  ", ";
    debug.nospace() << manager->bitRate() <<  " [Mb/s], ";
    debug.nospace() << manager->deviceStateString() <<  ") ";
    return debug;
}

}
