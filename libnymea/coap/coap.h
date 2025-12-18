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

#ifndef COAP_H
#define COAP_H

#include <QHostAddress>
#include <QHostInfo>
#include <QLoggingCategory>
#include <QObject>
#include <QPointer>
#include <QQueue>
#include <QUdpSocket>

#include "coapobserveresource.h"
#include "coapreply.h"
#include "coaprequest.h"
#include "libnymea.h"

/* Information about CoAP
 *
 * The Constrained Application Protocol (CoAP)         : https://tools.ietf.org/html/rfc7252
 * Blockwise transfers in CoAP                         : https://tools.ietf.org/html/draft-ietf-core-block-18
 * Constrained RESTful Environments (CoRE) Link Format : http://tools.ietf.org/html/rfc6690
 * Observing Resources in CoAP                         : https://tools.ietf.org/html/rfc7641
 *
 */

class LIBNYMEA_EXPORT Coap : public QObject
{
    Q_OBJECT

public:
    Coap(QObject *parent = nullptr, const quint16 &port = 5683);

    CoapReply *ping(const CoapRequest &request);
    CoapReply *get(const CoapRequest &request);
    CoapReply *put(const CoapRequest &request, const QByteArray &data = QByteArray());
    CoapReply *post(const CoapRequest &request, const QByteArray &data = QByteArray());
    CoapReply *deleteResource(const CoapRequest &request);
    CoapReply *customRequest(CoapPdu::ReqRspCode requestCode, const CoapRequest &request, const QByteArray &data = QByteArray());

    // Notifications for observable resources
    CoapReply *enableResourceNotifications(const CoapRequest &request);
    CoapReply *disableNotifications(const CoapRequest &request);

    bool joinMulticastGroup(const QHostAddress &address = QHostAddress("224.0.1.187"));
    bool leaveMulticastGroup(const QHostAddress &address = QHostAddress("224.0.1.187"));

private:
    QUdpSocket *m_socket;

    QPointer<CoapReply> m_reply;
    QQueue<CoapReply *> m_replyQueue;

    QHash<int, CoapReply *> m_runningHostLookups;

    QHash<QByteArray, CoapObserveResource> m_observeResources; // token | resource

    // Blockwise notifications
    QPointer<CoapReply> m_observerReply;
    QHash<CoapReply *, CoapObserveResource> m_observeReplyResource; // observe reply | resource
    QHash<CoapReply *, int> m_observeBlockwise;                     // observe reply | observe nr.

    void lookupHost();
    void sendRequest(CoapReply *reply, const bool &lookedUp = false);
    void sendData(const QHostAddress &hostAddress, const quint16 &port, const QByteArray &data);
    void sendCoapPdu(const QHostAddress &address, const quint16 &port, const CoapPdu &pdu);

    void processResponse(const CoapPdu &pdu, const QHostAddress &address, const quint16 &port);
    void processIdBasedResponse(CoapReply *reply, const CoapPdu &pdu);
    void processTokenBasedResponse(CoapReply *reply, const CoapPdu &pdu);

    void processNotification(const CoapPdu &pdu, const QHostAddress &address, const quint16 &port);

    void processBlock1Response(CoapReply *reply, const CoapPdu &pdu);
    void processBlock2Response(CoapReply *reply, const CoapPdu &pdu);

    void processBlock2Notification(CoapReply *reply, const CoapPdu &pdu);

signals:
    void replyFinished(CoapReply *reply);
    void notificationReceived(const CoapObserveResource &resource, const int &notificationNumber, const QByteArray &payload);
    void multicastMessageReceived(const QHostAddress &source, const CoapPdu &pdu);

private slots:
    void hostLookupFinished(const QHostInfo &hostInfo);
    void onReadyRead();
    void onReplyTimeout();
    void onReplyFinished();
};

#endif // COAP_H
