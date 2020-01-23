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

#ifndef WIRELESSNETWORKMANAGER_H
#define WIRELESSNETWORKMANAGER_H

#include <QObject>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusContext>
#include <QDBusArgument>

#include "networkdevice.h"
#include "wirelessaccesspoint.h"

namespace nymeaserver {

class WirelessNetworkDevice : public NetworkDevice
{
    Q_OBJECT
    Q_PROPERTY(QString interface READ interface)
    Q_PROPERTY(QString macAddress READ macAddress)
    Q_PROPERTY(NetworkDeviceState state READ deviceState)
    Q_PROPERTY(QString bitRate READ bitRate)
    Q_PROPERTY(WirelessAccessPoint* currentAccessPoint READ activeAccessPoint)

public:

    explicit WirelessNetworkDevice(const QDBusObjectPath &objectPath, QObject *parent = nullptr);

    // Properties
    QString macAddress() const;
    int bitRate() const;
    WirelessAccessPoint *activeAccessPoint();

    // Accesspoints
    QList<WirelessAccessPoint *> accessPoints();
    WirelessAccessPoint *getAccessPoint(const QString &ssid);
    WirelessAccessPoint *getAccessPoint(const QDBusObjectPath &objectPath);

    // Methods
    void scanWirelessNetworks();

private:
    QDBusInterface *m_wirelessInterface;

    QString m_macAddress;
    int m_bitRate;
    WirelessAccessPoint *m_activeAccessPoint;
    QDBusObjectPath m_activeAccessPointObjectPath;

    QHash<QDBusObjectPath, WirelessAccessPoint *> m_accessPointsTable;

    void readAccessPoints();

    void setMacAddress(const QString &macAddress);
    void setBitrate(const int &bitRate);
    void setActiveAccessPoint(const QDBusObjectPath &activeAccessPointObjectPath);

private slots:
    void accessPointAdded(const QDBusObjectPath &objectPath);
    void accessPointRemoved(const QDBusObjectPath &objectPath);
    void propertiesChanged(const QVariantMap &properties);

signals:
    void bitRateChanged(const bool &bitRate);
    void stateChanged(const NetworkDeviceState &state);
};

QDebug operator<<(QDebug debug, WirelessNetworkDevice *device);

}

#endif // WIRELESSNETWORKMANAGER_H
