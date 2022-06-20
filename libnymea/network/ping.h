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

#ifndef PING_H
#define PING_H

#include <QUrl>
#include <QQueue>
#include <QTimer>
#include <QObject>
#include <QHostInfo>
#include <QHostAddress>
#include <QSocketNotifier>
#include <QLoggingCategory>

#include "libnymea.h"
#include "pingreply.h"

#include <netinet/ip_icmp.h>

#define ICMP_PACKET_SIZE  64
#define ICMP_TTL_VALUE  64
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

    PingReply *ping(const QHostAddress &hostAddress, uint retries = 3);
    PingReply *ping(const QHostAddress &hostAddress, bool lookupHost, uint retries = 3);

signals:
    void availableChanged(bool available);

private:
    struct icmpPacket {
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
    void sendNextReply();
    QHash<int, PingReply *> m_pendingHostLookups;

    //Error performPing(const QString &address);
    void performPing(PingReply *reply);
    void verifyErrno(int error);

    // Helper
    unsigned short calculateChecksum(unsigned short *b, int len);
    void cleanUpSocket();
    void timeValueSubtract(struct timeval *start, struct timeval *stop);
    quint16 calculateRequestId();

    PingReply *createReply(const QHostAddress &hostAddress);
    void finishReply(PingReply *reply, PingReply::Error error);

private slots:
    void onSocketReadyRead(int socketDescriptor);
    void onHostLookupFinished(const QHostInfo &info);

};

#endif // PING_H
