/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
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

#ifndef NETWORKDEVICEDISCOVERYIMPL_H
#define NETWORKDEVICEDISCOVERYIMPL_H

#include <QObject>
#include <QSettings>
#include <QLoggingCategory>
#include <QDateTime>

#include <network/networkdeviceinfo.h>
#include <network/networkdevicediscovery.h>

#include "macaddressdatabase.h"

#include "networkdevicemonitorimpl.h"
#include "macaddressdatabasereplyimpl.h"
#include "networkdevicediscoveryreplyimpl.h"

class Ping;
class ArpSocket;

Q_DECLARE_LOGGING_CATEGORY(dcNetworkDeviceDiscovery)

namespace nymeaserver {

class NetworkDeviceDiscoveryImpl : public NetworkDeviceDiscovery
{
    Q_OBJECT
public:
    explicit NetworkDeviceDiscoveryImpl(QObject *parent = nullptr);
    ~NetworkDeviceDiscoveryImpl() override;

    bool available() const override;
    bool enabled() const override;

    bool running() const override;

    NetworkDeviceDiscoveryReply *discover() override;

    NetworkDeviceMonitor *registerMonitor(const MacAddress &macAddress) override;

    void unregisterMonitor(const MacAddress &macAddress) override;
    void unregisterMonitor(NetworkDeviceMonitor *networkDeviceMonitor) override;

    PingReply *ping(const QHostAddress &address) override;

    MacAddressDatabaseReply *lookupMacAddress(const QString &macAddress) override;
    MacAddressDatabaseReply *lookupMacAddress(const MacAddress &macAddress) override;

    bool sendArpRequest(const QHostAddress &address) override;

    QHash<MacAddress, NetworkDeviceInfo> cache() const override;

protected:
    void setEnabled(bool enabled) override;

private:
    MacAddressDatabase *m_macAddressDatabase = nullptr;
    ArpSocket *m_arpSocket = nullptr;
    Ping *m_ping = nullptr;
    bool m_enabled = true;
    bool m_running = false;

    QTimer *m_discoveryTimer = nullptr;
    QTimer *m_monitorTimer = nullptr;

    QDateTime m_lastDiscovery;
    QDateTime m_lastCacheHousekeeping;

    uint m_rediscoveryInterval = 300; // 5 min
    uint m_monitorInterval = 60; // 1 min
    uint m_cacheCleanupPeriod = 30; // days

    NetworkDeviceDiscoveryReplyImpl *m_currentReply = nullptr;
    QList<PingReply *> m_runningPingRepies;

    QHash<MacAddress, NetworkDeviceMonitorImpl *> m_monitors;
    QHash<MacAddress, QDateTime> m_lastSeen;

    QSettings *m_cacheSettings;
    QHash<MacAddress, NetworkDeviceInfo> m_networkInfoCache;

    void pingAllNetworkDevices();

    void processMonitorPingResult(PingReply *reply, NetworkDeviceMonitorImpl *monitor);

    void loadNetworkDeviceCache();
    void removeFromNetworkDeviceCache(const MacAddress &macAddress);
    void saveNetworkDeviceCache(const NetworkDeviceInfo &deviceInfo);

    void updateCache(const NetworkDeviceInfo &deviceInfo);
    void evaluateMonitor(NetworkDeviceMonitorImpl *monitor);

    void processArpTraffic(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress);

    // Time helpers
    bool longerAgoThan(const QDateTime &dateTime, uint minutes);
    QDateTime convertMinuteBased(const QDateTime &dateTime = QDateTime());

private slots:
    void onArpResponseReceived(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress);
    void onArpRequstReceived(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress);
    void evaluateMonitors();
    void finishDiscovery();

};

}

#endif // NETWORKDEVICEDISCOVERYIMPL_H
