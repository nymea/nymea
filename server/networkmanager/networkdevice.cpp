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
    m_deviceState(DeviceStateUnknown),
    m_deviceStateReason(DeviceStateReasonUnknown)
{

    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.isConnected()) {
        qCWarning(dcNetworkManager()) << "NetworkDevice: System DBus not connected";
        return;
    }

    QDBusInterface networkDeviceInterface(serviceString, m_objectPath.path(), deviceInterfaceString, QDBusConnection::systemBus());
    if(!networkDeviceInterface.isValid()) {
        qCWarning(dcNetworkManager()) << "NetworkDevice: Invalid DBus device interface" << m_objectPath.path();
        return;
    }

    m_udi = networkDeviceInterface.property("Udi").toString();
    m_interface = networkDeviceInterface.property("Interface").toString();
    m_ipInterface = networkDeviceInterface.property("IpInterface").toString();
    m_driver = networkDeviceInterface.property("Driver").toString();
    m_driverVersion = networkDeviceInterface.property("DriverVersion").toString();
    m_firmwareVersion = networkDeviceInterface.property("FirmwareVersion").toString();
    m_physicalPortId = networkDeviceInterface.property("PhysicalPortId").toString();
    m_mtu = networkDeviceInterface.property("Mtu").toUInt();
    m_metered = networkDeviceInterface.property("Metered").toUInt();

    m_deviceState = DeviceState(networkDeviceInterface.property("State").toUInt());
    m_deviceType = DeviceType(networkDeviceInterface.property("DeviceType").toUInt());

    m_activeConnection = qdbus_cast<QDBusObjectPath>(networkDeviceInterface.property("ActiveConnection"));
    m_ip4Config = qdbus_cast<QDBusObjectPath>(networkDeviceInterface.property("Ip4Config"));
    m_ip6Config = qdbus_cast<QDBusObjectPath>(networkDeviceInterface.property("Ip6Config"));

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

NetworkDevice::DeviceState NetworkDevice::deviceState() const
{
    return m_deviceState;
}

NetworkDevice::DeviceStateReason NetworkDevice::deviceStateReason() const
{
    return m_deviceStateReason;
}

NetworkDevice::DeviceType NetworkDevice::deviceType() const
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

QString NetworkDevice::deviceTypeToString(const NetworkDevice::DeviceType &deviceType)
{
    QMetaObject metaObject = NetworkDevice::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("DeviceType").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    return QString(metaEnum.valueToKey(deviceType)).remove("DeviceType");
}

QString NetworkDevice::deviceStateToString(const NetworkDevice::DeviceState &deviceState)
{
    QMetaObject metaObject = NetworkDevice::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("DeviceState").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    return QString(metaEnum.valueToKey(deviceState)).remove("DeviceState");
}

QString NetworkDevice::deviceStateReasonToString(const NetworkDevice::DeviceStateReason &deviceStateReason)
{
    QMetaObject metaObject = NetworkDevice::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("DeviceStateReason").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    return QString(metaEnum.valueToKey(deviceStateReason)).remove("DeviceStateReason");
}

void NetworkDevice::onStateChanged(uint newState, uint oldState, uint reason)
{
    qCDebug(dcNetworkManager()) << m_interface << deviceStateToString(DeviceState(oldState)) << "-->" << deviceStateToString(DeviceState(newState)) << ":" << deviceStateReasonToString(DeviceStateReason(reason));

    m_deviceState = DeviceState(newState);
    emit deviceStateChanged();
}

QDebug operator<<(QDebug debug, NetworkDevice *device)
{
    debug.nospace() << "NetworkDevice(" << device->interface() << " - " << NetworkDevice::deviceTypeToString(device->deviceType()) << ")";
    return debug;
}

}
