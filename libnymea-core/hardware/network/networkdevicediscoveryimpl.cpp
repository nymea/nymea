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

#include "networkdevicediscoveryimpl.h"
#include "nymeasettings.h"
#include "loggingcategories.h"


#include <math.h>

#include <network/ping.h>
#include <network/arpsocket.h>
#include <network/networkutils.h>

#define CACHE_VERSION 1

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
        if (!m_runningPingReplies.isEmpty()) {
            qCDebug(dcNetworkDeviceDiscovery()) << "Discovery timeout occurred. There are still" << m_runningPingReplies.count()  << "ping replies pending and" << m_ping->queueCount() << "addresses int the ping queue. Aborting them...";
            foreach (PingReply *reply, m_runningPingReplies) {
                reply->abort();
            }
        }

        // We still wait for any mac manufacturer lookups, since we got already a mac...
        if (!m_runningMacDatabaseReplies.isEmpty()) {
            qCDebug(dcNetworkDeviceDiscovery()) << "Discovery timeout occurred but there are still" << m_runningMacDatabaseReplies.count() << "mac database replies pending. Waiting for them to finish...";
            return;
        }

        if (m_currentDiscoveryReply) {
            qCDebug(dcNetworkDeviceDiscovery()) << "Discovery timeout occurred and all pending replies have finished.";
            finishDiscovery();
        }
    });

    // Timer for updating the monitors
    m_monitorTimer = new QTimer(this);
    m_monitorTimer->setInterval(10000);
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
        //  connect(m_currentDiscoveryReply, &NetworkDeviceDiscoveryReplyImpl::networkDeviceInfoAdded, this, &NetworkDeviceDiscoveryImpl::updateCache);
        connect(m_currentDiscoveryReply, &NetworkDeviceDiscoveryReplyImpl::finished, this, [this](){
            // Finish all pending replies
            foreach (NetworkDeviceDiscoveryReplyImpl *reply, m_pendingDiscoveryReplies) {
                // Sync all network device infos with all pending replies
                foreach (const NetworkDeviceInfo &info, m_currentDiscoveryReply->networkDeviceInfos()) {
                    reply->addCompleteNetworkDeviceInfo(info);
                }
            }

            // Update local cache right after a discovery finished
            foreach (const NetworkDeviceInfo &info, m_currentDiscoveryReply->networkDeviceInfos()) {
                updateCache(info);
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
    m_pendingDiscoveryReplies.append(reply);

    if (!available()) {
        qCWarning(dcNetworkDeviceDiscovery()) << "The network discovery is not available. Please make sure the binary has the required capability (CAP_NET_RAW) or start the application as root.";
        // Finish the discovery in the next event loop so any connections after the creation will work as expected
        QTimer::singleShot(0, this, &NetworkDeviceDiscoveryImpl::finishDiscovery);
        return reply;
    }

    if (alreadyRunning) {
        // Emit allready discovered hosts so any integration can implement safly a connect on the host hostAddressDiscovered signal
        // even if the internal discvoery was already running
        QTimer::singleShot(0, reply, [this, reply](){
            if (!m_currentDiscoveryReply)
                return;

            foreach (const QHostAddress &address, m_currentDiscoveryReply->currentCache().keys()) {
                reply->hostAddressDiscovered(address);
            }

            connect(m_currentDiscoveryReply, &NetworkDeviceDiscoveryReplyImpl::hostAddressDiscovered, reply, &NetworkDeviceDiscoveryReplyImpl::hostAddressDiscovered);
        });
    } else {
        qCInfo(dcNetworkDeviceDiscovery()) << "Starting network device discovery ...";

        connect(m_currentDiscoveryReply, &NetworkDeviceDiscoveryReplyImpl::hostAddressDiscovered, reply, &NetworkDeviceDiscoveryReplyImpl::hostAddressDiscovered);

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

NetworkDeviceMonitor *NetworkDeviceDiscoveryImpl::registerMonitor(Thing *thing)
{
    if (!thing->thingClass().interfaces().contains("networkdevice")) {
        qCWarning(dcNetworkDeviceDiscovery()) << "Cannot register network device monitor because the thing"
                                              << thing << "does not implement the \"networkdevice\" interface.";
        return nullptr;
    }

    MacAddress macAddress(thing->paramValue("macAddress").toString());
    QString hostName = thing->paramValue("hostName").toString();
    QHostAddress address(thing->paramValue("address").toString());

    NetworkDeviceInfo::MonitorMode mode = NetworkDeviceInfo::MonitorModeMac;
    if (macAddress.isNull()) {
        if (!hostName.isEmpty()) {
            mode = NetworkDeviceInfo::MonitorModeHostName;
        } else {
            if (address.isNull()) {
                qCWarning(dcNetworkDeviceDiscovery()) << "Cannot register monitor for thing" << thing <<
                    "because there are not enough information available." <<
                    "At least one parameter from the networkdevice interface needs to be provided." << thing->params();
                return nullptr;
            } else {
                mode = NetworkDeviceInfo::MonitorModeIp;
            }
        }
    } // else we use the MAC address mode

    // Check if we already have a monitor for these settings...
    NetworkDeviceMonitorImpl *internalMonitor = nullptr;
    for (QHash<NetworkDeviceMonitorImpl *, QVector<NetworkDeviceMonitorImpl *>>::const_iterator iterator = m_monitors.cbegin(),
         end = m_monitors.cend(); iterator != end; ++iterator) {
        if (iterator.key()->address() == address && iterator.key()->macAddress() == macAddress && iterator.key()->hostName() == hostName) {
            internalMonitor = iterator.key();
            break;
        }
    }

    bool newMonitor = true;
    if (internalMonitor) {
        qCDebug(dcNetworkDeviceDiscovery()) << "Already have an internal monitor for this network device" << internalMonitor;
        newMonitor = false;
    } else {
        // Create a new monitor for the internal use
        internalMonitor = new NetworkDeviceMonitorImpl(macAddress, hostName, address, this);
        m_monitors.insert(internalMonitor, QVector<NetworkDeviceMonitorImpl *>());
    }

    internalMonitor->setMonitorMode(mode);

    if (m_networkInfoCache.isEmpty()) {
        qCDebug(dcNetworkDeviceDiscovery()) << "Cache is empty. Starting an internal discovery...";
        NetworkDeviceDiscoveryReply *reply = discover();
        connect(reply, &NetworkDeviceDiscoveryReply::finished, reply, &NetworkDeviceDiscoveryReply::deleteLater);
    }

    if (address == QHostAddress::LocalHost || hostName == "localhost") {
        // Special case, we create for localhost a networkdevice info, since we are not discovering localhost
        internalMonitor->setNetworkDeviceInfo(NetworkDeviceInfo(QHostAddress("127.0.0.1")));
    } else {
        // Find and set the network device info
        for (int i = 0; i < m_networkInfoCache.count(); i++) {
            const NetworkDeviceInfo networkDeviceInfo = m_networkInfoCache.at(i);

            switch (internalMonitor->monitorMode()) {
            case NetworkDeviceInfo::MonitorModeMac:
                // Search the unique mac address
                if (networkDeviceInfo.macAddressInfos().hasMacAddress(internalMonitor->macAddress())) {
                    qCDebug(dcNetworkDeviceDiscovery()) << "MAC monitor:" << networkDeviceInfo;
                    internalMonitor->setNetworkDeviceInfo(networkDeviceInfo);
                }
                break;
            case NetworkDeviceInfo::MonitorModeHostName:
                // Search the hostname in the cache
                if (networkDeviceInfo.hostName() == internalMonitor->hostName()) {
                    qCDebug(dcNetworkDeviceDiscovery()) << "Host name monitor:" << networkDeviceInfo;
                    internalMonitor->setNetworkDeviceInfo(networkDeviceInfo);
                }
                break;
            case NetworkDeviceInfo::MonitorModeIp:
                // Search the IP in the cache
                if (networkDeviceInfo.address() == internalMonitor->address()) {
                    qCDebug(dcNetworkDeviceDiscovery()) << "IP monitor:" << networkDeviceInfo;
                    internalMonitor->setNetworkDeviceInfo(networkDeviceInfo);
                }
                break;
            }
        }
    }

    // Create a new plugin monitor object we are going to return...
    NetworkDeviceMonitorImpl *pluginMonitor = createPluginMonitor(internalMonitor);
    m_monitors[internalMonitor].append(pluginMonitor);
    qCDebug(dcNetworkDeviceDiscovery()) << "Registered successfully" << pluginMonitor;

    // In case this is a new monitor, let's evaluate it right the way so know asap if the device is reachable or not
    if (newMonitor)
        evaluateMonitor(internalMonitor);

    return pluginMonitor;
}

void NetworkDeviceDiscoveryImpl::unregisterMonitor(NetworkDeviceMonitor *networkDeviceMonitor)
{
    if (!networkDeviceMonitor)
        return;

    cleanupPluginMonitor(qobject_cast<NetworkDeviceMonitorImpl *>(networkDeviceMonitor));
}

PingReply *NetworkDeviceDiscoveryImpl::ping(const QHostAddress &address, uint retries)
{
    PingReply *reply = m_ping->ping(address, retries);
    // Note: we use any ping used trough this method also for the monitor evaluation
    watchPingReply(reply);
    return reply;
}

PingReply *NetworkDeviceDiscoveryImpl::ping(const QString &hostName, uint retries)
{
    PingReply *reply = m_ping->ping(hostName, retries);
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

NetworkDeviceInfos NetworkDeviceDiscoveryImpl::cache() const
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
    QList<QHostAddress> ownAddresses;
    QList<TargetNetwork> targetNetworks;

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

            // Only IPv4
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            // Store the own address of this network interface in any case,
            // since we don't want to ping our self
            ownAddresses.append(entry.ip());

            TargetNetwork targetNetwork;
            targetNetwork.networkInterface = networkInterface;
            targetNetwork.addressEntry = entry;
            targetNetwork.address = QHostAddress(entry.ip().toIPv4Address() & entry.netmask().toIPv4Address());

            qCDebug(dcNetworkDeviceDiscovery()) << "  Checking entry" << entry.ip().toString();
            qCDebug(dcNetworkDeviceDiscovery()) << "    Host address:" << entry.ip().toString();
            qCDebug(dcNetworkDeviceDiscovery()) << "    Network address:" << targetNetwork.address.toString();
            qCDebug(dcNetworkDeviceDiscovery()) << "    Broadcast address:" << entry.broadcast().toString();
            qCDebug(dcNetworkDeviceDiscovery()) << "    Netmask:" << entry.netmask().toString();
            qCDebug(dcNetworkDeviceDiscovery()) << "    Address rang from" << targetNetwork.address.toString() << "-->" << targetNetwork.addressEntry.broadcast().toString();

            // Let's scan only 255.255.255.0 networks for now
            if (entry.prefixLength() < 24) {
                qCDebug(dcNetworkDeviceDiscovery()) << "Skipping network interface" << networkInterface.name() << "because there are to many hosts to contact. The network detector was designed for /24 networks.";
                continue;
            }

            // Filter out duplicated networks (for example connected using wifi and ethernet to the same network) ...

            bool duplicatedNetwork = false;
            foreach (const TargetNetwork &tn, targetNetworks) {
                if (tn.address == targetNetwork.address && tn.addressEntry.netmask() == targetNetwork.addressEntry.netmask()) {
                    qCDebug(dcNetworkDeviceDiscovery()) << "Skipping network interface" << targetNetwork.networkInterface.name() << targetNetwork.address.toString() << "as ping target network because it seems to be the same network as" << tn.networkInterface.name() << tn.address.toString();
                    duplicatedNetwork = true;
                    break;
                }
            }

            if (duplicatedNetwork)
                continue;

            targetNetworks.append(targetNetwork);
        }
    }

    foreach (const TargetNetwork &targetNetwork, targetNetworks) {

        // Send ping request to each address within the range
        quint32 targetHostsCount = pow(2, 32 - targetNetwork.addressEntry.prefixLength()) - 1;
        for (quint32 i = 1; i < targetHostsCount; i++) {
            QHostAddress targetAddress(targetNetwork.address.toIPv4Address() + i);

            // Skip the broadcast
            if (targetAddress == targetNetwork.addressEntry.broadcast())
                continue;

            // Skip our self
            if (ownAddresses.contains(targetAddress))
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

                if (m_runningPingReplies.isEmpty() && !m_runningMacDatabaseReplies.isEmpty()) {
                    qCDebug(dcNetworkDeviceDiscovery()) << "All ping replies have finished but there are still" << m_runningMacDatabaseReplies.count() << "mac db lookups pending. Waiting for them to finish...";
                    return;
                }

                if (m_runningPingReplies.isEmpty() && m_runningMacDatabaseReplies.isEmpty() && m_currentDiscoveryReply && !m_discoveryTimer->isActive()) {
                    qCDebug(dcNetworkDeviceDiscovery()) << "All pending replies have finished.";
                    finishDiscovery();
                }
            });
        }
    }
}

