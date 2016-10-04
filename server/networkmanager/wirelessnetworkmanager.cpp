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
    QObject(parent),
    m_objectPath(objectPath),
    m_connected(false),
    m_managed(false),
    m_state(DeviceStateUnknown),
    m_stateReason(DeviceStateReasonUnknown)
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.isConnected()) {
        qCWarning(dcNetworkManager()) << "System DBus not connected";
        return;
    }

    QDBusConnection::systemBus().connect(serviceString, m_objectPath.path(), wirelessInterfaceString, "AccessPointAdded", this, SLOT(accessPointAdded(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(serviceString, m_objectPath.path(), wirelessInterfaceString, "AccessPointRemoved", this, SLOT(accessPointRemoved(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(serviceString, m_objectPath.path(), deviceInterfaceString, "StateChanged", this, SLOT(deviceStateChanged(quint32,quint32,quint32)));

    readWirelessDeviceProperties();

    qCDebug(dcNetworkManager()) << this;

    readAccessPoints();
}

QDBusObjectPath WirelessNetworkManager::objectPath() const
{
    return m_objectPath;
}

QString WirelessNetworkManager::udi() const
{
    return m_udi;
}

QString WirelessNetworkManager::macAddress() const
{
    return m_macAddress;
}

QString WirelessNetworkManager::interfaceName() const
{
    return m_interfaceName;
}

QString WirelessNetworkManager::driver() const
{
    return m_driver;
}

QString WirelessNetworkManager::driverVersion() const
{
    return m_driverVersion;
}

bool WirelessNetworkManager::connected() const
{
    return m_connected;
}

bool WirelessNetworkManager::managed() const
{
    return m_managed;
}

WirelessNetworkManager::DeviceState WirelessNetworkManager::state() const
{
    return m_state;
}

WirelessNetworkManager::DeviceStateReason WirelessNetworkManager::stateReason() const
{
    return m_stateReason;
}

void WirelessNetworkManager::scanWirelessNetworks()
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.isConnected()) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkManager: System DBus not connected";
        return;
    }

    QDBusInterface wirelessInterface(serviceString, m_objectPath.path(), wirelessInterfaceString, systemBus);
    if(!wirelessInterface.isValid()) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkManager: Could not scan wireless networks: Invalid wireless dbus interface";
        return;
    }

    QDBusMessage query= wirelessInterface.call("RequestScan", QVariantMap());
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

QString WirelessNetworkManager::deviceStateToString(const WirelessNetworkManager::DeviceState &state)
{
    QMetaObject metaObject = WirelessNetworkManager::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("DeviceState").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    return QString(metaEnum.valueToKey(state)).remove("DeviceState");
}

QString WirelessNetworkManager::deviceStateReasonToString(const WirelessNetworkManager::DeviceStateReason &stateReason)
{
    QMetaObject metaObject = WirelessNetworkManager::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("DeviceStateReason").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    return QString(metaEnum.valueToKey(stateReason)).remove("DeviceStateReason");
}

void WirelessNetworkManager::readAccessPoints()
{
    QDBusInterface wirelessInterface(serviceString, m_objectPath.path(), wirelessInterfaceString, QDBusConnection::systemBus());
    if(!wirelessInterface.isValid()) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkManager: Could not read access points: Invalid wireless dbus interface";
        return;
    }

    QDBusMessage query = wirelessInterface.call("GetAccessPoints");
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

void WirelessNetworkManager::readWirelessDeviceProperties()
{
    QDBusInterface wirelessInterface(serviceString, m_objectPath.path(), wirelessInterfaceString, QDBusConnection::systemBus());
    if(!wirelessInterface.isValid()) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkManager: Could not read access points: Invalid wireless dbus interface";
        return;
    }

    QDBusInterface driverInterface(serviceString, m_objectPath.path(), deviceInterfaceString, QDBusConnection::systemBus());
    if(!driverInterface.isValid()) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkManager: Could not read driver information: Invalid driver dbus interface";
        return;
    }

    m_udi = driverInterface.property("Udi").toString();
    m_macAddress = wirelessInterface.property("HwAddress").toString();
    m_interfaceName = driverInterface.property("Interface").toString();
    m_driver = driverInterface.property("Driver").toString();
    m_driverVersion = driverInterface.property("DriverVersion").toString();

    setManaged(driverInterface.property("Managed").toBool());
    setState(DeviceState(driverInterface.property("State").toUInt()));
}

void WirelessNetworkManager::setConnected(const bool &connected)
{
    if (m_connected != connected) {
        m_connected = connected;
        emit connectedChanged(m_connected);
    }
}

void WirelessNetworkManager::setState(const DeviceState &state)
{
    m_state = state;
    emit stateChanged(m_state);

    switch (state) {
    case DeviceStateActivated:
        setConnected(true);
        break;
    default:
        setConnected(false);
        break;
    }
}

void WirelessNetworkManager::setStateReason(const WirelessNetworkManager::DeviceStateReason &stateReason)
{
    m_stateReason = stateReason;
}

void WirelessNetworkManager::setManaged(const bool &managed)
{
    m_managed = managed;
    emit managedChanged(m_managed);
}

void WirelessNetworkManager::deviceStateChanged(uint newState, uint oldState, uint reason)
{
    qCDebug(dcNetworkManager()) << "WirelessManager: state changed" << deviceStateToString(DeviceState(oldState)) << "-->" << deviceStateToString(DeviceState(newState)) << ":" << deviceStateReasonToString(DeviceStateReason(reason));

    setState(DeviceState(newState));
    setStateReason(DeviceStateReason(reason));
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

    // Add access point
    qCDebug(dcNetworkManager()) << "WirelessNetworkManager: [+]" << accessPoint;
    m_accessPointsTable.insert(objectPath, accessPoint);
}

void WirelessNetworkManager::accessPointRemoved(const QDBusObjectPath &objectPath)
{
    if (!m_accessPointsTable.keys().contains(objectPath)) {
        qCWarning(dcNetworkManager()) << "WirelessNetworkManager: Unknown access point removed" << objectPath.path();
        return;
    }

    // Remove access point
    WirelessAccessPoint *accessPoint = m_accessPointsTable.take(objectPath);
    qCDebug(dcNetworkManager()) << "WirelessNetworkManager: [-]" << accessPoint;
    accessPoint->deleteLater();
}

void WirelessNetworkManager::propertiesChanged(const QVariantMap &properties)
{
    qCDebug(dcNetworkManager()) << "WirelessNetworkManager: Property changed" << properties;
}

QDebug operator<<(QDebug debug, WirelessNetworkManager *manager)
{
    debug.nospace() << "WirelessManager(" << manager->interfaceName() << ", ";
    debug.nospace() << manager->macAddress() <<  ", ";
    debug.nospace() << manager->udi() <<  ", ";
    debug.nospace() << manager->driver() << ": " << manager->driverVersion() <<  ", ";
    debug.nospace() << WirelessNetworkManager::deviceStateToString(manager->state()) <<  ") ";
    return debug;
}

}
