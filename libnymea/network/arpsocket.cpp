// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "arpsocket.h"
#include "loggingcategories.h"
#include "networkutils.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>

#include <QHostInfo>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDataStream>

NYMEA_LOGGING_CATEGORY(dcArpSocket, "ArpSocket")
NYMEA_LOGGING_CATEGORY(dcArpSocketTraffic, "ArpSocketTraffic")

#define ETHER_PROTOCOL_LEN 4 // Length of the IPv4 address
#define ETHER_HEADER_LEN sizeof(struct ether_header)
#define ETHER_ARP_LEN sizeof(struct ether_arp)
#define ETHER_ARP_PACKET_LEN ETHER_HEADER_LEN + ETHER_ARP_LEN

ArpSocket::ArpSocket(QObject *parent) : QObject(parent)
{

}

bool ArpSocket::sendRequest()
{
    if (!m_isOpen)
        return false;

    // Send the ARP request through each network interface
    qCDebug(dcArpSocket()) << "Sending ARP request to all local network interfaces...";
    foreach (const QNetworkInterface &networkInterface, QNetworkInterface::allInterfaces()) {
        sendRequest(networkInterface);
    }

    return true;
}

bool ArpSocket::sendRequest(const QString &interfaceName)
{
    if (!m_isOpen)
        return false;

    // Get the interface
    qCDebug(dcArpSocket()) << "Sending ARP request to network interface" << interfaceName << "...";
    QNetworkInterface networkInterface = QNetworkInterface::interfaceFromName(interfaceName);
    if (!networkInterface.isValid()) {
        qCWarning(dcArpSocket()) << "Failed to send the ARP request to network interface" << interfaceName << "because the interface is not valid.";
        return false;
    }

    return sendRequest(networkInterface);
}

bool ArpSocket::sendRequest(const QNetworkInterface &networkInterface)
{
    if (!m_isOpen)
        return false;

    // Skip local host
    if (networkInterface.flags().testFlag(QNetworkInterface::IsLoopBack))
        return false;

    // If the interface index is unknown, we cannot use this network
    if (networkInterface.index() == 0) {
        qCDebug(dcArpSocket()) << "Failed to send the ARP request to network interface" << networkInterface.name() << "because the system interface index is unknown.";
        return false;
    }

    // Check if the interface is up and running
    if (!networkInterface.flags().testFlag(QNetworkInterface::IsUp)) {
        qCDebug(dcArpSocket()) << "Failed to send the ARP request to network interface" << networkInterface.name() << "because it is not up.";
        return false;
    }

    if (!networkInterface.flags().testFlag(QNetworkInterface::IsRunning)) {
        qCDebug(dcArpSocket()) << "Failed to send the ARP request to network interface" << networkInterface.name() << "because it is not running.";
        return false;
    }

    // Verify we have a hardware address (virtual network interfaces like tunnels)
    if (networkInterface.hardwareAddress().isEmpty()) {
        qCDebug(dcArpSocket()) << "Failed to send the ARP request to network interface" << networkInterface.name() << "because there is no hardware address which is required for ARP.";
        return false;
    }

    qCDebug(dcArpSocket()) << "Verifying network interface" << networkInterface.name() << networkInterface.hardwareAddress() << "...";
    foreach (const QNetworkAddressEntry &entry, networkInterface.addressEntries()) {
        // Only IPv4
        if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
            continue;

        qCDebug(dcArpSocket()) << "    Host address:" << entry.ip().toString();
        qCDebug(dcArpSocket()) << "    Broadcast address:" << entry.broadcast().toString();
        qCDebug(dcArpSocket()) << "    Netmask:" << entry.netmask().toString();
        quint32 addressRangeStart = entry.ip().toIPv4Address() & entry.netmask().toIPv4Address();
        quint32 addressRangeStop = entry.broadcast().toIPv4Address() | addressRangeStart;
        quint32 range = addressRangeStop - addressRangeStart;
        qCDebug(dcArpSocket()) << "    Address range" << range << " | from" << QHostAddress(addressRangeStart).toString() << "-->" << QHostAddress(addressRangeStop).toString();
        if (range > 255) {
            qCWarning(dcArpSocket()) << "Not sending ARP requests to the network" << networkInterface.name() << "because it has a to wide range for ARP broadcast pinging.";
            return false;
        }

        qCDebug(dcArpSocket()) << "Start sending ARP requests to each host within the range...";

        // Send ARP request to each address within the range
        for (quint32 i = 0; i < range; i++) {
            quint32 address = addressRangeStart + i;
            QHostAddress targetAddress(address);
            if (targetAddress == entry.ip())
                continue;

            sendRequestInternally(networkInterface.index(), MacAddress(networkInterface.hardwareAddress()), entry.ip(), MacAddress::broadcast(), targetAddress);
        }
    }

    return true;
}

