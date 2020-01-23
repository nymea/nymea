/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef WIRELESSACCESSPOINT_H
#define WIRELESSACCESSPOINT_H

#include <QObject>
#include <QDebug>
#include <QFlags>
#include <QDBusObjectPath>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusArgument>

namespace nymeaserver {

class WirelessAccessPoint : public QObject
{
    Q_OBJECT
    Q_FLAGS(ApSecurityModes)

    Q_PROPERTY(QString ssid READ ssid)
    Q_PROPERTY(QString macAddress READ macAddress)
    Q_PROPERTY(double frequency READ frequency)
    Q_PROPERTY(int signalStrength READ signalStrength NOTIFY signalStrengthChanged)
    Q_PROPERTY(bool protected READ isProtected)

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


    explicit WirelessAccessPoint(const QDBusObjectPath &objectPath, QObject *parent = nullptr);

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

Q_DECLARE_OPERATORS_FOR_FLAGS(nymeaserver::WirelessAccessPoint::ApSecurityModes)
Q_DECLARE_METATYPE(nymeaserver::WirelessAccessPoint::ApSecurityMode)

#endif // WIRELESSACCESSPOINT_H