void NetworkDeviceDiscoveryImpl::processMonitorPingResult(PingReply *reply, NetworkDeviceMonitorImpl *monitor)
{
    // Save the last time we tried to communicate
    if (reply->error() == PingReply::ErrorNoError) {
        qCDebug(dcNetworkDeviceDiscovery()) << "Ping response from" << monitor << reply->duration() << "ms";

        QDateTime currentDateTime = QDateTime::currentDateTime();
        m_lastSeen[reply->targetHostAddress()] = currentDateTime;

        for (int i = 0; i < m_networkInfoCache.count(); i++) {

            if (monitor->monitorMode() == NetworkDeviceInfo::MonitorModeHostName) {
                if (m_networkInfoCache.at(i).hostName() == monitor->hostName() && m_networkInfoCache.at(i).address() != reply->targetHostAddress()) {
                    QHostAddress oldAddress = m_networkInfoCache.at(i).address();
                    qCDebug(dcNetworkDeviceDiscovery()) << "Hostname" << monitor->hostName() << "changed the IP address from"
                                                        << oldAddress.toString()
                                                        << "-->"
                                                        << reply->targetHostAddress().toString();

                    removeFromNetworkDeviceCache(oldAddress);

                    NetworkDeviceInfo info = m_networkInfoCache.at(i);
                    info.setAddress(reply->targetHostAddress());

                    monitor->setNetworkDeviceInfo(info);
                    m_networkInfoCache[i] = info;
                    m_networkInfoCache.sortNetworkDevices();
                    saveNetworkDeviceCache(info);
                    break;
                }
            }
        }

        monitor->setLastSeen(currentDateTime);
        monitor->setReachable(true);
    } else {
        qCDebug(dcNetworkDeviceDiscovery()) << "Failed to ping device from" << monitor << "retrying" << reply->retries() << "times:" << reply->error();
        monitor->setReachable(false);
    }
}

