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

#include "wirednetworkdevice.h"
#include "loggingcategories.h"

namespace guhserver {

WiredNetworkDevice::WiredNetworkDevice(const QDBusObjectPath &objectPath, QObject *parent) :
    NetworkDevice(objectPath, parent)
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.isConnected()) {
        qCWarning(dcNetworkManager()) << "WiredNetworkDevice: System DBus not connected";
        return;
    }

    m_wiredInterface = new QDBusInterface(serviceString, this->objectPath().path(), wiredInterfaceString, systemBus, this);
    if(!m_wiredInterface->isValid()) {
        qCWarning(dcNetworkManager()) << "WiredNetworkDevice: Invalid wired dbus interface";
        return;
    }

    setMacAddress(m_wiredInterface->property("HwAddress").toString());
    setBitRate(m_wiredInterface->property("Bitrate").toInt());
    setPluggedIn(m_wiredInterface->property("Carrier").toBool());

    QDBusConnection::systemBus().connect(serviceString, this->objectPath().path(), wiredInterfaceString, "PropertiesChanged", this, SLOT(propertiesChanged(QVariantMap)));
}

QString WiredNetworkDevice::macAddress() const
{
    return m_macAddress;
}

int WiredNetworkDevice::bitRate() const
{
    return m_bitRate;
}

bool WiredNetworkDevice::pluggedIn() const
{
    return m_pluggedIn;
}

void WiredNetworkDevice::setMacAddress(const QString &macAddress)
{
    m_macAddress = macAddress;
}

void WiredNetworkDevice::setBitRate(const int &bitRate)
{
    m_bitRate = bitRate;
}

void WiredNetworkDevice::setPluggedIn(const bool &pluggedIn)
{
    m_pluggedIn = pluggedIn;
}

void WiredNetworkDevice::propertiesChanged(const QVariantMap &properties)
{
    if (properties.contains("Carrier"))
        setPluggedIn(properties.value("Carrier").toBool());

}

QDebug operator<<(QDebug debug, WiredNetworkDevice *networkDevice)
{
    debug.nospace() << "WiredNetworkDevice(" << networkDevice->interface() << ", ";
    debug.nospace() << networkDevice->macAddress() <<  ", ";
    debug.nospace() << networkDevice->bitRate() <<  " [Mb/s], ";
    debug.nospace() << networkDevice->pluggedIn() <<  ", ";
    debug.nospace() << networkDevice->deviceStateString() <<  ") ";
    return debug;
}

}