bool ArpSocket::sendRequest(const QHostAddress &targetAddress)
{
    if (!m_isOpen)
        return false;

    if (targetAddress.protocol() != QAbstractSocket::IPv4Protocol) {
        qCWarning(dcArpSocket()) << "Not sending ARP request to host" << targetAddress << "because only IPv4 is supported.";
        return false;
    }

    qCDebug(dcArpSocket()) << "Sending ARP request to host" << targetAddress.toString() << "...";
    foreach (const QNetworkInterface &networkInterface, QNetworkInterface::allInterfaces()) {
        foreach (const QNetworkAddressEntry &entry, networkInterface.addressEntries()) {
            // Only IPv4
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            qCDebug(dcArpSocket()) << "Check subnet for" << networkInterface.name() << entry.ip() << entry.netmask();
            if (targetAddress.isInSubnet(entry.ip(), entry.prefixLength())) {
                return sendRequestInternally(networkInterface.index(), MacAddress(networkInterface.hardwareAddress()), entry.ip(), MacAddress::broadcast(), targetAddress);
            } else {
                qCDebug(dcArpSocket()) << targetAddress << "is not part of subnet" << entry.ip() << "netmask" << entry.netmask() << "netmask int" << entry.netmask().toIPv4Address();
            }
        }
    }

    qCWarning(dcArpSocket()) << "Failed to send ARP request to" << targetAddress.toString() << "because no valid network interface could be found.";
    return false;
}

bool ArpSocket::isOpen() const
{
    return m_isOpen;
}

bool ArpSocket::openSocket()
{
    qCDebug(dcArpSocket()) << "Open ARP socket...";

    if (m_isOpen) {
        qCWarning(dcArpSocket()) << "Failed to enable ARP scanner because the scanner is already running.";
        return false;
    }

    // Build socket descriptor
    m_socketDescriptor = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (m_socketDescriptor < 0) {
        qCWarning(dcArpSocket()) << "Failed to create the ARP capture socket for" << "." << strerror(errno);
        return false;
    }

    // Configure non blocking
    if (fcntl(m_socketDescriptor, F_SETFL, fcntl(m_socketDescriptor, F_GETFL, 0) | O_NONBLOCK) != 0) {
        qCWarning(dcArpSocket()) << "Failed to set the ARP socket function control to non-blocking" << strerror(errno);
        close(m_socketDescriptor);
        return false;
    }

    m_socketNotifier = new QSocketNotifier(m_socketDescriptor, QSocketNotifier::Read, this);
    m_socketNotifier->setEnabled(false);
    connect(m_socketNotifier, &QSocketNotifier::activated, this, [=](int socket){
        if (socket != m_socketDescriptor)
            return;

        // Make sure to read all data from the socket...
        int bytesReceived = 0;
        while (bytesReceived >= 0) {
            unsigned char receiveBuffer[ETHER_ARP_PACKET_LEN];
            memset(&receiveBuffer, 0, sizeof(receiveBuffer));

            // Read the buffer
            bytesReceived = recv(m_socketDescriptor, receiveBuffer, ETHER_ARP_PACKET_LEN, 0);
            if (bytesReceived < 0)
                continue;

            processDataBuffer(receiveBuffer, bytesReceived);
        }
    });

    m_socketNotifier->setEnabled(true);
    m_isOpen = true;
    qCDebug(dcArpSocket()) << "ARP enabled successfully";

    // Send broadcast request
    //sendRequest();
    return true;
}

