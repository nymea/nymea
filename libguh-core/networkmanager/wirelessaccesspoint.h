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

#ifndef WIRELESSACCESSPOINT_H
#define WIRELESSACCESSPOINT_H

#include <QObject>
#include <QDebug>
#include <QFlags>
#include <QDBusObjectPath>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusArgument>

namespace guhserver {

class WirelessAccessPoint : public QObject
{
    Q_OBJECT
    Q_FLAGS(ApSecurityModes)

public:
    enum ApSecurityMode{
        ApSecurityModeNone         = 0x000,
        ApSecurityModePairWep40    = 0x001,
        ApSecurityModePairWep104   = 0x002,
        ApSecurityModePairTkip     = 0x004,
        ApSecurityModePairCcmp     = 0x008,
        ApSecurityModeGroupWep40   = 0x010,
        ApSecurityModeGroupWep104  = 0x020,
        ApSecurityModeGroupTkip    = 0x040,
        ApSecurityModeGroupCcmp    = 0x080,
        ApSecurityModeKeyMgmtPsk   = 0x100,
        ApSecurityModeKeyMgmt8021X = 0x200,
    };
    Q_DECLARE_FLAGS(ApSecurityModes, ApSecurityMode)


    explicit WirelessAccessPoint(const QDBusObjectPath &objectPath, QObject *parent = 0);

    QDBusObjectPath objectPath() const;

    QString ssid() const;
    QString macAddress() const;
    double frequency() const;
    int signalStrength() const;
    bool isProtected() const;

    WirelessAccessPoint::ApSecurityModes securityFlags() const;

private:
    QDBusObjectPath m_objectPath;
    QString m_ssid;
    QString m_macAddress;
    double m_frequency;
    int m_signalStrength;
    bool m_isProtected;
    WirelessAccessPoint::ApSecurityModes m_securityFlags;

    void setSsid(const QString &ssid);
    void setMacAddress(const QString &macAddress);
    void setFrequency(const double &frequency);
    void setSignalStrength(const int &signalStrength);
    void setIsProtected(const bool &isProtected);
    void setSecurityFlags(const WirelessAccessPoint::ApSecurityModes &securityFlags);

signals:
    void signalStrengthChanged();

private slots:
    void onPropertiesChanged(const QVariantMap &properties);

};

QDebug operator<<(QDebug debug, WirelessAccessPoint *accessPoint);

}

Q_DECLARE_OPERATORS_FOR_FLAGS(guhserver::WirelessAccessPoint::ApSecurityModes)
Q_DECLARE_METATYPE(guhserver::WirelessAccessPoint::ApSecurityMode)

#endif // WIRELESSACCESSPOINT_H