void NetworkDeviceDiscoveryImpl::watchPingReply(PingReply *reply)
{
    connect(reply, &PingReply::finished, this, [this, reply](){
        if (reply->error() == PingReply::ErrorNoError) {

            int index = m_networkInfoCache.indexFromHostAddress(reply->targetHostAddress());
            if (index < 0)
                return;

            m_lastSeen[reply->targetHostAddress()] = QDateTime::currentDateTime();
            saveNetworkDeviceCache(m_networkInfoCache.at(index));
        }

        // Update any relevant monitor
        foreach (NetworkDeviceMonitorImpl *monitor, m_monitors.keys()) {
            if ((monitor->monitorMode() == NetworkDeviceInfo::MonitorModeIp && reply->targetHostAddress() == monitor->address()) ||
                (monitor->monitorMode() == NetworkDeviceInfo::MonitorModeHostName && reply->hostName() == monitor->hostName()) ||
                (monitor->monitorMode() == NetworkDeviceInfo::MonitorModeMac && reply->targetHostAddress() == monitor->networkDeviceInfo().address())) {
                processMonitorPingResult(reply, monitor);
            }
        }
    });
}

void NetworkDeviceDiscoveryImpl::loadNetworkDeviceCache()
{
    qCInfo(dcNetworkDeviceDiscovery()) << "Loading cached network device information from" << m_cacheSettings->fileName();

    m_networkInfoCache.clear();
    QDateTime currentDateTime = QDateTime::currentDateTime();

    uint cacheVersion = m_cacheSettings->value("version", 0).toUInt();

    // Load only the cache if we have the same format, otherwise we discard the cache and start over with the current version
    // No need to migrate the caches, a new discovery will to that for us ...

    if (cacheVersion == CACHE_VERSION) {
        m_cacheSettings->beginGroup("NetworkDeviceInfos");
        foreach (const QString &addressString, m_cacheSettings->childGroups()) {

            m_cacheSettings->beginGroup(addressString);

            QHostAddress address(addressString);
            QDateTime lastSeen = QDateTime::fromMSecsSinceEpoch(m_cacheSettings->value("lastSeen").toLongLong());

            // Remove the info from the cache if not seen fo the last 30 days...
            if (lastSeen.date().addDays(m_cacheCleanupPeriod) < currentDateTime.date()) {
                qCDebug(dcNetworkDeviceDiscovery()) << "Removing network device cache entry since it did not show up within the last" << m_cacheCleanupPeriod << "days" << address.toString();
                m_cacheSettings->remove("");
                m_cacheSettings->endGroup(); // mac address
                continue;
            }

            NetworkDeviceInfo info(address);
            info.setHostName(m_cacheSettings->value("hostName").toString());
            info.setNetworkInterface(QNetworkInterface::interfaceFromName(m_cacheSettings->value("interface").toString()));

            int size = m_cacheSettings->beginReadArray("mac");
            for (int i = 0; i < size; i++) {
                m_cacheSettings->setArrayIndex(i);
                MacAddress macAddress(m_cacheSettings->value("mac").toString());
                QString vendor = m_cacheSettings->value("vendor").toString();
                info.addMacAddress(macAddress, vendor);
                // Cache the mac information for less DB access
                if (!macAddress.isNull() && !vendor.isEmpty()) {
                    m_macVendorCache[macAddress] = vendor;
                }
            }
            m_cacheSettings->endArray();
            m_cacheSettings->endGroup(); // address
            qCDebug(dcNetworkDeviceDiscovery()) << "Loaded cached" << info << "last seen" << lastSeen.toString();
            m_networkInfoCache.append(info);
            m_lastSeen[info.address()] = lastSeen;
        }
        m_cacheSettings->endGroup(); // NetworkDeviceInfos

    } else {
        qCDebug(dcNetworkDeviceDiscovery()) << "The cache format version has changed. Discard the network device cache ...";
        m_cacheSettings->setValue("version", CACHE_VERSION);

        m_cacheSettings->beginGroup("NetworkDeviceInfos");
        m_cacheSettings->remove("");
        m_cacheSettings->endGroup(); // NetworkDeviceInfos
    }

    qCInfo(dcNetworkDeviceDiscovery()) << "Loaded" << m_networkInfoCache.count() << "network device infos from cache.";

    // Add the localhost
    NetworkDeviceInfo localhostInfo(QHostAddress::LocalHost);
    localhostInfo.setHostName("localhost");
    m_networkInfoCache.append(localhostInfo);

    // We just did some housekeeping while loading from the cache
    m_lastCacheHousekeeping = QDateTime::currentDateTime();
}