void ArpSocket::closeSocket()
{
    m_isOpen = false;

    if (m_socketNotifier) {
        m_socketNotifier->setEnabled(false);
        delete m_socketNotifier;
        m_socketNotifier = nullptr;
    }

    if (m_socketDescriptor >= 0) {
        close(m_socketDescriptor);
        m_socketDescriptor = -1;
    }

    qCDebug(dcArpSocket()) << "ARP disabled successfully";
}

bool ArpSocket::sendRequestInternally(int networkInterfaceIndex, const MacAddress &senderMacAddress, const QHostAddress &senderHostAddress, const MacAddress &targetMacAddress, const QHostAddress &targetHostAddress)
{
    // Set up data structures
    unsigned char sendingBuffer[ETHER_ARP_PACKET_LEN];
    memset(sendingBuffer, 0, ETHER_ARP_PACKET_LEN);
    struct ether_header *etherHeader = (struct ether_header *)sendingBuffer;
    struct ether_arp *arpPacket = (struct ether_arp *)(sendingBuffer + sizeof(struct ether_header));

    // Build the ethernet header
    fillMacAddress(etherHeader->ether_dhost, targetMacAddress);
    fillMacAddress(etherHeader->ether_shost, senderMacAddress);
    etherHeader->ether_type = htons(ETHERTYPE_ARP);

    // Build the ARP header
    arpPacket->ea_hdr.ar_hrd = htons(ARPHRD_ETHER);
    arpPacket->ea_hdr.ar_pro = htons(ETH_P_IP);
    arpPacket->ea_hdr.ar_hln = ETHER_ADDR_LEN;
    arpPacket->ea_hdr.ar_pln = ETHER_PROTOCOL_LEN;
    arpPacket->ea_hdr.ar_op = htons(ARPOP_REQUEST);

    // Write the ARP packet
    fillMacAddress(arpPacket->arp_sha, senderMacAddress);
    fillHostAddress(arpPacket->arp_spa, senderHostAddress);
    fillMacAddress(arpPacket->arp_tha, targetMacAddress);
    fillHostAddress(arpPacket->arp_tpa, targetHostAddress);

    struct sockaddr_ll socketAddress;
    memset(&socketAddress, 0, sizeof(socketAddress));
    socketAddress.sll_family = AF_PACKET;
    socketAddress.sll_protocol = htons(ETH_P_ARP);
    socketAddress.sll_ifindex = networkInterfaceIndex;
    socketAddress.sll_hatype =  htons(ARPHRD_ETHER);
    socketAddress.sll_pkttype = PACKET_BROADCAST;
    socketAddress.sll_halen = ETH_ALEN;
    memset(socketAddress.sll_addr, 0x00, 6);

    //qCDebug(dcArpSocket()) << "Send ARP request to" << targetHostAddress.toString();
    int bytesSent = sendto(m_socketDescriptor, sendingBuffer, ETHER_ARP_PACKET_LEN, 0, (struct sockaddr *)&socketAddress, sizeof(socketAddress));
    if (bytesSent < 0) {
        qCWarning(dcArpSocket()) << "Failed to send ARP packet data to" << targetHostAddress.toString() << strerror(errno);
        return false;
    }

    return true;
}

