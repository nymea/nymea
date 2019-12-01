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
    \class nymeaserver::NetworkDevice
    \brief Represents a generic network device the \l{NetworkManager}.

    \ingroup networkmanager
    \inmodule core

    \sa WiredNetworkDevice, WirelessNetworkDevice
*/

/*! \enum nymeaserver::NetworkDevice::NetworkDeviceState
    \value NetworkDeviceStateUnknown
    \value NetworkDeviceStateUnmanaged
    \value NetworkDeviceStateUnavailable
    \value NetworkDeviceStateDisconnected
    \value NetworkDeviceStatePrepare
    \value NetworkDeviceStateConfig
    \value NetworkDeviceStateNeedAuth
    \value NetworkDeviceStateIpConfig
    \value NetworkDeviceStateIpCheck
    \value NetworkDeviceStateSecondaries
    \value NetworkDeviceStateActivated
    \value NetworkDeviceStateDeactivating
    \value NetworkDeviceStateFailed
*/

/*! \enum nymeaserver::NetworkDevice::NetworkDeviceStateReason
    \value NetworkDeviceStateReasonNone
    \value NetworkDeviceStateReasonUnknown
    \value NetworkDeviceStateReasonNowManaged
    \value NetworkDeviceStateReasonNowUnmanaged
    \value NetworkDeviceStateReasonConfigFailed
    \value NetworkDeviceStateReasonIpConfigUnavailable
    \value NetworkDeviceStateReasonIpConfigExpired
    \value NetworkDeviceStateReasonNoSecrets
    \value NetworkDeviceStateReasonSupplicantDisconnected
    \value NetworkDeviceStateReasonSupplicantConfigFailed
    \value NetworkDeviceStateReasonSupplicantFailed
    \value NetworkDeviceStateReasonSupplicantTimeout
    \value NetworkDeviceStateReasonPppStartFailed
    \value NetworkDeviceStateReasonPppDisconnected
    \value NetworkDeviceStateReasonPppFailed
    \value NetworkDeviceStateReasonDhcpStartFailed
    \value NetworkDeviceStateReasonDhcpError
    \value NetworkDeviceStateReasonDhcpFailed
    \value NetworkDeviceStateReasonSharedStartFailed
    \value NetworkDeviceStateReasonSharedFailed
    \value NetworkDeviceStateReasonAutoIpStartFailed
    \value NetworkDeviceStateReasonAutoIpError
    \value NetworkDeviceStateReasonAutoIpFailed
    \value NetworkDeviceStateReasonModemBusy
    \value NetworkDeviceStateReasonModemNoDialTone
    \value NetworkDeviceStateReasonModemNoCarrier
    \value NetworkDeviceStateReasonModemDialTimeout
    \value NetworkDeviceStateReasonModemDialFailed
    \value NetworkDeviceStateReasonModemInitFailed
    \value NetworkDeviceStateReasonGsmApnFailed
    \value NetworkDeviceStateReasonGsmRegistrationNotSearching
    \value NetworkDeviceStateReasonGsmRegistrationDenied
    \value NetworkDeviceStateReasonGsmRegistrationTimeout
    \value NetworkDeviceStateReasonGsmRegistrationFailed
    \value NetworkDeviceStateReasonGsmPinCheckFailed
    \value NetworkDeviceStateReasonFirmwareMissing
    \value NetworkDeviceStateReasonRemoved
    \value NetworkDeviceStateReasonSleeping
    \value NetworkDeviceStateReasonConnectionRemoved
    \value NetworkDeviceStateReasonUserRequest
    \value NetworkDeviceStateReasonCarrier
    \value NetworkDeviceStateReasonConnectionAssumed
    \value NetworkDeviceStateReasonSupplicantAvailable
    \value NetworkDeviceStateReasonModemNotFound
    \value NetworkDeviceStateReasonBtFailed
    \value NetworkDeviceStateReasonGsmSimNotInserted
    \value NetworkDeviceStateReasonGsmSimPinRequired
    \value NetworkDeviceStateReasonGsmSimPukRequired
    \value NetworkDeviceStateReasonGsmSimWrong
    \value NetworkDeviceStateReasonInfinibandMode
    \value NetworkDeviceStateReasonDependencyFailed
    \value NetworkDeviceStateReasonBR2684Failed
    \value NetworkDeviceStateReasonModemManagerUnavailable
    \value NetworkDeviceStateReasonSsidNotFound
    \value NetworkDeviceStateReasonSecondaryConnectionFailed
    \value NetworkDeviceStateReasonDcbFoecFailed
    \value NetworkDeviceStateReasonTeamdControlFailed
    \value NetworkDeviceStateReasonModemFailed
    \value NetworkDeviceStateReasonModemAvailable
    \value NetworkDeviceStateReasonSimPinIncorrect
    \value NetworkDeviceStateReasonNewActivision
    \value NetworkDeviceStateReasonParentChanged
    \value NetworkDeviceStateReasonParentManagedChanged
*/


/*! \enum nymeaserver::NetworkDevice::NetworkDeviceType
    \value NetworkDeviceTypeUnknown
    \value NetworkDeviceTypeEthernet
    \value NetworkDeviceTypeWifi
    \value NetworkDeviceTypeBluetooth
    \value NetworkDeviceTypeOlpcMesh
    \value NetworkDeviceTypeWiMax
    \value NetworkDeviceTypeModem
    \value NetworkDeviceTypeInfiniBand
    \value NetworkDeviceTypeBond
    \value NetworkDeviceTypeVLan
    \value NetworkDeviceTypeAdsl
    \value NetworkDeviceTypeBridge
    \value NetworkDeviceTypeGeneric
    \value NetworkDeviceTypeTeam
    \value NetworkDeviceTypeTun
    \value NetworkDeviceTypeIpTunnel
    \value NetworkDeviceTypeMacVLan
    \value NetworkDeviceTypeVXLan
    \value NetworkDeviceTypeVEth
*/


