/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
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

/*!
    \class Coap
    \brief The client connection class to a CoAP server.

    \ingroup coap-group
    \inmodule libnymea

    The Coap class provides a signal solt based communication with a \l{https://tools.ietf.org/html/rfc7252}{CoAP (Constrained Application Protocol)}
    server. The API of this class was inspired by the \l{http://doc.qt.io/qt-5/qnetworkaccessmanager.html}{QNetworkAccessManager} and was
    written according to the \l{https://tools.ietf.org/html/rfc7252}{RFC7252}.
    This class supports also blockwise transfere according to the \l{https://tools.ietf.org/html/draft-ietf-core-block-18}{IETF V18} specifications and
    observing resources according to the \l{https://tools.ietf.org/html/rfc7641}{RFC7641}.

    \sa CoapReply, CoapRequest

    \section2 Example
    \code
        MyClass::MyClass(QObject *parent) :
          QObject(parent)
        {
          Coap *coap = new Coap(this);
          connect(coap, SIGNAL(replyFinished(CoapReply*)), this, SLOT(onReplyFinished(CoapReply*)));

          CoapRequest request(QUrl("coap://coap.me/hello"));
          coap->get(request);
        }
    \endcode

    \code
        void MyClass::onReplyFinished(CoapReply *reply)
        {
          if (reply->error() != CoapReply::NoError) {
            qWarning() << "Reply finished with error" << reply->errorString();
            reply->deleteLater();
            return;
          }

          qDebug() << "Reply finished" << reply;
          reply->deleteLater();
        }
    \endcode
*/

/*! \fn void Coap::replyFinished(CoapReply *reply);
    This signal is emitted when the given \a reply is finished.
*/

/*! \fn void Coap::notificationReceived(const CoapObserveResource &resource, const int &notificationNumber, const QByteArray &payload);
    This signal is emitted when a value of an observed \a resource changed. The \a notificationNumber specifies the count of the notification
    to keep the correct order. The value can be parsed from the \a payload parameter.
*/

#include "coap.h"
#include "coappdu.h"
#include "coapoption.h"

Q_LOGGING_CATEGORY(dcCoap, "Coap")