void ArpSocket::processDataBuffer(unsigned char *receiveBuffer, int size)
{
    // Parse data using structs header + arp
    QByteArray receivedBufferBytes;
    for (int i = 0; i < size; i++) {
        receivedBufferBytes.append(receiveBuffer[i]);
    }

    struct ether_header *etherHeader = (struct ether_header *)(receiveBuffer);
    struct ether_arp *arpPacket = (struct ether_arp *)(receiveBuffer + ETHER_HEADER_LEN);
    MacAddress ethernetSourceMacAddress = MacAddress(etherHeader->ether_shost);
    MacAddress ethernetDestinationMacAddress = MacAddress(etherHeader->ether_dhost);

    MacAddress senderMacAddress = MacAddress(arpPacket->arp_sha);
    QHostAddress senderHostAddress = getHostAddressString(arpPacket->arp_spa);
    MacAddress targetMacAddress = MacAddress(arpPacket->arp_tha);
    QHostAddress targetHostAddress = getHostAddressString(arpPacket->arp_tpa);

    uint16_t etherType = htons(etherHeader->ether_type);
    if (etherType != ETHERTYPE_ARP) {
        qCWarning(dcArpSocketTraffic()) << "Received ARP socket data header with invalid type" << etherType;
        return;
    }

    // Filter for ARP replies
    uint16_t arpOperationCode = htons(arpPacket->arp_op);
    switch (arpOperationCode) {
    case ARPOP_REQUEST: {
        // The sender of the arp request provides ip and mac.
        // Lets find the corresponding interface and use it for the discovery and monitor
        if (senderHostAddress.isNull())
            return;

        QNetworkInterface networkInterface = NetworkUtils::getInterfaceForHostaddress(senderHostAddress);
        if (!networkInterface.isValid()) {
            qCDebug(dcArpSocket()) << "Could not find local interface from ARP request" << senderHostAddress.toString() << senderMacAddress.toString();
            return;
        }

        // Note: we are not interested in our own requests
        if (senderMacAddress != MacAddress(networkInterface.hardwareAddress()))
            return;

        /* Use only replies from hosts which are requesting for them self. */
        if (senderMacAddress == ethernetSourceMacAddress) {
            qCDebug(dcArpSocket()) << "ARP request" << receivedBufferBytes.toHex() << "from" << senderMacAddress.toString() << senderHostAddress.toString() << "-->" << targetMacAddress.toString() << targetHostAddress.toString() << "on" << networkInterface.name();
            emit arpRequestReceived(networkInterface, senderHostAddress, senderMacAddress);
        }

        break;
    }
    case ARPOP_REPLY: {
        QNetworkInterface networkInterface = NetworkUtils::getInterfaceForMacAddress(targetMacAddress);
        if (!networkInterface.isValid()) {
            qCDebug(dcArpSocket()) << "Could not find local interface from ARP response" << targetHostAddress.toString() << targetMacAddress.toString();
            return;
        }

        /* Use only replies from hosts which are responding for them self. In some cases where we have 2 network interfaces connected to the
         * same network that one interface is reposning for the other and we end up with 2 ip addresses for one mac address. */
        if (senderMacAddress == ethernetSourceMacAddress) {
            qCDebug(dcArpSocket()) << "ARP reply from" << ethernetSourceMacAddress.toString() << receivedBufferBytes.toHex() << "ARP: sender" << senderMacAddress.toString() << senderHostAddress.toString() << "-->" << targetMacAddress.toString() << targetHostAddress.toString() << "on" << networkInterface.name();
            emit arpResponse(networkInterface, senderHostAddress, senderMacAddress);
        } else {
            qCDebug(dcArpSocket()) << "ARP proxy reply from" << ethernetSourceMacAddress.toString() << receivedBufferBytes.toHex() << "ARP: sender" << senderMacAddress.toString() << senderHostAddress.toString() << "-->" << targetMacAddress.toString() << targetHostAddress.toString() << "on" << networkInterface.name();
        }
        break;
    }
    case ARPOP_RREQUEST:
        qCDebug(dcArpSocketTraffic()) << "RARP request from" << ethernetSourceMacAddress.toString() << receivedBufferBytes.toHex() << "ARP: sender" << senderMacAddress.toString() << senderHostAddress.toString() << "-->" << targetMacAddress.toString() << targetHostAddress.toString();
        break;
    case ARPOP_RREPLY:
        qCDebug(dcArpSocketTraffic()) << "PARP response from" << ethernetSourceMacAddress.toString() << receivedBufferBytes.toHex() << "ARP: sender" << senderMacAddress.toString() << senderHostAddress.toString() << "-->" << targetMacAddress.toString() << targetHostAddress.toString();
        break;
    case ARPOP_InREQUEST:
        qCDebug(dcArpSocketTraffic()) << "InARP request from" << ethernetSourceMacAddress.toString() << receivedBufferBytes.toHex() << "ARP: sender" << senderMacAddress.toString() << senderHostAddress.toString() << "-->" << targetMacAddress.toString() << targetHostAddress.toString();
        break;
    case ARPOP_InREPLY:
        qCDebug(dcArpSocketTraffic()) << "InARP response from" << ethernetSourceMacAddress.toString() << receivedBufferBytes.toHex() << "ARP: sender" << senderMacAddress.toString() << senderHostAddress.toString() << "-->" << targetMacAddress.toString() << targetHostAddress.toString();
        break;
    case ARPOP_NAK:
        qCDebug(dcArpSocketTraffic()) << "(ATM)ARP NAK from" << ethernetSourceMacAddress.toString() << receivedBufferBytes.toHex() << "ARP: sender" << senderMacAddress.toString() << senderHostAddress.toString() << "-->" << targetMacAddress.toString() << targetHostAddress.toString();
        break;
    default:
        qCWarning(dcArpSocketTraffic()) << "Received unhandled ARP operation code" << arpOperationCode << "from" << ethernetSourceMacAddress.toString() << receivedBufferBytes.toHex() << "ARP: sender" << senderMacAddress.toString() << senderHostAddress.toString();
        break;
    }
}

