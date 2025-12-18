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

#ifndef PING_H
#define PING_H

#include <QHostAddress>
#include <QHostInfo>
#include <QLoggingCategory>
#include <QObject>
#include <QQueue>
#include <QSocketNotifier>
#include <QTimer>
#include <QUrl>

#include "libnymea.h"
#include "pingreply.h"

#include <netinet/ip_icmp.h>

#define ICMP_PACKET_SIZE 64
#define ICMP_TTL_VALUE 64
#define ICMP_PAYLOAD_SIZE (ICMP_PACKET_SIZE - sizeof(struct icmphdr))

class LIBNYMEA_EXPORT Ping : public QObject
{
    Q_OBJECT
public:
    explicit Ping(QObject *parent = nullptr);

    QByteArray payload() const;
    void setPayload(const QByteArray &payload);

    bool available() const;

    PingReply::Error error() const;

    int queueCount() const;

    PingReply *ping(const QHostAddress &hostAddress, uint retries = 3);
    PingReply *ping(const QHostAddress &hostAddress, bool lookupHost, uint retries = 3);

    PingReply *ping(const QString &hostName, uint retries = 3);

signals:
    void availableChanged(bool available);

private:
    struct icmpPacket
    {
        struct icmphdr icmpHeadr;
        char icmpPayload[ICMP_PAYLOAD_SIZE];
    };

    // Config
    QByteArray m_payload = "ping from nymea";
    PingReply::Error m_error = PingReply::ErrorNoError;
    uint m_timeoutDuration = 5000;

    // Socket
    QSocketNotifier *m_socketNotifier = nullptr;
    int m_socketDescriptor = -1;
    QHash<quint16, PingReply *> m_pendingReplies;
    bool m_available = false;

    QQueue<PingReply *> m_replyQueue;
    QTimer *m_queueTimer = nullptr;
    PingReply *m_currentReply = nullptr;
    void sendNextReply();
    QHash<int, PingReply *> m_pendingHostNameLookups;
    QHash<int, PingReply *> m_pendingHostAddressLookups;

    //Error performPing(const QString &address);
    void performPing(PingReply *reply);
    void verifyErrno(int error);

    // Helper
    unsigned short calculateChecksum(unsigned short *b, int len);
    void cleanUpSocket();
    void timeValueSubtract(struct timeval *start, struct timeval *stop);
    quint16 calculateRequestId();

    PingReply *createReply(const QHostAddress &hostAddress);
    PingReply *createReply(const QString &hostName);
    void finishReply(PingReply *reply, PingReply::Error error);
    void cleanUpReply(PingReply *reply);

private slots:
    void onSocketReadyRead(int socketDescriptor);
    void onHostLookupFinished(const QHostInfo &info);
};

#endif // PING_H