/*! Constructs a Coap access manager with the given \a parent and \a port. */
Coap::Coap(QObject *parent, const quint16 &port) :
    QObject(parent),
    m_reply(nullptr)
{
    m_socket = new QUdpSocket(this);

    if (!m_socket->bind(QHostAddress::AnyIPv4, port, QAbstractSocket::ShareAddress))
        qCWarning(dcCoap) << "Could not bind to port" << port << m_socket->errorString();

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

/*! Performs a ping request to the CoAP server specified in the given \a request.
 *  Returns a \l{CoapReply} to match the response with the request. */
CoapReply *Coap::ping(const CoapRequest &request)
{
    return customRequest(CoapPdu::Empty, request);
}

/*! Performs a GET request to the CoAP server specified in the given \a request.
 *  Returns a \l{CoapReply} to match the response with the request. */
CoapReply *Coap::get(const CoapRequest &request)
{
    return customRequest(CoapPdu::Get, request);
}

/*! Performs a PUT request to the CoAP server specified in the given \a request and \a data.
 *  Returns a \l{CoapReply} to match the response with the request. */
CoapReply *Coap::put(const CoapRequest &request, const QByteArray &data)
{
    return customRequest(CoapPdu::Put, request, data);
}

/*! Performs a POST request to the CoAP server specified in the given \a request and \a data.
 *  Returns a \l{CoapReply} to match the response with the request. */
CoapReply *Coap::post(const CoapRequest &request, const QByteArray &data)
{
    return customRequest(CoapPdu::Post, request, data);
}

/*! Performs a DELETE request to the CoAP server specified in the given \a request.
 *  Returns a \l{CoapReply} to match the response with the request. */
CoapReply *Coap::deleteResource(const CoapRequest &request)
{
    return customRequest(CoapPdu::Delete, request);
}

CoapReply *Coap::customRequest(CoapPdu::ReqRspCode requestCode, const CoapRequest &request, const QByteArray &data)
{
    CoapReply *reply = new CoapReply(request, this);
    reply->setRequestMethod(requestCode);
    if (!data.isEmpty()) {
        reply->setRequestPayload(data);
    }

    connect(reply, &CoapReply::timeout, this, &Coap::onReplyTimeout);
    connect(reply, &CoapReply::finished, this, &Coap::onReplyFinished);

    if (request.url().scheme() != "coap") {
        reply->setError(CoapReply::InvalidUrlSchemeError);
        reply->m_isFinished = true;
        return reply;
    }

    // check if there is a request running
    if (m_reply.isNull()) {
        m_reply = reply;
        lookupHost();
    } else {
        m_replyQueue.enqueue(reply);
    }

    return reply;
}

/*! Enables notifications (observing) on the CoAP server for the resource specified in the
 *  given \a request.
 *  Returns a \l{CoapReply} to match the response with the request. */
CoapReply *Coap::enableResourceNotifications(const CoapRequest &request)
{
    CoapReply *reply = new CoapReply(request, this);
    reply->setRequestMethod(CoapPdu::Get);
    reply->setObservation(true);
    reply->setObservationEnable(true);

    connect(reply, &CoapReply::timeout, this, &Coap::onReplyTimeout);
    connect(reply, &CoapReply::finished, this, &Coap::onReplyFinished);

    if (request.url().scheme() != "coap") {
        reply->setError(CoapReply::InvalidUrlSchemeError);
        reply->m_isFinished = true;
        return reply;
    }

    // check if there is a request running
    if (m_reply.isNull()) {
        m_reply = reply;
        lookupHost();
    } else {
        m_replyQueue.enqueue(reply);
    }

    return reply;
}

/*! Disables notifications (observing) on the CoAP server for the resource specified in the
 *  given \a request.
 *  Returns a \l{CoapReply} to match the response with the request. */
CoapReply *Coap::disableNotifications(const CoapRequest &request)
{
    CoapReply *reply = new CoapReply(request, this);
    reply->setRequestMethod(CoapPdu::Get);
    reply->setMessageType(CoapPdu::Reset);
    reply->setObservation(true);
    reply->setObservationEnable(false);

    connect(reply, &CoapReply::timeout, this, &Coap::onReplyTimeout);
    connect(reply, &CoapReply::finished, this, &Coap::onReplyFinished);

    if (request.url().scheme() != "coap") {
        reply->setError(CoapReply::InvalidUrlSchemeError);
        reply->m_isFinished = true;
        return reply;
    }

    // check if there is a request running
    if (m_reply == 0) {
        m_reply = reply;
        lookupHost();
    } else {
        m_replyQueue.enqueue(reply);
    }

    return reply;
}

bool Coap::joinMulticastGroup(const QHostAddress &address)
{
    bool ok = m_socket->joinMulticastGroup(address);
    if (ok) {
        qCDebug(dcCoap()) << "Joined CoAP multucast group on:" << address;
    } else {
        qCWarning(dcCoap()) << "Error joining CoAP multicast group on:" << address;
    }
    return ok;
}

bool Coap::leaveMulticastGroup(const QHostAddress &address)
{
    return m_socket->leaveMulticastGroup(address);
}


void Coap::lookupHost()
{
    int lookupId = QHostInfo::lookupHost(m_reply->request().url().host(), this, SLOT(hostLookupFinished(QHostInfo)));
    m_runningHostLookups.insert(lookupId, m_reply);
}

void Coap::sendRequest(CoapReply *reply, const bool &lookedUp)
{
    CoapPdu pdu;
    pdu.setMessageType(reply->request().messageType());
    pdu.setReqRspCode(reply->requestMethod());
    pdu.createMessageId();
    pdu.createToken();

    // Add the options in correct order
    // Option number 3
    if (lookedUp)
        pdu.addOption(CoapOption::UriHost, reply->request().url().host().toUtf8());

    if (reply->observation() && reply->requestMethod() == CoapPdu::Get) {
        if (reply->observationEnable()) {
            // Option number 6
            pdu.addOption(CoapOption::Observe, 0);
            m_observeResources.insert(pdu.token(), CoapObserveResource(reply->request().url(), pdu.token()));
        } else {
            // if disable, we should use the sam token as the notifications
            foreach (const CoapObserveResource &resource, m_observeResources.values()) {
                if (resource.url() == reply->request().url()) {
                    pdu.setToken(resource.token());
                }
            }
            // Option number 6
            pdu.addOption(CoapOption::Observe, QByteArray::number(1));
            if (m_observeResources.contains(pdu.token()))
                m_observeResources.remove(pdu.token());
        }
    }

    // Option number 7
    if (reply->port() != 5683)
        pdu.addOption(CoapOption::UriPort, QByteArray::number(reply->request().url().port()));

    QStringList urlTokens = reply->request().url().path().split("/");
    urlTokens.removeAll(QString());

    // Option number 11
    foreach (const QString &token, urlTokens)
        pdu.addOption(CoapOption::UriPath, token.toUtf8());

    // Option number 12
    if (reply->requestMethod() == CoapPdu::Post || reply->requestMethod() == CoapPdu::Put) {
        pdu.addOption(CoapOption::ContentFormat, QByteArray(1, ((quint8)reply->request().contentType())));

        // check if we have to block the payload
        if (reply->requestPayload().size() > 64) {
            pdu.addOption(CoapOption::Block1, CoapPduBlock::createBlock(0, 2, true));
            pdu.setPayload(reply->requestPayload().mid(0, 64));
        } else {
            pdu.setPayload(reply->requestPayload());
        }
    }

    // Option number 15
    if (reply->request().url().hasQuery())
        pdu.addOption(CoapOption::UriQuery, reply->request().url().query().toUtf8());

    // Option number 23
    if (reply->requestMethod() == CoapPdu::Get)
        pdu.addOption(CoapOption::Block2, CoapPduBlock::createBlock(0));

    QByteArray pduData = pdu.pack();
    reply->setRequestData(pduData);
    reply->setMessageId(pdu.messageId());
    reply->setMessageToken(pdu.token());
    reply->m_lockedUp = lookedUp;
    reply->m_timer->start();

    qCDebug(dcCoap) << "--->" << pdu;

    // send the data
    if (reply->request().messageType() == CoapPdu::NonConfirmable) {
        sendData(reply->hostAddress(), reply->port(), pduData);
        reply->setFinished();
    } else {
        sendData(reply->hostAddress(), reply->port(), pduData);
    }
}

void Coap::sendData(const QHostAddress &hostAddress, const quint16 &port, const QByteArray &data)
{
    m_socket->writeDatagram(data, hostAddress, port);
}

void Coap::sendCoapPdu(const QHostAddress &hostAddress, const quint16 &port, const CoapPdu &pdu)
{
    m_socket->writeDatagram(pdu.pack(), hostAddress, port);
}

void Coap::processResponse(const CoapPdu &pdu, const QHostAddress &address, const quint16 &port)
{
    // if we are waiting for a reply response
    if (m_reply) {
        qCDebug(dcCoap) << "<---" << QString("%1:%2").arg(address.toString()).arg(QString::number(port)) << pdu;
        if (!pdu.isValid()) {
            qCWarning(dcCoap) << "Got invalid PDU";
            m_reply->setError(CoapReply::InvalidPduError);
            m_reply->setFinished();
            return;
        }

        // check if the message is a response to a reply (message id based check)
        if (m_reply->messageId() == pdu.messageId()) {
            processIdBasedResponse(m_reply, pdu);
            return;
        }

        // check if we know the message by token (message token based check)
        if (m_reply->messageToken() == pdu.token()) {
            processTokenBasedResponse(m_reply, pdu);
            return;
        }
    }


    if (m_observerReply) {
        processBlock2Notification(m_observerReply, pdu);
        return;
    }

    // check if this is a notification
    if (m_observeResources.keys().contains(pdu.token())) {
        processNotification(pdu, address, port);
        return;
    }

    // If this is neither a response to a request of a notification from an observed resource, it's a multicast message
    qCDebug(dcCoap) << "<---" << QString("%1:%2").arg(address.toString()).arg(QString::number(port)) << pdu;
    emit multicastMessageReceived(address, pdu);
}

void Coap::processIdBasedResponse(CoapReply *reply, const CoapPdu &pdu)
{
    // check if this is an empty ACK response (which indicates a separated response)
    if (pdu.reqRspCode() == CoapPdu::Empty && pdu.messageType() == CoapPdu::Acknowledgement) {
        reply->m_timer->stop();
        qCDebug(dcCoap) << "Got empty ACK. Data will be sent separated.";
        return;
    }

    // check if this is a Block1 pdu
    if (pdu.messageType() == CoapPdu::Acknowledgement && pdu.hasOption(CoapOption::Block1)) {
        processBlock1Response(reply, pdu);
        return;
    }

    // check if this is a Block2 pdu
    if (pdu.messageType() == CoapPdu::Acknowledgement && pdu.hasOption(CoapOption::Block2)) {
        processBlock2Response(reply, pdu);
        return;
    }

    // Piggybacked response
    reply->setReqRspCode(pdu.reqRspCode());
    reply->setContentType(pdu.contentType());
    reply->appendPayloadData(pdu.payload());
    reply->setFinished();
}

void Coap::processTokenBasedResponse(CoapReply *reply, const CoapPdu &pdu)
{
    // Separate Response
    CoapPdu responsePdu;
    responsePdu.setMessageType(CoapPdu::Acknowledgement);
    responsePdu.setReqRspCode(CoapPdu::Empty);
    responsePdu.setMessageId(pdu.messageId());
    sendCoapPdu(reply->hostAddress(), reply->port(), responsePdu);

    reply->setReqRspCode(pdu.reqRspCode());
    reply->setContentType(pdu.contentType());
    reply->appendPayloadData(pdu.payload());
    reply->setFinished();
}

void Coap::processNotification(const CoapPdu &pdu, const QHostAddress &address, const quint16 &port)
{
    CoapObserveResource resource = m_observeResources.value(pdu.token());
    qCDebug(dcCoap) << "<--- Notification" << endl << pdu;

    // check if it is a blockwise notification
    if (pdu.hasOption(CoapOption::Block2)) {
        if (!m_observeReplyResource.values().contains(resource)) {

            qCDebug(dcCoap) << "Got first part of blocked notification";

            // First part of the blocked notification
            // respond with ACK
            CoapPdu responsePdu;
            responsePdu.setMessageType(CoapPdu::Acknowledgement);
            responsePdu.setReqRspCode(CoapPdu::Empty);
            responsePdu.setMessageId(pdu.messageId());
            responsePdu.setToken(pdu.token());

            qCDebug(dcCoap) << "---> Notification" << endl << responsePdu;
            sendCoapPdu(address, port, responsePdu);

            // create reply for blockwise transfere
            if (!m_observerReply.isNull()) {
                m_observeBlockwise.remove(m_observerReply);
                m_observerReply->deleteLater();
            }

            m_observerReply = new CoapReply(CoapRequest(resource.url()), this);
            m_observerReply->setRequestMethod(CoapPdu::Get);
            m_observerReply->appendPayloadData(pdu.payload());

            // Lets store the observation number
            int notificationNumber = 0;
            foreach (const CoapOption &option, pdu.options()) {
                if (option.option() == CoapOption::Observe) {
                    notificationNumber = option.data().toHex().toInt(0, 16);
                }
            }

            m_observeReplyResource.insert(m_observerReply, resource);
            m_observeBlockwise.insert(m_observerReply, notificationNumber);

            connect(m_observerReply.data(), &CoapReply::timeout, this, &Coap::onReplyTimeout);
            connect(m_observerReply.data(), &CoapReply::finished, this, &Coap::onReplyFinished);

            CoapPdu pdu;
            pdu.setMessageType(m_observerReply->request().messageType());
            pdu.setReqRspCode(m_observerReply->requestMethod());
            pdu.createMessageId();
            pdu.createToken();

            // Add the options in correct order
            // Option number 3
            pdu.addOption(CoapOption::UriHost, m_observerReply->request().url().host().toUtf8());

            QStringList urlTokens = m_observerReply->request().url().path().split("/");
            urlTokens.removeAll(QString());

            // Option number 11
            foreach (const QString &token, urlTokens)
                pdu.addOption(CoapOption::UriPath, token.toUtf8());

            // Option number 15
            if (m_observerReply->request().url().hasQuery())
                pdu.addOption(CoapOption::UriQuery, m_observerReply->request().url().query().toUtf8());

            // Option number 23
            pdu.addOption(CoapOption::Block2, CoapPduBlock::createBlock(1, 2, true));

            QByteArray pduData = pdu.pack();
            m_observerReply->setRequestData(pduData);
            m_observerReply->setHostAddress(address);
            m_observerReply->setPort(port);
            m_observerReply->m_timer->start();

            qCDebug(dcCoap) << "---> Notification" << endl << pdu;
            sendData(address, port, pduData);

            return;
        }
    }

    // respond with ACK
    CoapPdu responsePdu;
    responsePdu.setMessageType(CoapPdu::Acknowledgement);
    responsePdu.setReqRspCode(CoapPdu::Empty);
    responsePdu.setMessageId(pdu.messageId());
    responsePdu.setToken(pdu.token());

    qCDebug(dcCoap) << "---> Notification" << endl << responsePdu;
    sendCoapPdu(address, port, responsePdu);

    int notificationNumber = 0;
    foreach (const CoapOption &option, pdu.options()) {
        if (option.option() == CoapOption::Observe) {
            notificationNumber = option.data().toHex().toInt(0, 16);
        }
    }

    emit notificationReceived(resource, notificationNumber, pdu.payload());
}

void Coap::processBlock1Response(CoapReply *reply, const CoapPdu &pdu)
{
    qCDebug(dcCoap) << "Sent successfully block #" << pdu.block().blockNumber();

    // create next block
    int index = (pdu.block().blockNumber() * 64) + 64;
    QByteArray newBlockData = reply->requestPayload().mid(index, 64);
    bool moreFlag = true;

    // check if this was the last block
    if (newBlockData.isEmpty()) {
        reply->setReqRspCode(pdu.reqRspCode());
        reply->setContentType(pdu.contentType());
        reply->setFinished();
        return;
    }

    // check if this is the last block or there will be no next block
    if (newBlockData.size() < 64 || (index + 64) == reply->requestPayload().size())
        moreFlag = false;

    CoapPdu nextBlockRequest;
    nextBlockRequest.setContentType(reply->request().contentType());
    nextBlockRequest.setMessageType(reply->request().messageType());
    nextBlockRequest.setReqRspCode(reply->requestMethod());
    nextBlockRequest.setMessageId(pdu.messageId() + 1);
    nextBlockRequest.setToken(pdu.token());

    // Add the options in correct order
    // Option number 3
    if (reply->m_lockedUp)
        nextBlockRequest.addOption(CoapOption::UriHost, reply->request().url().host().toUtf8());

    // Option number 7
    if (reply->port() != 5683)
        nextBlockRequest.addOption(CoapOption::UriPort, QByteArray::number(reply->request().url().port()));

    QStringList urlTokens = reply->request().url().path().split("/");
    urlTokens.removeAll(QString());

    // Option number 11
    foreach (const QString &token, urlTokens)
        nextBlockRequest.addOption(CoapOption::UriPath, token.toUtf8());

    // Option number 15
    if (reply->request().url().hasQuery())
        nextBlockRequest.addOption(CoapOption::UriQuery, reply->request().url().query().toUtf8());

    // Option number 27
    nextBlockRequest.addOption(CoapOption::Block1, CoapPduBlock::createBlock(pdu.block().blockNumber() + 1, 2, moreFlag));

    nextBlockRequest.setPayload(newBlockData);

    QByteArray pduData = nextBlockRequest.pack();
    reply->setRequestData(pduData);
    reply->m_timer->start();
    reply->m_retransmissions = 1;

    reply->setMessageId(nextBlockRequest.messageId());

    qCDebug(dcCoap) << "--->" << nextBlockRequest;
    sendData(reply->hostAddress(), reply->port(), pduData);
}

void Coap::processBlock2Response(CoapReply *reply, const CoapPdu &pdu)
{
    reply->appendPayloadData(pdu.payload());

    // check if this was the last block
    if (!pdu.block().moreFlag()) {
        reply->setReqRspCode(pdu.reqRspCode());
        reply->setContentType(pdu.contentType());
        reply->setFinished();
        return;
    }

    CoapPdu nextBlockRequest;
    nextBlockRequest.setContentType(reply->request().contentType());
    nextBlockRequest.setMessageType(reply->request().messageType());
    nextBlockRequest.setReqRspCode(reply->requestMethod());
    nextBlockRequest.setMessageId(pdu.messageId() + 1);
    nextBlockRequest.setToken(pdu.token());

    // Add the options in correct order
    // Option number 3
    if (reply->m_lockedUp)
        nextBlockRequest.addOption(CoapOption::UriHost, reply->request().url().host().toUtf8());

    // Option number 7
    if (reply->port() != 5683)
        nextBlockRequest.addOption(CoapOption::UriPort, QByteArray::number(reply->request().url().port()));

    QStringList urlTokens = reply->request().url().path().split("/");
    urlTokens.removeAll(QString());

    // Option number 11
    foreach (const QString &token, urlTokens)
        nextBlockRequest.addOption(CoapOption::UriPath, token.toUtf8());

    // Option number 15
    if (reply->request().url().hasQuery())
        nextBlockRequest.addOption(CoapOption::UriQuery, reply->request().url().query().toUtf8());

    // Option number 23
    nextBlockRequest.addOption(CoapOption::Block2, CoapPduBlock::createBlock(pdu.block().blockNumber() + 1, 2, false));

    QByteArray pduData = nextBlockRequest.pack();
    reply->setRequestData(pduData);
    reply->m_timer->start();

    reply->setMessageId(nextBlockRequest.messageId());

    qCDebug(dcCoap) << "--->" << nextBlockRequest;
    sendData(reply->hostAddress(), reply->port(), pduData);
}

void Coap::processBlock2Notification(CoapReply *reply, const CoapPdu &pdu)
{
    if (!m_observeReplyResource.contains(reply)) {
        qCWarning(dcCoap) << "Could not find observation resource for" << reply;
        return;
    }

    CoapObserveResource resource = m_observeReplyResource.value(reply);

    // respond Block2
    // check if this was the last block
    if (!pdu.block().moreFlag()) {
        // respond with ACK
        CoapPdu responsePdu;
        responsePdu.setMessageType(CoapPdu::Acknowledgement);
        responsePdu.setReqRspCode(CoapPdu::Empty);
        responsePdu.setMessageId(pdu.messageId());
        responsePdu.setToken(pdu.token());

        qCDebug(dcCoap) << "---> Notification" << endl << responsePdu;
        sendCoapPdu(reply->hostAddress(), reply->port(), responsePdu);

        reply->appendPayloadData(pdu.payload());

        emit notificationReceived(resource, m_observeBlockwise.take(reply), reply->payload());
        m_observeReplyResource.remove(m_observerReply);
        m_observerReply->deleteLater();
        m_observerReply.clear();
        return;
    }

    reply->appendPayloadData(pdu.payload());

    CoapPdu nextBlockRequest;
    nextBlockRequest.setContentType(reply->request().contentType());
    nextBlockRequest.setMessageType(reply->request().messageType());
    nextBlockRequest.setReqRspCode(reply->requestMethod());
    nextBlockRequest.setMessageId(pdu.messageId() + 1);
    nextBlockRequest.setToken(pdu.token());

    // Add the options in correct order
    // Option number 3
    if (reply->m_lockedUp)
        nextBlockRequest.addOption(CoapOption::UriHost, reply->request().url().host().toUtf8());

    // Option number 7
    if (reply->port() != 5683)
        nextBlockRequest.addOption(CoapOption::UriPort, QByteArray::number(reply->request().url().port()));

    QStringList urlTokens = reply->request().url().path().split("/");
    urlTokens.removeAll(QString());

    // Option number 11
    foreach (const QString &token, urlTokens)
        nextBlockRequest.addOption(CoapOption::UriPath, token.toUtf8());

    // Option number 15
    if (reply->request().url().hasQuery())
        nextBlockRequest.addOption(CoapOption::UriQuery, reply->request().url().query().toUtf8());

    // Option number 23
    nextBlockRequest.addOption(CoapOption::Block2, CoapPduBlock::createBlock(pdu.block().blockNumber() + 1, 2, false));

    QByteArray pduData = nextBlockRequest.pack();
    reply->setRequestData(pduData);
    reply->m_timer->start();

    reply->setMessageId(nextBlockRequest.messageId());

    qCDebug(dcCoap) << "---> Notification" << endl << nextBlockRequest;
    sendData(reply->hostAddress(), reply->port(), pduData);
}

void Coap::hostLookupFinished(const QHostInfo &hostInfo)
{
    CoapReply *reply = m_runningHostLookups.take(hostInfo.lookupId());;
    reply->setPort(reply->request().url().port(5683));

    if (hostInfo.error() != QHostInfo::NoError) {
        qCDebug(dcCoap) << "Host lookup for" << reply->request().url().host() << "failed:" << hostInfo.errorString();
        reply->setError(CoapReply::HostNotFoundError);
        reply->setFinished();
        return;
    }

    QHostAddress hostAddress = hostInfo.addresses().first();
    reply->setHostAddress(hostAddress);

    // check if the url had to be looked up
    if (reply->request().url().host() != hostAddress.toString()) {
        qCDebug(dcCoap) << reply->request().url().host() << " -> " << hostAddress.toString();
        sendRequest(reply, true);
    } else {
        sendRequest(reply);
    }
}

void Coap::onReadyRead()
{
    QHostAddress hostAddress;
    quint16 port;

    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram(m_socket->pendingDatagramSize(), 0);
        m_socket->readDatagram(datagram.data(), datagram.size(), &hostAddress, &port);
        qCDebug(dcCoap()) << "Datagram received from:" << hostAddress << ":" << datagram.toHex();
        CoapPdu pdu(datagram);
        processResponse(pdu, hostAddress, port);
    }
}

void Coap::onReplyTimeout()
{
    CoapReply *reply = qobject_cast<CoapReply *>(sender());
    if (reply->m_retransmissions < 5) {
        qCDebug(dcCoap) << QString("Reply timeout: resending message %1/4").arg(reply->m_retransmissions);
    }
    reply->resend();
    m_socket->writeDatagram(reply->requestData(), reply->hostAddress(), reply->port());
}

void Coap::onReplyFinished()
{
    CoapReply *reply = qobject_cast<CoapReply *>(sender());

    if (reply == m_observerReply) {
        m_observeReplyResource.remove(m_observerReply);
        m_observerReply->deleteLater();
        m_observerReply.clear();
        qCWarning(dcCoap) << "Notification reply finished wirh error" << reply->errorString();
        return;
    }

    if (reply != m_reply)
        qCWarning(dcCoap) << "This should never happen!! Please report a bug if you get this message!";

    emit replyFinished(reply);
    m_reply.clear();

    // check if there is a request in the queue
    if (!m_replyQueue.isEmpty()) {
        m_reply = m_replyQueue.dequeue();
        if (m_reply)
            lookupHost();
    }
}