void NetworkDeviceDiscoveryImpl::removeFromNetworkDeviceCache(const QHostAddress &address)
{
    if (address.isNull())
        return;

    m_networkInfoCache.removeHostAddress(address);
    m_lastSeen.remove(address);
    m_cacheSettings->beginGroup("NetworkDeviceInfos");
    m_cacheSettings->beginGroup(address.toString());
    m_cacheSettings->remove("");
    m_cacheSettings->endGroup(); // address
    m_cacheSettings->endGroup(); // NetworkDeviceInfos
    m_cacheSettings->sync();
}

void NetworkDeviceDiscoveryImpl::saveNetworkDeviceCache(const NetworkDeviceInfo &deviceInfo)
{
    if (!deviceInfo.isValid() || !deviceInfo.isComplete() || deviceInfo.address() == QHostAddress::LocalHost)
        return;

    m_cacheSettings->beginGroup("NetworkDeviceInfos");
    m_cacheSettings->beginGroup(deviceInfo.address().toString());
    m_cacheSettings->setValue("hostName", deviceInfo.hostName());
    m_cacheSettings->setValue("interface", deviceInfo.networkInterface().name());
    m_cacheSettings->setValue("lastSeen", convertMinuteBased(m_lastSeen.value(deviceInfo.address())).toMSecsSinceEpoch());

    if (!deviceInfo.macAddressInfos().isEmpty()){
        m_cacheSettings->beginWriteArray("mac");
        for (int i = 0; i < deviceInfo.macAddressInfos().size(); i++) {
            m_cacheSettings->setArrayIndex(i);
            m_cacheSettings->setValue("mac", deviceInfo.macAddressInfos().at(i).macAddress().toString());
            m_cacheSettings->setValue("vendor", deviceInfo.macAddressInfos().at(i).vendorName());
        }
        m_cacheSettings->endArray(); // mac
    }

    m_cacheSettings->endGroup(); // address
    m_cacheSettings->endGroup(); // NetworkDeviceInfos
    m_cacheSettings->sync();
}

