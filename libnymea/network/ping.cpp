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

#include "ping.h"
#include "networkutils.h"
#include "loggingcategories.h"

#include <fcntl.h>
#include <sys/types.h>
#include <resolv.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include <QtEndian>

NYMEA_LOGGING_CATEGORY(dcPing, "Ping")
NYMEA_LOGGING_CATEGORY(dcPingTraffic, "PingTraffic")

Ping::Ping(QObject *parent) : QObject(parent)
{
    m_queueTimer = new QTimer(this);
    m_queueTimer->setInterval(20);
    m_queueTimer->setSingleShot(true);
    connect(m_queueTimer, &QTimer::timeout, this, [=](){
        sendNextReply();
    });

    // Build socket descriptor
    m_socketDescriptor = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (m_socketDescriptor < 0) {
        qCWarning(dcPing()) << "Failed to create the ICMP socket." << strerror(errno);
        verifyErrno(errno);
        return;
    }

    // Set time to live value
    const int val = ICMP_TTL_VALUE;
    if (setsockopt(m_socketDescriptor, SOL_IP, IP_TTL, &val, sizeof(val)) != 0) {
        verifyErrno(errno);
        qCWarning(dcPing()) << "Failed to set the ICMP socket TTL option:" << strerror(errno);
        cleanUpSocket();
        return;
    }

    // Configure non blocking
    if (fcntl(m_socketDescriptor, F_SETFL, fcntl(m_socketDescriptor, F_GETFL, 0) | O_NONBLOCK) != 0) {
        verifyErrno(errno);
        qCWarning(dcPing()) << "Failed to set the ICMP socket function control to non-blocking" << strerror(errno);
        cleanUpSocket();
        return;
    }

    // Create the socket notifier for read notification
    m_socketNotifier = new QSocketNotifier(m_socketDescriptor, QSocketNotifier::Read, this);
    connect(m_socketNotifier, &QSocketNotifier::activated, this, &Ping::onSocketReadyRead);

    m_socketNotifier->setEnabled(true);
    m_available = true;
    qCDebug(dcPing()) << "ICMP socket set up successfully (Socket ID:" << m_socketDescriptor << ")";
}

QByteArray Ping::payload() const
{
    return m_payload;
}

void Ping::setPayload(const QByteArray &payload)
{
    Q_ASSERT_X(static_cast<uint>(payload.count()) <= ICMP_PAYLOAD_SIZE, "ping", QString("maximal payload size is %1").arg(ICMP_PAYLOAD_SIZE).toLocal8Bit());
    m_payload = payload;
}

bool Ping::available() const
{
    return m_available;
}

PingReply::Error Ping::error() const
{
    return m_error;
}

PingReply *Ping::ping(const QHostAddress &hostAddress, uint retries)
{
    PingReply *reply = createReply(hostAddress);
    reply->m_retries = retries;

    // Perform the reply in the next event loop to give the user time to do the reply connects
    m_replyQueue.enqueue(reply);
    sendNextReply();

    return reply;
}

PingReply *Ping::ping(const QHostAddress &hostAddress, bool lookupHost, uint retries)
{
    PingReply *reply = createReply(hostAddress);
    reply->m_retries = retries;
    reply->m_doHostLookup = lookupHost;

    // Perform the reply in the next event loop to give the user time to do the reply connects
    m_replyQueue.enqueue(reply);
    sendNextReply();

    return reply;
}

void Ping::sendNextReply()
{
    if (m_queueTimer->isActive())
        return;

    if (m_replyQueue.isEmpty())
        return;

    PingReply *reply = m_replyQueue.dequeue();
    qCDebug(dcPing()) << "Send next reply," << m_replyQueue.count() << "left in queue";
    m_queueTimer->start();
    QTimer::singleShot(0, reply, [=]() { performPing(reply); });
}

