/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef NETWORKDEVICEDISCOVERY_H
#define NETWORKDEVICEDISCOVERY_H

#include <QTimer>
#include <QObject>
#include <QLoggingCategory>

#include "ping.h"
#include "libnymea.h"
#include "networkdevicediscoveryreply.h"

class ArpSocket;
class MacAddressDatabase;
class MacAddressDatabaseReply;

Q_DECLARE_LOGGING_CATEGORY(dcNetworkDeviceDiscovery)

class LIBNYMEA_EXPORT NetworkDeviceDiscovery : public QObject
{
    Q_OBJECT
public:
    explicit NetworkDeviceDiscovery(QObject *parent = nullptr);

    NetworkDeviceDiscoveryReply *discover();

    bool available() const;
    bool running() const;

    PingReply *ping(const QHostAddress &address);
    MacAddressDatabaseReply *lookupMacAddress(const QString &macAddress);

signals:
    void runningChanged(bool running);

private:
    MacAddressDatabase *m_macAddressDatabase = nullptr;
    ArpSocket *m_arpSocket = nullptr;
    Ping *m_ping = nullptr;
    bool m_running = false;

    QTimer *m_discoveryTimer = nullptr;
    NetworkDeviceDiscoveryReply *m_currentReply = nullptr;
    QList<PingReply *> m_runningPingRepies;

    void pingAllNetworkDevices();
    void finishDiscovery();

    void updateOrAddNetworkDeviceArp(const QNetworkInterface &interface, const QHostAddress &address, const QString &macAddress, const QString &manufacturer = QString());

private slots:
    void onArpResponseRceived(const QNetworkInterface &interface, const QHostAddress &address, const QString &macAddress);

};

#endif // NETWORKDEVICEDISCOVERY_H
