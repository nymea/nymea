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

#include "wirelessaccesspoint.h"
#include "loggingcategories.h"
#include "dbus-interfaces.h"

namespace guhserver {

WirelessAccessPoint::WirelessAccessPoint(const QDBusObjectPath &objectPath, QObject *parent) :
    QObject(parent),
    m_objectPath(objectPath)
{
    QDBusInterface accessPointInterface(serviceString, m_objectPath.path(), accessPointInterfaceString, QDBusConnection::systemBus());
    if(!accessPointInterface.isValid()) {
        qCWarning(dcNetworkManager()) << "Invalid access point dbus interface";
        return;
    }

    // Init properties
    setSsid(accessPointInterface.property("Ssid").toString());
    setMacAddress(accessPointInterface.property("HwAddress").toString());
    setFrequency(accessPointInterface.property("Frequency").toDouble() / 1000);
    setSignalStrength(accessPointInterface.property("Strength").toUInt());
    setSecurityFlags(WirelessAccessPoint::ApSecurityModes(accessPointInterface.property("WpaFlags").toUInt()));

    QDBusConnection::systemBus().connect(serviceString, objectPath.path(), accessPointInterfaceString, "PropertiesChanged", this, SLOT(onPropertiesChanged(QVariantMap)));
}

QDBusObjectPath WirelessAccessPoint::objectPath() const
{
    return m_objectPath;
}

QString WirelessAccessPoint::ssid() const
{
    return m_ssid;
}

void WirelessAccessPoint::setSsid(const QString &ssid)
{
    m_ssid = ssid;
}

QString WirelessAccessPoint::macAddress() const
{
    return m_macAddress;
}

void WirelessAccessPoint::setMacAddress(const QString &macAddress)
{
    m_macAddress = macAddress;
}

double WirelessAccessPoint::frequency() const
{
    return m_frequency;
}

void WirelessAccessPoint::setFrequency(const double &frequency)
{
    m_frequency = frequency;
}

int WirelessAccessPoint::signalStrength() const
{
    return m_signalStrength;
}

void WirelessAccessPoint::setSignalStrength(const int &signalStrength)
{
    m_signalStrength = signalStrength;
    emit signalStrengthChanged();
}

WirelessAccessPoint::ApSecurityModes WirelessAccessPoint::securityFlags() const
{
    return m_securityFlags;
}

void WirelessAccessPoint::setSecurityFlags(const WirelessAccessPoint::ApSecurityModes &securityFlags)
{
    m_securityFlags = securityFlags;
}

void WirelessAccessPoint::onPropertiesChanged(const QVariantMap &properties)
{
    //qCDebug(dcNetworkManager()) << "AccessPoint" << ssid() << ": Properties changed" << properties;
    if (properties.contains("Strength"))
        setSignalStrength(properties.value("Strength").toUInt());

}

QDebug operator<<(QDebug debug, WirelessAccessPoint *accessPoint)
{
    return debug.nospace() << "AccessPoint(" << accessPoint->signalStrength() << "%, " <<  accessPoint->frequency()<< " GHz, " << accessPoint->ssid() << ")";
}

}
