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
#include <QDataStream>

NYMEA_LOGGING_CATEGORY(dcArpSocket, "ArpSocket")

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

    // Send the ARP request trough each network interface
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
    qCDebug(dcArpSocket()) << "Sending ARP request to all network interfaces" << interfaceName << "...";
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

    // If have no interface indes, we cannot use this network
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

            sendRequestInternally(networkInterface.index(), networkInterface.hardwareAddress(), entry.ip(), "ff:ff:ff:ff:ff:ff", targetAddress);
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

            if (targetAddress.isInSubnet(entry.ip(), entry.netmask().toIPv4Address())) {
                return sendRequestInternally(networkInterface.index(), networkInterface.hardwareAddress(), entry.ip(), "ff:ff:ff:ff:ff:ff", targetAddress);
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
        while (true) {
            char receiveBuffer[ETHER_ARP_PACKET_LEN];
            memset(&receiveBuffer, 0, sizeof(receiveBuffer));

            // Read the buffer
            int bytesReceived = recv(m_socketDescriptor, receiveBuffer, ETHER_ARP_PACKET_LEN, 0);
            if (bytesReceived < 0) {
                // Finished reading
                return;
            }

            // Parse data using structs header + arp
            struct ether_header *etherHeader = (struct ether_header *)(receiveBuffer);
            struct ether_arp *arpPacket = (struct ether_arp *)(receiveBuffer + ETHER_HEADER_LEN);
            QString senderMacAddress = getMacAddressString(arpPacket->arp_sha);
            QHostAddress senderHostAddress = getHostAddressString(arpPacket->arp_spa);
            QString targetMacAddress = getMacAddressString(arpPacket->arp_tha);
            QHostAddress targetHostAddress = getHostAddressString(arpPacket->arp_tpa);
            uint16_t etherType = htons(etherHeader->ether_type);
            if (etherType != ETHERTYPE_ARP) {
                qCWarning(dcArpSocket()) << "Received ARP socket data header with invalid type" << etherType;
                return;
            }

            // Filter for ARP replies
            uint16_t arpOperationCode = htons(arpPacket->arp_op);
            switch (arpOperationCode) {
            case ARPOP_REQUEST:
                //qCDebug(dcArpSocket()) << "ARP request from " << senderMacAddress << senderHostAddress.toString() << "-->" << targetMacAddress << targetHostAddress.toString();
                break;
            case ARPOP_REPLY: {
                QNetworkInterface networkInterface = NetworkUtils::getInterfaceForMacAddress(targetMacAddress);
                if (!networkInterface.isValid()) {
                    qCWarning(dcArpSocket()) << "Could not find interface from ARP response" << targetHostAddress.toString() << targetMacAddress;
                    return;
                }

                qCDebug(dcArpSocket()) << "ARP response from" << senderMacAddress << senderHostAddress.toString() << "on" << networkInterface.name();
                emit arpResponse(networkInterface, senderHostAddress, senderMacAddress.toLower());
                break;
            }
            case ARPOP_RREQUEST:
                //qCDebug(dcArpSocket()) << "RARP request from" << senderMacAddress << senderHostAddress.toString() << "-->" << targetMacAddress << targetHostAddress.toString();
                break;
            case ARPOP_RREPLY:
                //qCDebug(dcArpSocket()) << "PARP response from" << senderMacAddress << senderHostAddress.toString() << "-->" << targetMacAddress << targetHostAddress.toString();
                break;
            case ARPOP_InREQUEST:
                //qCDebug(dcArpSocket()) << "InARP request from" << senderMacAddress << senderHostAddress.toString() << "-->" << targetMacAddress << targetHostAddress.toString();
                break;
            case ARPOP_InREPLY:
                //qCDebug(dcArpSocket()) << "InARP response from" << senderMacAddress << senderHostAddress.toString() << "-->" << targetMacAddress << targetHostAddress.toString();
                break;
            case ARPOP_NAK:
                //qCDebug(dcArpSocket()) << "(ATM)ARP NAK from" << senderMacAddress << senderHostAddress.toString() << "-->" << targetMacAddress << targetHostAddress.toString();
                break;
            default:
                qCWarning(dcArpSocket()) << "Received unhandled ARP operation code" << arpOperationCode << "from" << senderMacAddress << senderHostAddress.toString();
                break;
            }
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

bool ArpSocket::sendRequestInternally(int networkInterfaceIndex, const QString &senderMacAddress, const QHostAddress &senderHostAddress, const QString &targetMacAddress, const QHostAddress &targetHostAddress)
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

QString ArpSocket::getMacAddressString(uint8_t *senderHardwareAddress)
{
    QStringList hexValues;
    for (int i = 0; i < ETHER_ADDR_LEN; i++) {
        hexValues.append(QString("%1").arg(senderHardwareAddress[i], 2, 16, QLatin1Char('0')));
    }

    return hexValues.join(":");
}

QHostAddress ArpSocket::getHostAddressString(uint8_t *senderIpAddress)
{
    QStringList values;
    for (int i = 0; i < ETHER_PROTOCOL_LEN; i++) {
        values.append(QString("%1").arg(senderIpAddress[i]));
    }

    return QHostAddress(values.join("."));
}

void ArpSocket::fillMacAddress(uint8_t *targetArray, const QString &macAddress)
{
    QStringList macValues = macAddress.split(":");
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