/*! \fn void NetworkDevice::deviceChanged();
    This signal will be emitted when the properties of this \l{NetworkDevice} have changed.
*/


#include "networkdevice.h"
#include "loggingcategories.h"

#include <QMetaEnum>

namespace nymeaserver {

/*! Constructs a new \l{NetworkDevice} with the given dbus \a objectPath and \a parent. */
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

    if (m_ip4Config.path() != "/") {
        QDBusInterface iface(serviceString, m_ip4Config.path(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

        QDBusMessage reply = iface.call("Get", "org.freedesktop.NetworkManager.IP4Config", "AddressData");
        QVariant v = reply.arguments().first();
        QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();

        arg.beginArray();
        while(!arg.atEnd()) {
            QVariantMap m;
            arg >> m;
            m_addresses.append(m.value("address").toString());
        }
    }

    QDBusConnection::systemBus().connect(serviceString, m_objectPath.path(), deviceInterfaceString, "StateChanged", this, SLOT(onStateChanged(uint,uint,uint)));
}

/*! Returns the dbus object path of this \l{NetworkDevice}. */
QDBusObjectPath NetworkDevice::objectPath() const
{
    return m_objectPath;
}

/*! Returns the udi of this \l{NetworkDevice}. */
QString NetworkDevice::udi() const
{
    return m_udi;
}

/*! Returns the interface name of this \l{NetworkDevice}. */
QString NetworkDevice::interface() const
{
    return m_interface;
}

/*! Returns the ip interface of this \l{NetworkDevice}. */
QString NetworkDevice::ipInterface() const
{
    return m_ipInterface;
}

/*! Returns the used driver name for this \l{NetworkDevice}. */
QString NetworkDevice::driver() const
{
    return m_driver;
}

/*! Returns the version of the used driver for this \l{NetworkDevice}. */
QString NetworkDevice::driverVersion() const
{
    return m_driverVersion;
}

/*! Returns the firmware version of this \l{NetworkDevice}. */
QString NetworkDevice::firmwareVersion() const
{
    return m_firmwareVersion;
}

/*! Returns the physical port id of this \l{NetworkDevice}. */
QString NetworkDevice::physicalPortId() const
{
    return m_physicalPortId;
}

/*! Returns the mtu of this \l{NetworkDevice}. */
uint NetworkDevice::mtu() const
{
    return m_mtu;
}

/*! Returns the metered property of this \l{NetworkDevice}. */
uint NetworkDevice::metered() const
{
    return m_metered;
}

/*! Returns true if autoconnect is enabled for this \l{NetworkDevice}. */
bool NetworkDevice::autoconnect() const
{
    return m_autoconnect;
}

/*! Returns the device state of this \l{NetworkDevice}. \sa NetworkDeviceState, */
NetworkDevice::NetworkDeviceState NetworkDevice::deviceState() const
{
    return m_deviceState;
}

/*! Returns the human readable device state of this \l{NetworkDevice}. \sa NetworkDeviceState, */
QString NetworkDevice::deviceStateString() const
{
    return NetworkDevice::deviceStateToString(m_deviceState);
}

/*! Returns the reason for the current state of this \l{NetworkDevice}. \sa NetworkDeviceStateReason, */
NetworkDevice::NetworkDeviceStateReason NetworkDevice::deviceStateReason() const
{
    return m_deviceStateReason;
}

/*! Returns the device type of this \l{NetworkDevice}. \sa NetworkDeviceType, */
NetworkDevice::NetworkDeviceType NetworkDevice::deviceType() const
{
    return m_deviceType;
}

/*! Returns the dbus object path of the currently active connection of this \l{NetworkDevice}. */
QDBusObjectPath NetworkDevice::activeConnection() const
{
    return m_activeConnection;
}

/*! Returns the dbus object path from the IPv4 configuration of this \l{NetworkDevice}. */
QDBusObjectPath NetworkDevice::ip4Config() const
{
    return m_ip4Config;
}

/*! Returns the list of dbus object paths for the currently available connection of this \l{NetworkDevice}. */
QList<QDBusObjectPath> NetworkDevice::availableConnections() const
{
    return m_availableConnections;
}

/*! Disconnect the current connection from this \l{NetworkDevice}. */
void NetworkDevice::disconnectDevice()
{
    QDBusMessage query = m_networkDeviceInterface->call("Disconnect");
    if(query.type() != QDBusMessage::ReplyMessage)
        qCWarning(dcNetworkManager()) << query.errorName() << query.errorMessage();

}

/*! Returns the human readable device type string of the given \a deviceType. \sa NetworkDeviceType, */
QString NetworkDevice::deviceTypeToString(const NetworkDevice::NetworkDeviceType &deviceType)
{
    QMetaObject metaObject = NetworkDevice::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("NetworkDeviceType").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    return QString(metaEnum.valueToKey(deviceType)).remove("NetworkDeviceType");
}

/*! Returns the human readable device state string of the given \a deviceState. \sa NetworkDeviceState, */
QString NetworkDevice::deviceStateToString(const NetworkDevice::NetworkDeviceState &deviceState)
{
    QMetaObject metaObject = NetworkDevice::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("NetworkDeviceState").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);
    return QString(metaEnum.valueToKey(deviceState));
}

/*! Returns the human readable device state reason string of the given \a deviceStateReason. \sa NetworkDeviceStateReason, */
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