void Ping::performPing(PingReply *reply)
{
    if (!m_available) {
        qCDebug(dcPing()) << "Cannot send ping request" << m_error;
        finishReply(reply, m_error);
        return;
    }

    if (reply->targetHostAddress().isNull()) {
        m_error = PingReply::ErrorInvalidHostAddress;
        qCWarning(dcPing()) << "Cannot send ping request" << m_error;
        finishReply(reply, m_error);
        return;
    }

    // Get host ip address
    struct hostent *hostname = gethostbyname(reply->targetHostAddress().toString().toLocal8Bit().constData());
    struct sockaddr_in pingAddress;
    memset(&pingAddress, 0, sizeof(pingAddress));
    pingAddress.sin_family = hostname->h_addrtype;
    pingAddress.sin_port = 0;
    pingAddress.sin_addr.s_addr = *(long*)hostname->h_addr;

    QHostAddress targetHostAddress = QHostAddress(qFromBigEndian(pingAddress.sin_addr.s_addr));

    // Build the ICMP echo request packet
    struct icmpPacket requestPacket;
    memset(&requestPacket, 0, sizeof(requestPacket));
    requestPacket.icmpHeadr.type = ICMP_ECHO;
    if (reply->requestId() == 0) {
        reply->m_requestId = calculateRequestId();
    }
    requestPacket.icmpHeadr.un.echo.id = htons(reply->requestId());
    requestPacket.icmpHeadr.un.echo.sequence = htons(reply->sequenceNumber());

    // Write the ICMP payload
    memset(&requestPacket.icmpPayload, ' ', sizeof(requestPacket.icmpPayload));
    for (int i = 0; i < m_payload.count(); i++)
        requestPacket.icmpPayload[i] = m_payload.at(i);

    // Calculate the ICMP packet checksum
    requestPacket.icmpHeadr.checksum = calculateChecksum(reinterpret_cast<unsigned short *>(&requestPacket), sizeof(requestPacket));

    // Get time for ping measurement and fill reply information
    if (gettimeofday(&reply->m_startTime, nullptr) < 0 ) {
        qCWarning(dcPing()) << "Failed to get start time for ping measurement" << strerror(errno);
    }

    qCDebug(dcPingTraffic()) << "Send ICMP echo request" << reply->targetHostAddress().toString() << ICMP_PACKET_SIZE << "[Bytes]"
                      << "ID:" << QString("0x%1").arg(reply->requestId(), 4, 16, QChar('0'))
                      << "Sequence:" << reply->sequenceNumber();

    // Send packet to the target ip
    int bytesSent = sendto(m_socketDescriptor, &requestPacket, sizeof(requestPacket), 0, (struct sockaddr *)&pingAddress, sizeof(pingAddress));
    if (bytesSent < 0) {
        verifyErrno(errno);
        qCWarning(dcPing()) << "Failed to send data to" << reply->targetHostAddress().toString() << strerror(errno);
        finishReply(reply, m_error);
        return;
    }

    // Start reply timer and handle timeout
    m_pendingReplies.insert(reply->requestId(), reply);
    reply->m_timer->start(m_timeoutDuration);
}

void Ping::verifyErrno(int error)
{
    switch (error) {
    case ENETDOWN:
        m_error = PingReply::ErrorNetworkDown;
        break;
    case ENETUNREACH:
        m_error = PingReply::ErrorNetworkUnreachable;
        break;
    case EACCES:
    case EPERM:
        m_error = PingReply::ErrorPermissionDenied;
        break;
    default:
        m_error = PingReply::ErrorSocketError;
    }
}

