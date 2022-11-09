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

#include "networkdevicediscoveryimpl.h"

#include "nymeasettings.h"
#include "loggingcategories.h"

#include <network/ping.h>
#include <network/arpsocket.h>
#include <network/networkutils.h>

NYMEA_LOGGING_CATEGORY(dcNetworkDeviceDiscovery, "NetworkDeviceDiscovery")

namespace nymeaserver {

NetworkDeviceDiscoveryImpl::NetworkDeviceDiscoveryImpl(QObject *parent) :
    NetworkDeviceDiscovery(parent)
{
    // Create ARP socket
    m_arpSocket = new ArpSocket(this);
    connect(m_arpSocket, &ArpSocket::arpResponse, this, &NetworkDeviceDiscoveryImpl::onArpResponseReceived);
    bool arpAvailable = m_arpSocket->openSocket();
    if (!arpAvailable)
        m_arpSocket->closeSocket();

    // Create ping socket
    m_ping = new Ping(this);
    if (!m_ping->available())
        qCWarning(dcNetworkDeviceDiscovery()) << "Failed to create ping tool" << m_ping->error();

    // Init MAC database if available
    m_macAddressDatabase = new MacAddressDatabase(this);

    // Timer for max duration af a discovery
    m_discoveryTimer = new QTimer(this);
    m_discoveryTimer->setInterval(20000);
    m_discoveryTimer->setSingleShot(true);
    connect(m_discoveryTimer, &QTimer::timeout, this, [=](){
        if (m_runningPingReplies.isEmpty() && m_runningMacDatabaseReplies.isEmpty() && m_currentDiscoveryReply) {
            qCDebug(dcNetworkDeviceDiscovery()) << "Discovery timeout occurred and all pending replies have finished.";
            finishDiscovery();
        }
    });

    // Timer for updating the monitors
    m_monitorTimer = new QTimer(this);
    m_monitorTimer->setInterval(5000);
    m_monitorTimer->setSingleShot(false);
    connect(m_monitorTimer, &QTimer::timeout, this, &NetworkDeviceDiscoveryImpl::evaluateMonitors);

    if (!arpAvailable && !m_ping->available()) {
        qCWarning(dcNetworkDeviceDiscovery()) << "Network device discovery is not available on this system.";
    } else {
        qCInfo(dcNetworkDeviceDiscovery()) << "Created successfully";
    }

    m_cacheSettings = new QSettings(NymeaSettings::cachePath() + "/network-device-discovery.cache", QSettings::IniFormat);
    loadNetworkDeviceCache();

    // Start the timer only if the resource is available
    if (available()) {
        // Start the monitor timer in any case, we do also the cache cleanup there...
        m_monitorTimer->start();
    }
}

NetworkDeviceDiscoveryImpl::~NetworkDeviceDiscoveryImpl()
{
    delete m_cacheSettings;
}

NetworkDeviceDiscoveryReply *NetworkDeviceDiscoveryImpl::discover()
{
    // Each user calling this method receives it's own reply object.
    // For internal tracking we use the current reply, but each caller gets it's own
    // reply and owns the object, even if the discovery has been finished.

    // Create the internal object if required
    bool alreadyRunning = (m_currentDiscoveryReply != nullptr);
    if (alreadyRunning) {
        if (m_currentDiscoveryReply->isFinished()) {
            qCDebug(dcNetworkDeviceDiscovery()) << "Discovery internally already running and finished.";
        } else {
            qCDebug(dcNetworkDeviceDiscovery()) << "Discovery internally already running. Re-using the current running discovery reply.";
        }
    } else {
        qCDebug(dcNetworkDeviceDiscovery()) << "Starting internally a new discovery.";
        m_currentDiscoveryReply = new NetworkDeviceDiscoveryReplyImpl(this);
        connect(m_currentDiscoveryReply, &NetworkDeviceDiscoveryReplyImpl::networkDeviceInfoAdded, this, &NetworkDeviceDiscoveryImpl::updateCache);
        connect(m_currentDiscoveryReply, &NetworkDeviceDiscoveryReplyImpl::finished, this, [this](){
            // Finish all pending replies
            foreach (NetworkDeviceDiscoveryReplyImpl *reply, m_pendingDiscoveryReplies) {

                // Sync all network device infos with all pending replies
                foreach (const NetworkDeviceInfo &info, m_currentDiscoveryReply->networkDeviceInfos()) {
                    reply->addCompleteNetworkDeviceInfo(info);
                }

                foreach (const NetworkDeviceInfo &info, m_currentDiscoveryReply->virtualNetworkDeviceInfos()) {
                    reply->addVirtualNetworkDeviceInfo(info);
                }
            }

            // Delete the current reply before finishing the pending replies.
            // Just in case some one restarts a discovery on finished, a new internal
            // object should be created
            m_currentDiscoveryReply->deleteLater();
            m_currentDiscoveryReply = nullptr;

            foreach (NetworkDeviceDiscoveryReplyImpl *reply, m_pendingDiscoveryReplies) {
                m_pendingDiscoveryReplies.removeAll(reply);
                reply->setFinished(true);
                emit reply->finished();
            }
        });
    }

    // Create the reply for the user
    NetworkDeviceDiscoveryReplyImpl *reply = new NetworkDeviceDiscoveryReplyImpl(this);
    connect(m_currentDiscoveryReply, &NetworkDeviceDiscoveryReplyImpl::networkDeviceInfoAdded, reply, &NetworkDeviceDiscoveryReplyImpl::addCompleteNetworkDeviceInfo);
    connect(m_currentDiscoveryReply, &NetworkDeviceDiscoveryReplyImpl::hostAddressDiscovered, reply, &NetworkDeviceDiscoveryReplyImpl::hostAddressDiscovered);
    m_pendingDiscoveryReplies.append(reply);

    if (!available()) {
        qCWarning(dcNetworkDeviceDiscovery()) << "The network discovery is not available. Please make sure the binary has the required capability (CAP_NET_RAW) or start the application as root.";
        // Finish the discovery in the next event loop so any connections after the creation will work as expected
        QTimer::singleShot(0, this, &NetworkDeviceDiscoveryImpl::finishDiscovery);
        return reply;
    }

    if (alreadyRunning) {
        // Add already discovered network device infos in the next event loop
        // so any connections after this method call will work as expected
        QTimer::singleShot(0, reply, [this, reply](){
            if (!m_currentDiscoveryReply)
                return;

            foreach (const NetworkDeviceInfo &networkDeviceInfo, m_currentDiscoveryReply->networkDeviceInfos()) {
                reply->addCompleteNetworkDeviceInfo(networkDeviceInfo);
            }
        });
    } else {
        qCInfo(dcNetworkDeviceDiscovery()) << "Starting network device discovery ...";

        if (m_ping->available())
            pingAllNetworkDevices();

        if (m_arpSocket->isOpen())
            m_arpSocket->sendRequest();

        m_discoveryTimer->start();

        m_running = true;
        emit runningChanged(m_running);
    }
    return reply;
}

bool NetworkDeviceDiscoveryImpl::available() const
{
    return m_arpSocket->isOpen() || m_ping->available();
}

bool NetworkDeviceDiscoveryImpl::enabled() const
{
    return m_enabled;
}

bool NetworkDeviceDiscoveryImpl::running() const
{
    return m_running;
}

NetworkDeviceMonitor *NetworkDeviceDiscoveryImpl::registerMonitor(const MacAddress &macAddress)
{
    if (macAddress.isNull()) {
        qCWarning(dcNetworkDeviceDiscovery()) << "Could not register monitor for invalid" << macAddress;
        return nullptr;
    }

    // Make sure we create only one monitor per MAC and keep track how many user
    // have access to this monitor otherwise an unregister could cause a crash in
    // an other plugin plugin which might still need it
    if (m_monitors.contains(macAddress)) {
        m_monitorsReferenceCount[macAddress] += 1;
        qCInfo(dcNetworkDeviceDiscovery()) << "Register network device monitor for" << macAddress << "which already exists. Returning existing monitor having now" << m_monitorsReferenceCount[macAddress] << "references.";
        return m_monitors.value(macAddress);
    }

    qCInfo(dcNetworkDeviceDiscovery()) << "Register new network device monitor for" << macAddress;

    // Fill in cached information
    NetworkDeviceInfo info;
    if (m_networkInfoCache.contains(macAddress)) {
        info = m_networkInfoCache.value(macAddress);
    } else {
        info.setMacAddress(macAddress.toString());
    }

    NetworkDeviceMonitorImpl *monitor = new NetworkDeviceMonitorImpl(macAddress, this);
    monitor->setNetworkDeviceInfo(info);
    m_monitors.insert(macAddress, monitor);
    m_monitorsReferenceCount[macAddress] = 1;

    if (!available()) {
        qCWarning(dcNetworkDeviceDiscovery()) << "Registered monitor but the hardware resource is not available. The monitor will not work as expected" << monitor;
        return monitor;
    }

    // Restart the monitor timer since we evaluate this one now
    m_monitorTimer->start();

    if (!monitor->networkDeviceInfo().isValid()) {
        qCDebug(dcNetworkDeviceDiscovery()) << "Adding network device monitor for unresolved mac address. Starting a discovery...";
        NetworkDeviceDiscoveryReply *reply = discover();
        connect(reply, &NetworkDeviceDiscoveryReply::finished, reply, &NetworkDeviceDiscoveryReply::deleteLater);
    }

    evaluateMonitor(monitor);

    return monitor;
}

void NetworkDeviceDiscoveryImpl::unregisterMonitor(const MacAddress &macAddress)
{
    if (m_monitorsReferenceCount.contains(macAddress)) {
        m_monitorsReferenceCount[macAddress] -= 1;
        if (m_monitorsReferenceCount[macAddress] > 0) {
            qCDebug(dcNetworkDeviceDiscovery()) << "Unregistered monitor for" << macAddress.toString() << "but keeping the monitor. There are still" << m_monitorsReferenceCount[macAddress] << "references to it.";
            return;
        }
    }

    if (m_monitors.contains(macAddress)) {
        NetworkDeviceMonitorImpl *monitor = m_monitors.take(macAddress);
        qCInfo(dcNetworkDeviceDiscovery()) << "Unregister" << monitor;
        monitor->deleteLater();
        m_monitorsReferenceCount.remove(macAddress);
    }
}

void NetworkDeviceDiscoveryImpl::unregisterMonitor(NetworkDeviceMonitor *networkDeviceMonitor)
{
    if (!networkDeviceMonitor)
        return;

    if (!m_monitors.values().contains(qobject_cast<NetworkDeviceMonitorImpl *>(networkDeviceMonitor)))
        return;

    unregisterMonitor(MacAddress(networkDeviceMonitor->networkDeviceInfo().macAddress()));
}

PingReply *NetworkDeviceDiscoveryImpl::ping(const QHostAddress &address, uint retries)
{
    PingReply *reply = m_ping->ping(address, retries);
    // Note: we use any ping used trough this method also for the monitor evaluation
    watchPingReply(reply);
    return reply;
}

PingReply *NetworkDeviceDiscoveryImpl::ping(const QHostAddress &address, bool lookupHost, uint retries)
{
    PingReply *reply = m_ping->ping(address, lookupHost, retries);
    // Note: we use any ping used trough this method also for the monitor evaluation
    watchPingReply(reply);
    return reply;
}

MacAddressDatabaseReply *NetworkDeviceDiscoveryImpl::lookupMacAddress(const QString &macAddress)
{
    return m_macAddressDatabase->lookupMacAddress(macAddress);
}

MacAddressDatabaseReply *NetworkDeviceDiscoveryImpl::lookupMacAddress(const MacAddress &macAddress)
{
    return lookupMacAddress(macAddress.toString());
}

bool NetworkDeviceDiscoveryImpl::sendArpRequest(const QHostAddress &address)
{
    if (m_arpSocket && m_arpSocket->isOpen())
        return m_arpSocket->sendRequest(address);

    return false;
}

QHash<MacAddress, NetworkDeviceInfo> NetworkDeviceDiscoveryImpl::cache() const
{
    return m_networkInfoCache;
}

void NetworkDeviceDiscoveryImpl::setEnabled(bool enabled)
{
    m_enabled = enabled;
    emit enabledChanged(m_enabled);
    // TODO: disable network discovery if false, not used for now
}

void NetworkDeviceDiscoveryImpl::pingAllNetworkDevices()
{
    qCDebug(dcNetworkDeviceDiscovery()) << "Starting ping for all network devices...";
    foreach (const QNetworkInterface &networkInterface, QNetworkInterface::allInterfaces()) {
        if (networkInterface.flags().testFlag(QNetworkInterface::IsLoopBack))
            continue;

        if (!networkInterface.flags().testFlag(QNetworkInterface::IsUp))
            continue;

        if (!networkInterface.flags().testFlag(QNetworkInterface::IsRunning))
            continue;

        qCDebug(dcNetworkDeviceDiscovery()) << "Verifying network interface" << networkInterface.name() << networkInterface.hardwareAddress() << "...";
        foreach (const QNetworkAddressEntry &entry, networkInterface.addressEntries()) {
            qCDebug(dcNetworkDeviceDiscovery()) << "  Checking entry" << entry.ip().toString();

            // Only IPv4
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            qCDebug(dcNetworkDeviceDiscovery()) << "    Host address:" << entry.ip().toString();
            qCDebug(dcNetworkDeviceDiscovery()) << "    Broadcast address:" << entry.broadcast().toString();
            qCDebug(dcNetworkDeviceDiscovery()) << "    Netmask:" << entry.netmask().toString();
            quint32 addressRangeStart = entry.ip().toIPv4Address() & entry.netmask().toIPv4Address();
            quint32 addressRangeStop = entry.broadcast().toIPv4Address() | addressRangeStart;
            quint32 range = addressRangeStop - addressRangeStart;

            // Let's scan only 255.255.255.0 networks for now
            if (range > 255)
                continue;

            qCDebug(dcNetworkDeviceDiscovery()) << "    Address range" << range << " | from" << QHostAddress(addressRangeStart).toString() << "-->" << QHostAddress(addressRangeStop).toString();
            // Send ping request to each address within the range
            for (quint32 i = 1; i < range; i++) {
                quint32 address = addressRangeStart + i;
                QHostAddress targetAddress(address);

                // Skip our self
                if (targetAddress == entry.ip())
                    continue;

                // Retry only once to ping a device and lookup the hostname on success
                PingReply *reply = ping(targetAddress, true, 1);
                m_runningPingReplies.append(reply);
                connect(reply, &PingReply::finished, this, [=](){
                    m_runningPingReplies.removeAll(reply);
                    if (reply->error() == PingReply::ErrorNoError) {
                        qCDebug(dcNetworkDeviceDiscovery()) << "Ping response from" << targetAddress.toString() << reply->hostName() << reply->duration() << "ms";
                        if (m_currentDiscoveryReply) {
                            m_currentDiscoveryReply->processPingResponse(targetAddress, reply->hostName());
                        }
                    }

                    if (m_runningPingReplies.isEmpty() && m_runningMacDatabaseReplies.isEmpty() && m_currentDiscoveryReply && !m_discoveryTimer->isActive()) {
                        qCDebug(dcNetworkDeviceDiscovery()) << "All pending replies have finished.";
                        finishDiscovery();
                    }
                });
            }
        }
    }
}

void NetworkDeviceDiscoveryImpl::processMonitorPingResult(PingReply *reply, NetworkDeviceMonitorImpl *monitor)
{
    // Save the last time we tried to communicate
    if (reply->error() == PingReply::ErrorNoError) {
        qCDebug(dcNetworkDeviceDiscovery()) << "Ping response from" << monitor << reply->duration() << "ms";
        monitor->setLastSeen(QDateTime::currentDateTime());
        monitor->setReachable(true);
    } else {
        qCDebug(dcNetworkDeviceDiscovery()) << "Failed to ping device from" << monitor << "retrying" << reply->retries() << "times:" << reply->error();
        monitor->setReachable(false);
    }
}

void NetworkDeviceDiscoveryImpl::watchPingReply(PingReply *reply)
{
    connect(reply, &PingReply::finished, this, [=](){
        // Search cache for mac address and update last seen
        if (reply->error() == PingReply::ErrorNoError) {
            foreach (const NetworkDeviceInfo &info, m_networkInfoCache) {
                if (info.address() == reply->targetHostAddress()) {
                    // Found info for this ip, update the cache
                    MacAddress macAddress(info.macAddress());
                    if (!macAddress.isNull() && m_networkInfoCache.contains(macAddress)) {
                        m_lastSeen[macAddress] = QDateTime::currentDateTime();
                        saveNetworkDeviceCache(m_networkInfoCache.value(macAddress));
                    }
                }
            }
        }

        // Update any monitor
        foreach (NetworkDeviceMonitorImpl *monitor, m_monitors.values()) {
            if (monitor->networkDeviceInfo().address() == reply->targetHostAddress()) {
                processMonitorPingResult(reply, monitor);
            }
        }
    });
}

void NetworkDeviceDiscoveryImpl::loadNetworkDeviceCache()
{
    qCInfo(dcNetworkDeviceDiscovery()) << "Loading cached network device information from" << m_cacheSettings->fileName();

    m_networkInfoCache.clear();
    QDateTime now = QDateTime::currentDateTime();

    m_cacheSettings->beginGroup("NetworkDeviceInfos");
    foreach (const QString &macAddress, m_cacheSettings->childGroups()) {
        m_cacheSettings->beginGroup(macAddress);

        MacAddress mac(macAddress);
        QDateTime lastSeen = QDateTime::fromMSecsSinceEpoch(m_cacheSettings->value("lastSeen").toLongLong());

        // Remove the info from the cache if not seen fo the last 30 days...
        if (lastSeen.date().addDays(m_cacheCleanupPeriod) < now.date()) {
            qCDebug(dcNetworkDeviceDiscovery()) << "Removing network device cache entry since it did not show up within the last" << m_cacheCleanupPeriod << "days" << mac.toString();
            m_cacheSettings->remove("");
            m_cacheSettings->endGroup(); // mac address
            continue;
        }

        NetworkDeviceInfo info(mac.toString());
        info.setAddress(QHostAddress(m_cacheSettings->value("address").toString()));
        info.setHostName(m_cacheSettings->value("hostName").toString());
        info.setMacAddressManufacturer(m_cacheSettings->value("manufacturer").toString());
        info.setNetworkInterface(QNetworkInterface::interfaceFromName(m_cacheSettings->value("interface").toString()));

        if (info.isValid() && info.isComplete()) {
            qCDebug(dcNetworkDeviceDiscovery()) << "Loaded cached" << info << "last seen" << lastSeen.toString();
            m_networkInfoCache[mac] = info;
            m_lastSeen[mac] = lastSeen;
        } else {
            qCWarning(dcNetworkDeviceDiscovery()) << "Clean up invalid cached network device info from cache" << info;
            m_cacheSettings->remove("");
        }

        m_cacheSettings->endGroup(); // mac address
    }
    m_cacheSettings->endGroup(); // NetworkDeviceInfos

    qCInfo(dcNetworkDeviceDiscovery()) << "Loaded" << m_networkInfoCache.count() << "network device infos from cache.";

    // We just did some housekeeping while loading from the cache
    m_lastCacheHousekeeping = QDateTime::currentDateTime();
}

void NetworkDeviceDiscoveryImpl::removeFromNetworkDeviceCache(const MacAddress &macAddress)
{
    if (macAddress.isNull())
        return;

    m_networkInfoCache.remove(macAddress);
    m_lastSeen.remove(macAddress);
    m_cacheSettings->beginGroup("NetworkDeviceInfos");
    m_cacheSettings->beginGroup(macAddress.toString());
    m_cacheSettings->remove("");
    m_cacheSettings->endGroup(); // mac address
    m_cacheSettings->endGroup(); // NetworkDeviceInfos
    m_cacheSettings->sync();
}

void NetworkDeviceDiscoveryImpl::saveNetworkDeviceCache(const NetworkDeviceInfo &deviceInfo)
{
    if (!deviceInfo.isValid() || !deviceInfo.isComplete())
        return;

    m_cacheSettings->beginGroup("NetworkDeviceInfos");
    m_cacheSettings->beginGroup(deviceInfo.macAddress());
    m_cacheSettings->setValue("address", deviceInfo.address().toString());
    m_cacheSettings->setValue("hostName", deviceInfo.hostName());
    m_cacheSettings->setValue("manufacturer", deviceInfo.macAddressManufacturer());
    m_cacheSettings->setValue("interface", deviceInfo.networkInterface().name());
    m_cacheSettings->setValue("lastSeen", convertMinuteBased(m_lastSeen.value(MacAddress(deviceInfo.macAddress()))).toMSecsSinceEpoch());
    m_cacheSettings->endGroup(); // mac address
    m_cacheSettings->endGroup(); // NetworkDeviceInfos
    m_cacheSettings->sync();
}

void NetworkDeviceDiscoveryImpl::updateCache(const NetworkDeviceInfo &deviceInfo)
{
    MacAddress macAddress(deviceInfo.macAddress());
    if (macAddress.isNull())
        return;

    if (m_monitors.contains(macAddress)) {
        NetworkDeviceMonitorImpl *monitor = m_monitors.value(macAddress);
        monitor->setNetworkDeviceInfo(deviceInfo);
    }

    if (m_networkInfoCache.value(macAddress) == deviceInfo)
        return;

    m_networkInfoCache[macAddress] = deviceInfo;
    saveNetworkDeviceCache(deviceInfo);
}

void NetworkDeviceDiscoveryImpl::evaluateMonitor(NetworkDeviceMonitorImpl *monitor)
{
    if (monitor->networkDeviceInfo().address().isNull())
        return;

    if (monitor->currentPingReply())
        return;

    // Start action if we have not seen the device for gracePeriod seconds
    QDateTime currentDateTime = QDateTime::currentDateTime();

    bool requiresRefresh = false;
    if (monitor->lastSeen().isNull()) {
        qCDebug(dcNetworkDeviceDiscovery()) << monitor << "requires refresh. Not seen since application start.";
        requiresRefresh = true;
    }

    if (!requiresRefresh && currentDateTime > monitor->lastSeen().addSecs(m_monitorInterval)) {
        qCDebug(dcNetworkDeviceDiscovery()) << monitor << "requires refresh. Not see since" << (currentDateTime.toMSecsSinceEpoch() - monitor->lastSeen().toMSecsSinceEpoch()) / 1000.0 << "s";
        requiresRefresh = true;
    }

    if (!requiresRefresh)
        return;

    // Try to ping first
    qCDebug(dcNetworkDeviceDiscovery()) << monitor << "try to ping" << monitor->networkDeviceInfo().address().toString();
    PingReply *reply = m_ping->ping(monitor->networkDeviceInfo().address(), monitor->pingRetries());
    monitor->setCurrentPingReply(reply);
    monitor->setLastConnectionAttempt(currentDateTime);

    connect(reply, &PingReply::retry, monitor, [=](PingReply::Error error, uint retryCount){
        Q_UNUSED(error)
        Q_UNUSED(retryCount)
        monitor->setLastConnectionAttempt(QDateTime::currentDateTime());
    });

    connect(reply, &PingReply::finished, monitor, [=](){
        monitor->setCurrentPingReply(nullptr);
        processMonitorPingResult(reply, monitor);
    });
}

void NetworkDeviceDiscoveryImpl::processArpTraffic(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress)
{
    QDateTime now = QDateTime::currentDateTime();
    m_lastSeen[macAddress] = now;

    if (m_networkInfoCache.contains(macAddress)) {
        if (m_networkInfoCache.value(macAddress).address() != address) {
            m_networkInfoCache[macAddress].setAddress(address);
            saveNetworkDeviceCache(m_networkInfoCache.value(macAddress));
        }
    }

    // Update the monitors
    NetworkDeviceMonitorImpl *monitor = m_monitors.value(macAddress);
    if (monitor) {
        monitor->setLastSeen(now);
        monitor->setReachable(true);
        if (monitor->networkDeviceInfo().address() != address) {
            NetworkDeviceInfo info = monitor->networkDeviceInfo();
            info.setAddress(address);
            monitor->setNetworkDeviceInfo(info);
            qCDebug(dcNetworkDeviceDiscovery()) << "NetworkDeviceMonitor" << monitor << "ip address changed";
            emit monitor->networkDeviceInfoChanged(monitor->networkDeviceInfo());
        }
    }

    // Check if we have currently  reply running
    if (!m_currentDiscoveryReply)
        return;

    // First process the response
    m_currentDiscoveryReply->processArpResponse(interface, address, macAddress);

    // Check if we know the mac address manufacturer from the cache
    bool requiresMacAddressLookup = true;
    if (m_networkInfoCache.contains(macAddress)) {
        QString cachedManufacturer = m_networkInfoCache[macAddress].macAddressManufacturer();
        if (!cachedManufacturer.isEmpty()) {
            // Found the mac address manufacturer in the cache, let's use that one...
            qCDebug(dcNetworkDeviceDiscovery()) << "Using cached manufacturer " << cachedManufacturer << "for" << macAddress.toString();
            m_currentDiscoveryReply->processMacManufacturer(macAddress, cachedManufacturer);
            requiresMacAddressLookup = false;
        }
    }

    if (!requiresMacAddressLookup)
        return;

    // Lookup the mac address vendor if possible
    if (m_macAddressDatabase->available()) {
        // Not found in the cache, and the mac address database is available...let's make a query
        MacAddressDatabaseReply *reply = m_macAddressDatabase->lookupMacAddress(macAddress.toString());
        m_runningMacDatabaseReplies.append(reply);
        connect(reply, &MacAddressDatabaseReply::finished, this, [this, macAddress, reply](){
            m_runningMacDatabaseReplies.removeAll(reply);

            // Note: set the mac manufacturer explicitly to make the info complete (even an empty sring)
            qCDebug(dcNetworkDeviceDiscovery()) << "MAC manufacturer lookup finished for" << macAddress << ":" << reply->manufacturer();
            if (m_currentDiscoveryReply) {
                m_currentDiscoveryReply->processMacManufacturer(macAddress, reply->manufacturer());

                if (m_runningPingReplies.isEmpty() && m_runningMacDatabaseReplies.isEmpty() && !m_discoveryTimer->isActive()) {
                    qCWarning(dcNetworkDeviceDiscovery()) << "All pending replies have finished.";
                    finishDiscovery();
                }
            }
        });
    } else {
        // Not found in the cache, and no mac address database available...we are done with mac vendor
        // Set the mac manufacturer explicitly to make the info complete
        m_currentDiscoveryReply->processMacManufacturer(macAddress, QString());
    }
}

bool NetworkDeviceDiscoveryImpl::longerAgoThan(const QDateTime &dateTime, uint seconds)
{
    uint duration = (QDateTime::currentDateTime().toMSecsSinceEpoch() - dateTime.toMSecsSinceEpoch()) / 1000.0;
    return duration >= seconds;
}

QDateTime NetworkDeviceDiscoveryImpl::convertMinuteBased(const QDateTime &dateTime)
{
    // We store the date time on minute accuracy to have
    // less write cycles and a higher resolution is not necessary

    // If the given datetime is null, use the current datetime
    QDateTime dateTimeToConvert;
    if (dateTime.isNull()) {
        dateTimeToConvert = QDateTime::currentDateTime();
    } else {
        dateTimeToConvert = dateTime;
    }

    dateTimeToConvert.setTime(QTime(dateTimeToConvert.time().hour(), dateTimeToConvert.time().minute(), 0, 0));
    return dateTimeToConvert;
}

void NetworkDeviceDiscoveryImpl::onArpResponseReceived(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress)
{
    // Ignore ARP from zero mac
    if (macAddress.isNull()) {
        qCDebug(dcNetworkDeviceDiscovery()) << "Ignoring ARP reply from" << address.toString() << macAddress.toString() << interface.name();
        return;
    }

    qCDebug(dcNetworkDeviceDiscovery()) << "ARP reply received" << address.toString() << macAddress.toString() << interface.name();
    processArpTraffic(interface, address, macAddress);
}

void NetworkDeviceDiscoveryImpl::onArpRequstReceived(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress)
{
    // Ignore ARP from zero mac
    if (macAddress.isNull()) {
        qCDebug(dcNetworkDeviceDiscovery()) << "Ignoring ARP reply from" << address.toString() << macAddress.toString() << interface.name();
        return;
    }

    qCDebug(dcNetworkDeviceDiscovery()) << "ARP request received" << address.toString() << macAddress.toString() << interface.name();
    processArpTraffic(interface, address, macAddress);
}

void NetworkDeviceDiscoveryImpl::evaluateMonitors()
{
    bool monitorRequiresRediscovery = false;
    foreach (NetworkDeviceMonitorImpl *monitor, m_monitors) {
        evaluateMonitor(monitor);

        // Check if there is any monitor which has not be seen since
        if (!monitor->reachable() && monitor->lastConnectionAttempt().isValid() && longerAgoThan(monitor->lastSeen(), m_monitorInterval)) {
            monitorRequiresRediscovery = true;
        }
    }

    if (monitorRequiresRediscovery && longerAgoThan(m_lastDiscovery, m_rediscoveryInterval)) {
        qCDebug(dcNetworkDeviceDiscovery()) << "There are unreachable monitors and the last discovery is more than" << m_rediscoveryInterval << "s ago. Starting network discovery to search the monitored network devices...";
        NetworkDeviceDiscoveryReply *reply = discover();
        connect(reply, &NetworkDeviceDiscoveryReply::finished, reply, &NetworkDeviceDiscoveryReply::deleteLater);
    }

    // Do some cache housekeeping if required
    if (m_lastCacheHousekeeping.addDays(1) < QDateTime::currentDateTime()) {
        qCInfo(dcNetworkDeviceDiscovery()) << "Starting cache housekeeping since it is more than one day since the last clanup...";
        QDateTime now = QDateTime::currentDateTime();
        foreach (const MacAddress &mac, m_lastSeen.keys()) {
            // Remove the info from the cache if not seen fo the last 30 days...
            if (m_lastSeen.value(mac).date().addDays(m_cacheCleanupPeriod) < QDateTime::currentDateTime().date()) {
                qCDebug(dcNetworkDeviceDiscovery()) << "Removing network device cache entry since it did not show up within the last" << m_cacheCleanupPeriod << "days" << mac.toString();
                removeFromNetworkDeviceCache(mac);
            }
        }

        m_lastCacheHousekeeping = now;
    }
}

void NetworkDeviceDiscoveryImpl::finishDiscovery()
{
    qCDebug(dcNetworkDeviceDiscovery()) << "Finishing discovery...";
    m_discoveryTimer->stop();

    m_running = false;
    emit runningChanged(m_running);

    emit cacheUpdated();

    m_lastDiscovery = QDateTime::currentDateTime();

    // Clean up internal reply
    if (m_currentDiscoveryReply) {
        m_currentDiscoveryReply->processDiscoveryFinished();
    }
}

}