void NetworkDeviceDiscoveryImpl::updateCache(const NetworkDeviceInfo &deviceInfo)
{
    // Update monitors
    foreach (NetworkDeviceMonitorImpl *monitor, m_monitors.keys()) {
        if (monitor->isMyNetworkDeviceInfo(deviceInfo)) {
            monitor->setNetworkDeviceInfo(deviceInfo);
            break;
        }
    }

    // Save only if changed
    int index = m_networkInfoCache.indexFromHostAddress(deviceInfo.address());

    if (index >= 0 && m_networkInfoCache.at(index) == deviceInfo)
        return;

    if (index < 0) {
        m_networkInfoCache.append(deviceInfo);
    } else {
        m_networkInfoCache[index] = deviceInfo;
    }

    saveNetworkDeviceCache(deviceInfo);
}

void NetworkDeviceDiscoveryImpl::evaluateMonitor(NetworkDeviceMonitorImpl *monitor)
{
    if (monitor->currentPingReply()) {
        qCDebug(dcNetworkDeviceDiscovery()) << "Monitor has still a ping reply pending:" << monitor;
        return;
    }

    qCDebug(dcNetworkDeviceDiscovery()) << "Evaluate monitor" << monitor;

    // Start action if we have not seen the device for gracePeriod seconds
    QDateTime currentDateTime = QDateTime::currentDateTime();

    bool requiresRefresh = false;

    if (!monitor->networkDeviceInfo().isValid()) {
        qCDebug(dcNetworkDeviceDiscovery()) << "Network device info not valid for" << monitor;
        requiresRefresh = true;
    }

    if (monitor->lastSeen().isNull()) {
        qCDebug(dcNetworkDeviceDiscovery()) << monitor << "requires refresh. Not seen since application start.";
        requiresRefresh = true;
    }

    if (!requiresRefresh && currentDateTime > monitor->lastSeen().addSecs(m_monitorInterval)) {
        qCDebug(dcNetworkDeviceDiscovery()) << monitor << "requires refresh. Not see since" << (currentDateTime.toMSecsSinceEpoch() - monitor->lastSeen().toMSecsSinceEpoch()) / 1000.0 << "s";
        requiresRefresh = true;
    }

    if (!requiresRefresh && monitor->networkDeviceInfo().isComplete() && currentDateTime <= monitor->lastSeen().addSecs(m_monitorInterval)) {
        // We have seen this device within the last minute, make sure the monitor is reachable
        monitor->setReachable(true);
        return;
    }

    if (!requiresRefresh && monitor->reachable() && currentDateTime <= monitor->lastConnectionAttempt().addSecs(m_rediscoveryInterval)) {
        // Note: in some cases only a failed ping is able to mark a device as offline if some ARP proxy is responding to our ARP requests.
        // Ping all network devices in any case once in a while, even if we think it is reachable.
        qCDebug(dcNetworkDeviceDiscovery()) << monitor << "performing verification ping.";
        requiresRefresh = true;
    }

    if (!requiresRefresh)
        return;

    // Try to ping first.

    // IMPORTANT: use the ping methods from this object, so the result will automatically
    // be evaluated for the monitors and cache
    PingReply *reply = nullptr;
    if (monitor->monitorMode() == NetworkDeviceInfo::MonitorModeHostName) {
        qCDebug(dcNetworkDeviceDiscovery()) << monitor << "try to ping" << monitor->hostName();
        reply = ping(monitor->hostName(), monitor->pingRetries());
    } else {
        qCDebug(dcNetworkDeviceDiscovery()) << monitor << "try to ping" << monitor->networkDeviceInfo().address().toString();
        reply = ping(monitor->networkDeviceInfo().address(), monitor->pingRetries());
    }

    monitor->setCurrentPingReply(reply);
    monitor->setLastConnectionAttempt(currentDateTime);

    connect(reply, &PingReply::retry, monitor, [monitor](PingReply::Error error, uint retryCount){
        Q_UNUSED(error)
        Q_UNUSED(retryCount)
        monitor->setLastConnectionAttempt(QDateTime::currentDateTime());
    });

    connect(reply, &PingReply::destroyed, monitor, [monitor, reply](){
        if (monitor->currentPingReply() == reply) {
            monitor->setCurrentPingReply(nullptr);
        }
    });

    connect(reply, &PingReply::finished, monitor, [monitor, reply](){
        if (monitor->currentPingReply() == reply) {
            monitor->setCurrentPingReply(nullptr);
        }
    });
}

