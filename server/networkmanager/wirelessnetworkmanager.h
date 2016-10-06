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

namespace guhserver {

class WirelessNetworkManager : public NetworkDevice
{
    Q_OBJECT
public:

    explicit WirelessNetworkManager(const QDBusObjectPath &objectPath, QObject *parent = 0);

    QString macAddress() const;
    int bitrate() const;

    void scanWirelessNetworks();

    QList<WirelessAccessPoint *> accessPoints();
    WirelessAccessPoint *getAccessPoint(const QString &ssid);
    WirelessAccessPoint *getAccessPoint(const QDBusObjectPath &objectPath);

private:
    QDBusInterface *m_wirelessInterface;
    QString m_macAddress;
    int m_bitrate;

    QHash<QDBusObjectPath, WirelessAccessPoint *> m_accessPointsTable;

    void readAccessPoints();

    void setConnected(const bool &connected);
    void setState(const NetworkDeviceState &state);
    void setStateReason(const NetworkDeviceStateReason &stateReason);

private slots:
    void accessPointAdded(const QDBusObjectPath &objectPath);
    void accessPointRemoved(const QDBusObjectPath &objectPath);
    void propertiesChanged(const QVariantMap &properties);

signals:
    void connectedChanged(const bool &connected);
    void stateChanged(const NetworkDeviceState &state);
};

QDebug operator<<(QDebug debug, WirelessNetworkManager *manager);

}

#endif // WIRELESSNETWORKMANAGER_H
