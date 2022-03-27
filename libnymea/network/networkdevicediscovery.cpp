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

#include "networkdevicediscovery.h"
#include "loggingcategories.h"
#include "networkutils.h"
#include "macaddressdatabase.h"
#include "arpsocket.h"

#include <QDateTime>

NYMEA_LOGGING_CATEGORY(dcNetworkDeviceDiscovery, "NetworkDeviceDiscovery")

NetworkDeviceDiscovery::NetworkDeviceDiscovery(QObject *parent) :
    QObject(parent)
{
    // Create ARP socket
    m_arpSocket = new ArpSocket(this);
    connect(m_arpSocket, &ArpSocket::arpResponse, this, &NetworkDeviceDiscovery::onArpResponseRceived);
    bool arpAvailable = m_arpSocket->openSocket();
    if (!arpAvailable) {
        m_arpSocket->closeSocket();
    }

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
        if (m_runningPingRepies.isEmpty() && m_currentReply) {
            finishDiscovery();
        }
    });

    if (!arpAvailable && !m_ping->available()) {
        qCWarning(dcNetworkDeviceDiscovery()) << "Network device discovery is not available on this system.";
    } else {
        qCDebug(dcNetworkDeviceDiscovery()) << "Created successfully";
    }
}

NetworkDeviceDiscoveryReply *NetworkDeviceDiscovery::discover()
{
    if (m_currentReply) {
        qCDebug(dcNetworkDeviceDiscovery()) << "Discovery already running. Returning current pending discovery reply...";
        return m_currentReply;
    }

    qCDebug(dcNetworkDeviceDiscovery()) << "Starting network device discovery ...";
    NetworkDeviceDiscoveryReply *reply = new NetworkDeviceDiscoveryReply(this);
    m_currentReply = reply;
    m_currentReply->m_startTimestamp = QDateTime::currentMSecsSinceEpoch();

    if (m_ping->available()) {
        pingAllNetworkDevices();
    }

    if (m_arpSocket->isOpen()) {
        m_arpSocket->sendRequest();
    }

    m_discoveryTimer->start();
    m_running = true;
    emit runningChanged(m_running);
    return reply;
}

bool NetworkDeviceDiscovery::available() const
{
    return m_arpSocket->isOpen() || m_ping->available();
}

bool NetworkDeviceDiscovery::running() const
{
    return m_running;
}

PingReply *NetworkDeviceDiscovery::ping(const QHostAddress &address)
{
    return m_ping->ping(address);
}

MacAddressDatabaseReply *NetworkDeviceDiscovery::lookupMacAddress(const QString &macAddress)
{
    return m_macAddressDatabase->lookupMacAddress(macAddress);
}

void NetworkDeviceDiscovery::pingAllNetworkDevices()
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

                PingReply *reply = m_ping->ping(targetAddress);
                m_runningPingRepies.append(reply);
                connect(reply, &PingReply::finished, this, [=](){
                    m_runningPingRepies.removeAll(reply);
                    if (reply->error() == PingReply::ErrorNoError) {
                        qCDebug(dcNetworkDeviceDiscovery()) << "Ping response from" << targetAddress.toString() << reply->hostName() << reply->duration() << "ms";
                        int index = m_currentReply->networkDeviceInfos().indexFromHostAddress(targetAddress);
                        if (index < 0) {
                            // Add the network device
                            NetworkDeviceInfo networkDeviceInfo;
                            networkDeviceInfo.setAddress(targetAddress);
                            networkDeviceInfo.setHostName(reply->hostName());
                            m_currentReply->networkDeviceInfos().append(networkDeviceInfo);
                        } else {
                            m_currentReply->networkDeviceInfos()[index].setAddress(targetAddress);
                            m_currentReply->networkDeviceInfos()[index].setHostName(reply->hostName());
                            if (!m_currentReply->networkDeviceInfos()[index].networkInterface().isValid()) {
                                m_currentReply->networkDeviceInfos()[index].setNetworkInterface(NetworkUtils::getInterfaceForHostaddress(targetAddress));
                            }
                        }
                    }

                    if (m_runningPingRepies.isEmpty() && m_currentReply && !m_discoveryTimer->isActive()) {
                        finishDiscovery();
                    }
                });
            }
        }
    }
}