QHostAddress ArpSocket::getHostAddressString(uint8_t *senderIpAddress)
{
    QStringList values;
    for (int i = 0; i < ETHER_PROTOCOL_LEN; i++) {
        values.append(QString("%1").arg(senderIpAddress[i]));
    }

    return QHostAddress(values.join("."));
}

bool ArpSocket::loadArpCache(const QNetworkInterface &interface)
{
    QFile arpFile("/proc/net/arp");
    qCDebug(dcArpSocket()) << "Loading ARP cache from system" << arpFile.fileName() << "...";
    if (!arpFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(dcArpSocket()) << "Failed to load ARP cache from" << arpFile.fileName() << arpFile.errorString();
        return false;
    }

    // Read all data
    QByteArray data = arpFile.readAll();
    arpFile.close();

    // Parse data line by line
    int lineCount = -1;
    QTextStream stream(&data);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        lineCount += 1;

        // Skip the first line since it's just the header
        if (lineCount == 0)
            continue;

        qCDebug(dcArpSocket()) << "Checking line" << line;
        QStringList columns = line.split(QLatin1Char(' '));
        columns.removeAll("");

        // Make sure we have enought token
        if (columns.count() < 6) {
            qCWarning(dcArpSocket()) << "ARP cache line has invalid column count" << line;
            continue;
        }

        QHostAddress address(columns.at(0).trimmed());
        if (address.isNull()) {
            qCWarning(dcArpSocket()) << "ARP cache line has invalid IP address";
            continue;
        }

        QString macAddressString = columns.at(3).trimmed();
        MacAddress macAddress(macAddressString);
        if (macAddress.isNull()) {
            qCDebug(dcArpSocket()) << "ARP cache line has invalid MAC address" << macAddressString;
            continue;
        }

        QNetworkInterface addressInterface = QNetworkInterface::interfaceFromName(columns.at(5));
        if (!addressInterface.isValid()) {
            qCDebug(dcArpSocket()) << "ARP cache line has invalid network interface identifier" << columns << columns.at(5);
            continue;
        }

        // Check if we filter for specific interfaces
        if (interface.isValid() && addressInterface.name() != interface.name())
            continue;

        qCDebug(dcArpSocket()) << "Loaded from cache" << address.toString() << macAddress.toString() << addressInterface.name();
        emit arpResponse(addressInterface, address, macAddress);
    }

    return true;
}

void ArpSocket::fillMacAddress(uint8_t *targetArray, const MacAddress &macAddress)
{
    QStringList macValues = macAddress.toString().split(":");
    for (int i = 0; i < ETHER_ADDR_LEN; i++) {
        targetArray[i] = macValues.at(i).toUInt(nullptr, 16);
    }
}

void ArpSocket::fillHostAddress(uint8_t *targetArray, const QHostAddress &hostAddress)
{
    QByteArray hostData;
    QDataStream stream(&hostData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << hostAddress.toIPv4Address();
    for (int i = 0; i < ETHER_PROTOCOL_LEN; i++) {
        targetArray[i] = hostData.at(i);
    }
}

