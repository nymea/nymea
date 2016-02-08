/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of QtCoap.                                           *
 *                                                                         *
 *  QtCoap is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 3 of the License.                *
 *                                                                         *
 *  QtCoap is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with QtCoap. If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef COAP_H
#define COAP_H

#include <QObject>
#include <QHostInfo>
#include <QUdpSocket>
#include <QHostAddress>
#include <QQueue>

#include "libguh.h"
#include "coaprequest.h"
#include "coapreply.h"
#include "coapobserveresource.h"

/* Information about CoAP
 *
 * The Constrained Application Protocol (CoAP)         : https://tools.ietf.org/html/rfc7252
 * Blockwise transfers in CoAP                         : https://tools.ietf.org/html/draft-ietf-core-block-18
 * Constrained RESTful Environments (CoRE) Link Format : http://tools.ietf.org/html/rfc6690
 * Observing Resources in CoAP                         : https://tools.ietf.org/html/rfc7641
 *
 */

class LIBGUH_EXPORT Coap : public QObject
{
    Q_OBJECT

public:
    explicit Coap(QObject *parent = 0, const quint16 &port = 5683);

    CoapReply *ping(const CoapRequest &request);
    CoapReply *get(const CoapRequest &request);
    CoapReply *put(const CoapRequest &request, const QByteArray &data = QByteArray());
    CoapReply *post(const CoapRequest &request, const QByteArray &data = QByteArray());
    CoapReply *deleteResource(const CoapRequest &request);

    // Notifications for observable resources
    CoapReply *enableResourceNotifications(const CoapRequest &request);
    CoapReply *disableNotifications(const CoapRequest &request);

private:
    QUdpSocket *m_socket;

    CoapReply *m_reply;
    QHash<int, CoapReply *> m_runningHostLookups;
    QHash<QByteArray, CoapObserveResource> m_observeResources;

    QQueue<CoapReply *> m_replyQueue;

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

signals:
    void replyFinished(CoapReply *reply);
    void notificationReceived(const CoapObserveResource &resource, const int &notificationNumber, const QByteArray &payload);

private slots:
    void hostLookupFinished(const QHostInfo &hostInfo);
    void onReadyRead();
    void onReplyTimeout();
    void onReplyFinished();
};


#endif // COAP_H