void NetworkDeviceDiscovery::finishDiscovery()
{
    m_discoveryTimer->stop();
    m_running = false;
    emit runningChanged(m_running);

    // Sort by host address
    m_currentReply->networkDeviceInfos().sortNetworkDevices();

    qint64 durationMilliSeconds = QDateTime::currentMSecsSinceEpoch() - m_currentReply->m_startTimestamp;
    qCDebug(dcNetworkDeviceDiscovery()) << "Discovery finished. Found" << m_currentReply->networkDeviceInfos().count() << "network devices in" << QTime::fromMSecsSinceStartOfDay(durationMilliSeconds).toString("mm:ss.zzz");
    emit m_currentReply->finished();
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void NetworkDeviceDiscovery::updateOrAddNetworkDeviceArp(const QNetworkInterface &interface, const QHostAddress &address, const QString &macAddress, const QString &manufacturer)
{
    if (!m_currentReply)
        return;

    int index = m_currentReply->networkDeviceInfos().indexFromHostAddress(address);
    if (index >= 0) {
        // Update the network device
        m_currentReply->networkDeviceInfos()[index].setMacAddress(macAddress);
        if (!manufacturer.isEmpty())
            m_currentReply->networkDeviceInfos()[index].setMacAddressManufacturer(manufacturer);

        if (interface.isValid()) {
            m_currentReply->networkDeviceInfos()[index].setNetworkInterface(interface);
        }
    } else {
        index = m_currentReply->networkDeviceInfos().indexFromMacAddress(macAddress);
        if (index >= 0) {
            // Update the network device
            m_currentReply->networkDeviceInfos()[index].setAddress(address);
            if (!manufacturer.isEmpty())
                m_currentReply->networkDeviceInfos()[index].setMacAddressManufacturer(manufacturer);

            if (interface.isValid()) {
                m_currentReply->networkDeviceInfos()[index].setNetworkInterface(interface);
            }
        } else {
            // Add the network device
            NetworkDeviceInfo networkDeviceInfo;
            networkDeviceInfo.setAddress(address);
            networkDeviceInfo.setMacAddress(macAddress);
            if (!manufacturer.isEmpty())
                networkDeviceInfo.setMacAddressManufacturer(manufacturer);

            if (interface.isValid())
                networkDeviceInfo.setNetworkInterface(interface);

            m_currentReply->networkDeviceInfos().append(networkDeviceInfo);
        }
    }
}

void NetworkDeviceDiscovery::onArpResponseRceived(const QNetworkInterface &interface, const QHostAddress &address, const QString &macAddress)
{
    if (!m_currentReply) {
        qCDebug(dcNetworkDeviceDiscovery()) << "Received ARP reply from" << address.toString() << macAddress << "but there is no discovery running.";
        return;
    }

    qCDebug(dcNetworkDeviceDiscovery()) << "ARP reply received" << address.toString() << macAddress << interface.name();
    // Lookup the mac address vendor if possible
    if (m_macAddressDatabase->available()) {
        MacAddressDatabaseReply *reply = m_macAddressDatabase->lookupMacAddress(macAddress);
        connect(reply, &MacAddressDatabaseReply::finished, m_currentReply, [=](){
            qCDebug(dcNetworkDeviceDiscovery()) << "MAC manufacturer lookup finished for" << macAddress << ":" << reply->manufacturer();
            updateOrAddNetworkDeviceArp(interface, address, macAddress, reply->manufacturer());
        });
    } else {
        updateOrAddNetworkDeviceArp(interface, address, macAddress);
    }
}