unsigned short Ping::calculateChecksum(unsigned short *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;

    if (len == 1)
        sum += *(unsigned char*)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void Ping::cleanUpSocket()
{
    m_available = false;

    if (m_socketNotifier) {
        m_socketNotifier->setEnabled(false);
        delete  m_socketNotifier;
        m_socketNotifier = nullptr;
    }

    if (m_socketDescriptor >= 0) {
        close(m_socketDescriptor);
        m_socketDescriptor = -1;
    }
}

void Ping::timeValueSubtract(timeval *start, timeval *stop)
{
    int sec = start->tv_sec - stop->tv_sec;
    int usec = start->tv_usec - stop->tv_usec;
    if (usec < 0) {
        start->tv_sec = sec - 1;
        start->tv_usec = 1000000 + usec;
    } else {
        start->tv_sec = sec;
        start->tv_usec = usec;
    }
}

quint16 Ping::calculateRequestId()
{
    quint16 requestId = 0;
    while (requestId == 0 || m_pendingReplies.contains(requestId)) {
        requestId = rand();
    }

    return requestId;
}

PingReply *Ping::createReply(const QHostAddress &hostAddress)
{
    PingReply *reply = new PingReply(this);
    reply->m_targetHostAddress = hostAddress;
    reply->m_networkInterface = NetworkUtils::getInterfaceForHostaddress(hostAddress);

    connect(reply, &PingReply::timeout, this, [=](){
        // Note: this is not the ICMP timeout, here we actually got nothing from nobody...
        finishReply(reply, PingReply::ErrorTimeout);
    });

    connect(reply, &PingReply::aborted, this, [=](){
        finishReply(reply, PingReply::ErrorAborted);
    });

    return reply;
}

void Ping::finishReply(PingReply *reply, PingReply::Error error)
{
    // Check if we should retry
    if (reply->m_retryCount >= reply->m_retries ||
            error == PingReply::ErrorNoError ||
            error == PingReply::ErrorAborted ||
            error == PingReply::ErrorInvalidHostAddress ||
            error == PingReply::ErrorPermissionDenied) {
        // No retry, we are done
        reply->m_error = error;
        reply->m_timer->stop();
        m_pendingReplies.remove(reply->requestId());
        emit reply->finished();
        reply->deleteLater();
    } else {
        m_pendingReplies.remove(reply->requestId());
        reply->m_error = error;
        reply->m_retryCount++;
        reply->m_sequenceNumber++;

        qCDebug(dcPing()) << "Ping finished with error" << error << "Retry ping" << reply->targetHostAddress().toString() << reply->m_retryCount << "/" << reply->m_retries;
        emit reply->retry(error, reply->retryCount());

        // Note: will be restarted once actually sent trough the network
        reply->m_timer->stop();

        // Re-Enqueu the reply
        m_replyQueue.enqueue(reply);
        sendNextReply();
    }
}

void Ping::onSocketReadyRead(int socketDescriptor)
{
    // We must read all data otherwise the socket notifier does not work as expected
    while (true) {
        // Read the socket data and give some extra space for nested pakets...
        int receiveBufferSize = 2 * ICMP_PACKET_SIZE + sizeof(struct iphdr);
        char receiveBuffer[receiveBufferSize];
        memset(&receiveBuffer, 0, sizeof(receiveBufferSize));

        int bytesReceived = recv(socketDescriptor, &receiveBuffer, receiveBufferSize, 0);
        if (bytesReceived < 0) {
            return;
        }

        qCDebug(dcPingTraffic()) << "Received" << bytesReceived << "bytes" << "( Socket ID:" << m_socketDescriptor << ")";
        struct iphdr *ipHeader = (struct iphdr *)receiveBuffer;
        int ipHeaderLength = ipHeader->ihl << 2;
        int icmpPacketSize = htons(ipHeader->tot_len) - ipHeaderLength;
        QHostAddress senderAddress(qFromBigEndian(ipHeader->saddr));
        QHostAddress destinationAddress(qFromBigEndian(ipHeader->daddr));

        qCDebug(dcPingTraffic()) << "IP header: Lenght" << ipHeaderLength
                          << "Sender:" << senderAddress.toString()
                          << "Destination:" << destinationAddress.toString()
                          << "Size:" << htons(ipHeader->tot_len) << "B"
                          << "TTL" << ipHeader->ttl;

        struct icmp *responsePacket = reinterpret_cast<struct icmp *>(receiveBuffer + ipHeaderLength);
        quint16 icmpId = htons(responsePacket->icmp_id);
        quint16 icmpSequnceNumber = htons(responsePacket->icmp_seq);
        qCDebug(dcPingTraffic()) << "ICMP packt (Size:" << icmpPacketSize << "Bytes):"
                          << "Type" << responsePacket->icmp_type
                          << "Code:" << responsePacket->icmp_code
                          << "ID:" << QString("0x%1").arg(icmpId, 4, 16, QChar('0'))
                          << "Sequence:" << icmpSequnceNumber;

        if (responsePacket->icmp_type == ICMP_ECHOREPLY) {

            PingReply *reply = m_pendingReplies.take(icmpId);
            if (!reply) {
                qCDebug(dcPing()) << "No pending reply for ping echo response with id" << QString("0x%1").arg(icmpId, 4, 16, QChar('0')) << "Sequence:" << icmpSequnceNumber << "from" << senderAddress.toString();
                return;
            }

            // Make sure the sender matches the target
            if (reply->targetHostAddress() != senderAddress) {
                qCWarning(dcPing()) << "Received id for different target reply" << reply->targetHostAddress().toString() << "!=" << senderAddress.toString();
                finishReply(reply, PingReply::ErrorHostUnreachable);
                return;
            }

            // Verify sequence number
            if (icmpSequnceNumber != reply->sequenceNumber()) {
                qCWarning(dcPing()) << "Received echo reply with different sequence number" << icmpSequnceNumber;
                finishReply(reply, PingReply::ErrorInvalidResponse);
                return;
            }

            // Calculate ping duration 2 digits accuracy
            struct timeval receiveTimeValue;
            gettimeofday(&receiveTimeValue, nullptr);
            timeValueSubtract(&receiveTimeValue, &reply->m_startTime);
            reply->m_duration = qRound((receiveTimeValue.tv_sec * 1000 + (double)receiveTimeValue.tv_usec / 1000) * 100) / 100.0;

            qCDebug(dcPing()) << "Received ICMP response" << reply->targetHostAddress().toString() << ICMP_PACKET_SIZE << "[Bytes]"
                          << "ID:" << QString("0x%1").arg(icmpId, 4, 16, QChar('0'))
                          << "Sequence:" << icmpSequnceNumber
                          << "Time:" << reply->duration() << "[ms]";

            if (reply->doHostLookup()) {
                // Note: due to a Qt bug < 5.9 we need to use old SLOT style and cannot make use of lambda here
                int lookupId = QHostInfo::lookupHost(senderAddress.toString(), this, SLOT(onHostLookupFinished(QHostInfo)));
                m_pendingHostLookups.insert(lookupId, reply);
            } else {
                finishReply(reply, PingReply::ErrorNoError);
            }



        } else if (responsePacket->icmp_type == ICMP_DEST_UNREACH) {

            // Get the sending package
            int messageOffset = sizeof(struct iphdr) + 8;
            struct iphdr *nestedIpHeader = (struct iphdr *)(receiveBuffer + messageOffset);
            int nestedIpHeaderLength = nestedIpHeader->ihl << 2;
            int nestedIcmpPacketSize = htons(nestedIpHeader->tot_len) - nestedIpHeaderLength;
            QHostAddress nestedSenderAddress(qFromBigEndian(nestedIpHeader->saddr));
            QHostAddress nestedDestinationAddress(qFromBigEndian(nestedIpHeader->daddr));

            qCDebug(dcPingTraffic()) << "++ IP header: Lenght" << nestedIpHeaderLength
                              << "Sender:" << nestedSenderAddress.toString()
                              << "Destination:" << nestedDestinationAddress.toString()
                              << "Size:" << htons(nestedIpHeader->tot_len) << "B"
                              << "TTL" << ipHeader->ttl;

            struct icmp *nestedResponsePacket = reinterpret_cast<struct icmp *>(receiveBuffer + messageOffset + nestedIpHeaderLength);
            icmpId = htons(nestedResponsePacket->icmp_id);
            icmpSequnceNumber = htons(nestedResponsePacket->icmp_seq);

            qCDebug(dcPingTraffic()) << "++ ICMP packt (Size:" << nestedIcmpPacketSize << "Bytes):"
                              << "Type" << nestedResponsePacket->icmp_type
                              << "Code:" << nestedResponsePacket->icmp_code
                              << "ID:" << QString("0x%1").arg(icmpId, 4, 16, QChar('0'))
                              << "Sequence:" << icmpSequnceNumber;

            qCDebug(dcPing()) << "ICMP destination unreachable" << nestedDestinationAddress.toString()
                              << "Code:" << nestedResponsePacket->icmp_code
                              << "ID:" << QString("0x%1").arg(icmpId, 4, 16, QChar('0'))
                              << "Sequence:" << icmpSequnceNumber;

            PingReply *reply = m_pendingReplies.take(icmpId);
            if (!reply) {
                qCDebug(dcPingTraffic()) << "No pending reply for ping echo response unreachable with ID"
                                              << QString("0x%1").arg(icmpId, 4, 16, QChar('0'))
                                              << "Sequence:" << icmpSequnceNumber
                                              << "from" << nestedSenderAddress.toString() << "to" << nestedDestinationAddress.toString();
                return;
            }

            finishReply(reply, PingReply::ErrorHostUnreachable);
        }
    }
}

void Ping::onHostLookupFinished(const QHostInfo &info)
{
    PingReply *reply = m_pendingHostLookups.value(info.lookupId());
    if (!reply) {
        qCWarning(dcPing()) << "Could not find reply after host lookup.";
        return;
    }

    if (info.error() != QHostInfo::NoError) {
        qCWarning(dcPing()) << "Failed to look up hostname after successfull ping" << reply->targetHostAddress().toString() << info.error();
    } else {
        qCDebug(dcPing()) << "Looked up hostname after successfull ping" << reply->targetHostAddress().toString() << info.hostName();
        if (info.hostName() != reply->targetHostAddress().toString()) {
            reply->m_hostName = info.hostName();
        }
    }

    finishReply(reply, PingReply::ErrorNoError);
}
