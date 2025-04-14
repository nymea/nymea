/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2024, nymea GmbH
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

#include <QHash>
#include <QObject>
#include <QSettings>
#include <QDateTime>
#include <QLoggingCategory>

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

    NetworkDeviceMonitor *registerMonitor(Thing *thing) override;
    void unregisterMonitor(NetworkDeviceMonitor *networkDeviceMonitor) override;

    PingReply *ping(const QHostAddress &address, uint retries = 3) override;
    PingReply *ping(const QString &hostName, uint retries = 3) override;
    PingReply *ping(const QHostAddress &address, bool lookupHost, uint retries = 3);

    MacAddressDatabaseReply *lookupMacAddress(const QString &macAddress) override;
    MacAddressDatabaseReply *lookupMacAddress(const MacAddress &macAddress) override;

    bool sendArpRequest(const QHostAddress &address) override;

    NetworkDeviceInfos cache() const override;

protected:
    void setEnabled(bool enabled) override;

private:
    typedef struct TargetNetwork {
        QNetworkInterface networkInterface;
        QNetworkAddressEntry addressEntry;
        QHostAddress address;
    } TargetNetwork;

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

    NetworkDeviceDiscoveryReplyImpl *m_currentDiscoveryReply = nullptr;
    QList<NetworkDeviceDiscoveryReplyImpl *> m_pendingDiscoveryReplies;
    QList<MacAddressDatabaseReply *> m_runningMacDatabaseReplies;
    QList<PingReply *> m_runningPingReplies;

    QHash<NetworkDeviceMonitorImpl *, QVector<NetworkDeviceMonitorImpl *>> m_monitors;
    QHash<QHostAddress, QDateTime> m_lastSeen;

    QHash<MacAddress, QString> m_macVendorCache;

    QHash<QHostAddress, NetworkDeviceInfo> m_infos;

    QSettings *m_cacheSettings;
    NetworkDeviceInfos m_networkInfoCache;

    void pingAllNetworkDevices();

    void processMonitorPingResult(PingReply *reply, NetworkDeviceMonitorImpl *monitor);

    void watchPingReply(PingReply *reply);

    void loadNetworkDeviceCache();
    void removeFromNetworkDeviceCache(const MacAddress &macAddress);
    void removeFromNetworkDeviceCache(const QHostAddress &address);
    void saveNetworkDeviceCache(const NetworkDeviceInfo &deviceInfo);

    void updateCache(const NetworkDeviceInfo &deviceInfo);
    void evaluateMonitor(NetworkDeviceMonitorImpl *monitor);

    void processArpTraffic(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress);

    // Time helpers
    bool longerAgoThan(const QDateTime &dateTime, uint minutes);
    QDateTime convertMinuteBased(const QDateTime &dateTime = QDateTime());

    NetworkDeviceMonitorImpl *createPluginMonitor(NetworkDeviceMonitorImpl *internalMonitor);
    void cleanupPluginMonitor(NetworkDeviceMonitorImpl *pluginMonitor);

private slots:
    void onArpResponseReceived(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress);
    void onArpRequstReceived(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress);
    void evaluateMonitors();
    void finishDiscovery();

    void onPluginMonitorDeleted(QObject *);

};

}

#endif // NETWORKDEVICEDISCOVERYIMPL_H
