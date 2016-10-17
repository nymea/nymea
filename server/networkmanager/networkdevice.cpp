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

#include "networkdevice.h"
#include "loggingcategories.h"

#include <QMetaEnum>

namespace guhserver {

NetworkDevice::NetworkDevice(const QDBusObjectPath &objectPath, QObject *parent) :
    QObject(parent),
    m_objectPath(objectPath),
    m_mtu(0),
    m_metered(0),
    m_deviceState(NetworkDeviceStateUnknown),
    m_deviceStateReason(NetworkDeviceStateReasonUnknown)
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.isConnected()) {
        qCWarning(dcNetworkManager()) << "NetworkDevice: System DBus not connected";
        return;
    }

    m_networkDeviceInterface = new QDBusInterface(serviceString, m_objectPath.path(), deviceInterfaceString, QDBusConnection::systemBus(), this);
    if(!m_networkDeviceInterface->isValid()) {
        qCWarning(dcNetworkManager()) << "NetworkDevice: Invalid DBus device interface" << m_objectPath.path();
        return;
    }

    m_udi = m_networkDeviceInterface->property("Udi").toString();
    m_interface = m_networkDeviceInterface->property("Interface").toString();
    m_ipInterface = m_networkDeviceInterface->property("IpInterface").toString();
    m_driver = m_networkDeviceInterface->property("Driver").toString();
    m_driverVersion = m_networkDeviceInterface->property("DriverVersion").toString();
    m_firmwareVersion = m_networkDeviceInterface->property("FirmwareVersion").toString();
    m_physicalPortId = m_networkDeviceInterface->property("PhysicalPortId").toString();
    m_mtu = m_networkDeviceInterface->property("Mtu").toUInt();
    m_metered = m_networkDeviceInterface->property("Metered").toUInt();
    m_autoconnect = m_networkDeviceInterface->property("Autoconnect").toBool();

    m_deviceState = NetworkDeviceState(m_networkDeviceInterface->property("State").toUInt());
    m_deviceType = NetworkDeviceType(m_networkDeviceInterface->property("DeviceType").toUInt());

    m_activeConnection = qdbus_cast<QDBusObjectPath>(m_networkDeviceInterface->property("ActiveConnection"));
    m_ip4Config = qdbus_cast<QDBusObjectPath>(m_networkDeviceInterface->property("Ip4Config"));
    m_ip6Config = qdbus_cast<QDBusObjectPath>(m_networkDeviceInterface->property("Ip6Config"));

    QDBusConnection::systemBus().connect(serviceString, m_objectPath.path(), deviceInterfaceString, "StateChanged", this, SLOT(onStateChanged(uint,uint,uint)));
}

QDBusObjectPath NetworkDevice::objectPath() const
{
    return m_objectPath;
}

QString NetworkDevice::udi() const
{
    return m_udi;
}

QString NetworkDevice::interface() const
{
    return m_interface;
}

QString NetworkDevice::ipInterface() const
{
    return m_ipInterface;
}

QString NetworkDevice::driver() const
{
    return m_driver;
}

QString NetworkDevice::driverVersion() const
{
    return m_driverVersion;
}

QString NetworkDevice::firmwareVersion() const
{
    return m_firmwareVersion;
}

QString NetworkDevice::physicalPortId() const
{
    return m_physicalPortId;
}

uint NetworkDevice::mtu() const
{
    return m_mtu;
}

uint NetworkDevice::metered() const
{
    return m_metered;
}

bool NetworkDevice::autoconnect() const
{
    return m_autoconnect;
}

NetworkDevice::NetworkDeviceState NetworkDevice::deviceState() const
{
    return m_deviceState;
}

QString NetworkDevice::deviceStateString() const
{
    return NetworkDevice::deviceStateToString(m_deviceState);
}

NetworkDevice::NetworkDeviceStateReason NetworkDevice::deviceStateReason() const
{
    return m_deviceStateReason;
}

NetworkDevice::NetworkDeviceType NetworkDevice::deviceType() const
{
    return m_deviceType;
}

QDBusObjectPath NetworkDevice::activeConnection() const
{
    return m_activeConnection;
}

QDBusObjectPath NetworkDevice::ip4Config() const
{
    return m_ip4Config;
}

QList<QDBusObjectPath> NetworkDevice::availableConnections() const
{
    return m_availableConnections;
}

void NetworkDevice::disconnectDevice()
{
    QDBusMessage query = m_networkDeviceInterface->call("Disconnect");
    if(query.type() != QDBusMessage::ReplyMessage)
        qCWarning(dcNetworkManager()) << query.errorName() << query.errorMessage();

}

QString NetworkDevice::deviceTypeToString(const NetworkDevice::NetworkDeviceType &deviceType)
{
    QMetaObject metaObject = NetworkDevice::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("NetworkDeviceType").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    return QString(metaEnum.valueToKey(deviceType)).remove("NetworkDeviceType");
}

QString NetworkDevice::deviceStateToString(const NetworkDevice::NetworkDeviceState &deviceState)
{
    QMetaObject metaObject = NetworkDevice::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("NetworkDeviceState").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    return QString(metaEnum.valueToKey(deviceState));
}

QString NetworkDevice::deviceStateReasonToString(const NetworkDevice::NetworkDeviceStateReason &deviceStateReason)
{
    QMetaObject metaObject = NetworkDevice::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("NetworkDeviceStateReason").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    return QString(metaEnum.valueToKey(deviceStateReason));
}

void NetworkDevice::onStateChanged(uint newState, uint oldState, uint reason)
{
    Q_UNUSED(oldState);
    qCDebug(dcNetworkManager()) << m_interface << "--> State changed:" << deviceStateToString(NetworkDeviceState(newState)) << ":" << deviceStateReasonToString(NetworkDeviceStateReason(reason));
    if (m_deviceState != NetworkDeviceState(newState)) {
        m_deviceState = NetworkDeviceState(newState);
        emit deviceChanged();
    }
}

QDebug operator<<(QDebug debug, NetworkDevice *device)
{
    debug.nospace() << "NetworkDevice(" << device->interface() << " - " << NetworkDevice::deviceTypeToString(device->deviceType()) << ", " << device->deviceStateString() << ")";
    return debug;
}

}