void NetworkDeviceDiscoveryImpl::processArpTraffic(const QNetworkInterface &interface, const QHostAddress &address, const MacAddress &macAddress)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    m_lastSeen[address] = currentDateTime;

    // Update monitors and cache
    for (int i = 0; i < m_networkInfoCache.count(); i++) {
        switch (m_networkInfoCache.at(i).monitorMode()) {
        case NetworkDeviceInfo::MonitorModeMac:
            if (m_networkInfoCache.at(i).macAddressInfos().hasMacAddress(macAddress) &&
                m_networkInfoCache.at(i).address() != address) {

                QHostAddress oldAddress = m_networkInfoCache.at(i).address();
                qCDebug(dcNetworkDeviceDiscovery()) << "Host" << macAddress.toString() << "changed the IP address from"
                                                    << oldAddress.toString()
                                                    << "-->"
                                                    << address.toString();

                removeFromNetworkDeviceCache(oldAddress);

                NetworkDeviceInfo info = m_networkInfoCache.at(i);
                info.setAddress(address);

                m_networkInfoCache[i] = info;
                m_networkInfoCache.sortNetworkDevices();
                saveNetworkDeviceCache(info);

                foreach (NetworkDeviceMonitorImpl *monitor, m_monitors.keys()) {
                    if (monitor->macAddress() == macAddress) {
                        monitor->setNetworkDeviceInfo(info);
                        monitor->setLastSeen(currentDateTime);
                        monitor->setReachable(true);
                        break;
                    }
                }
                break;
            }
            break;
        case NetworkDeviceInfo::MonitorModeHostName:
        case NetworkDeviceInfo::MonitorModeIp:
            saveNetworkDeviceCache(m_networkInfoCache.at(i));
            break;
        }
    }

    // Check if we have currently  reply running
    if (!m_currentDiscoveryReply)
        return;

    // First process the response
    m_currentDiscoveryReply->processArpResponse(interface, address, macAddress);

    // Check if we know the mac address manufacturer from the cache
    bool requiresMacAddressLookup = true;
    if (m_macVendorCache.contains(macAddress)) {
        QString cachedManufacturer = m_macVendorCache.value(macAddress);
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
            if (!reply->manufacturer().isEmpty())
                m_macVendorCache.insert(macAddress, reply->manufacturer());

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
    uint duration = (QDateTime::currentMSecsSinceEpoch() - dateTime.toMSecsSinceEpoch()) / 1000.0;
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

NetworkDeviceMonitorImpl *NetworkDeviceDiscoveryImpl::createPluginMonitor(NetworkDeviceMonitorImpl *internalMonitor)
{
    NetworkDeviceMonitorImpl *pluginMonitor = new NetworkDeviceMonitorImpl(internalMonitor->macAddress(), internalMonitor->hostName(), internalMonitor->address(), this);
    pluginMonitor->setNetworkDeviceInfo(internalMonitor->networkDeviceInfo());
    pluginMonitor->setMonitorMode(internalMonitor->monitorMode());
    pluginMonitor->setLastSeen(internalMonitor->lastSeen());
    pluginMonitor->setLastConnectionAttempt(internalMonitor->lastConnectionAttempt());
    pluginMonitor->setPingRetries(internalMonitor->pingRetries());
    pluginMonitor->setReachable(internalMonitor->reachable());

    // internal monitor --> plugin monitor
    connect(internalMonitor, &NetworkDeviceMonitorImpl::reachableChanged, pluginMonitor, [pluginMonitor](bool reachable){
        pluginMonitor->setReachable(reachable);
    });
    connect(internalMonitor, &NetworkDeviceMonitorImpl::lastSeenChanged, pluginMonitor, [pluginMonitor](const QDateTime &lastSeen){
        pluginMonitor->setLastSeen(lastSeen);
    });
    connect(internalMonitor, &NetworkDeviceMonitorImpl::networkDeviceInfoChanged, pluginMonitor, [pluginMonitor](const NetworkDeviceInfo &networkDeviceInfo){
        pluginMonitor->setNetworkDeviceInfo(networkDeviceInfo);
    });

    // plugin monitor --> internal monitor
    connect(pluginMonitor, &NetworkDeviceMonitorImpl::pingRetriesChanged, internalMonitor, [internalMonitor](uint pingRetries){
        internalMonitor->setPingRetries(pingRetries);
    });

    // In case the plugin user is deleting the monitor object, we need to clean up here and check if we can remove the internal monitor
    connect(pluginMonitor, &NetworkDeviceMonitorImpl::destroyed, this, &NetworkDeviceDiscoveryImpl::onPluginMonitorDeleted);

    return pluginMonitor;
}

void NetworkDeviceDiscoveryImpl::cleanupPluginMonitor(NetworkDeviceMonitorImpl *pluginMonitor)
{
    qCDebug(dcNetworkDeviceDiscovery()) << "Unregister plugin monitor" << pluginMonitor;
    foreach (NetworkDeviceMonitorImpl *internalMonitor, m_monitors.keys()) {
        if (m_monitors.value(internalMonitor).contains(pluginMonitor)) {
            m_monitors[internalMonitor].removeAll(pluginMonitor);
            disconnect(pluginMonitor, &NetworkDeviceMonitorImpl::destroyed, this, &NetworkDeviceDiscoveryImpl::onPluginMonitorDeleted);
            pluginMonitor->deleteLater();

            if (m_monitors.value(internalMonitor).isEmpty()) {
                qCDebug(dcNetworkDeviceDiscovery()) << "No monitor registered for this network device any more. Unregister internal monitor" << internalMonitor;
                // Last refference for this monitor, nobody need this any more. Clean up...
                m_monitors.remove(internalMonitor);
                internalMonitor->deleteLater();
            }
        }
    }
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
    foreach (NetworkDeviceMonitorImpl *monitor, m_monitors.keys()) {
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

    // FIXME
    // // Do some cache housekeeping if required
    // if (m_lastCacheHousekeeping.addDays(1) < QDateTime::currentDateTime()) {
    //     qCInfo(dcNetworkDeviceDiscovery()) << "Starting cache housekeeping since it is more than one day since the last clanup...";
    //     QDateTime currentDateTime = QDateTime::currentDateTime();
    //     foreach (const MacAddress &mac, m_lastSeen.keys()) {
    //         // Remove the info from the cache if not seen fo the last 30 days...
    //         if (m_lastSeen.value(mac).date().addDays(m_cacheCleanupPeriod) < QDateTime::currentDateTime().date()) {
    //             qCDebug(dcNetworkDeviceDiscovery()) << "Removing network device cache entry since it did not show up within the last" << m_cacheCleanupPeriod << "days" << mac.toString();
    //             removeFromNetworkDeviceCache(mac);
    //         }
    //     }
    //     m_lastCacheHousekeeping = currentDateTime;
    // }
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

void NetworkDeviceDiscoveryImpl::onPluginMonitorDeleted(QObject *)
{
    cleanupPluginMonitor(qobject_cast<NetworkDeviceMonitorImpl *>(sender()));
}

}
