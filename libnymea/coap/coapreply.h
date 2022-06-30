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

#ifndef COAPREPLY_H
#define COAPREPLY_H

#include <QObject>
#include <QTimer>

#include "libnymea.h"
#include "coappdu.h"
#include "coapoption.h"
#include "coaprequest.h"

class LIBNYMEA_EXPORT CoapReply : public QObject
{
    friend class Coap;

    Q_OBJECT
    Q_ENUMS(Error)

public:
    enum Error {
        NoError,
        HostNotFoundError,
        TimeoutError,
        InvalidUrlSchemeError,
        InvalidPduError
    };

    CoapRequest request() const;
    QByteArray payload() const;

    bool isFinished() const;
    bool isRunning() const;

    Error error() const;
    QString errorString() const;

    CoapPdu::ContentType contentType() const;
    CoapPdu::MessageType messageType() const;
    CoapPdu::ReqRspCode reqRspCode() const;

private:
    CoapReply(const CoapRequest &request, QObject *parent = 0);

    void appendPayloadData(const QByteArray &data);

    void setFinished();
    void setError(const Error &error);

    void resend();

    void setContentType(CoapPdu::ContentType contentType = CoapPdu::TextPlain);
    void setMessageType(CoapPdu::MessageType messageType);
    void setReqRspCode(CoapPdu::ReqRspCode reqRspCode);

    QTimer *m_timer;
    CoapRequest m_request;
    QByteArray m_payload;

    Error m_error;

    bool m_isFinished;
    int m_retransmissions;

    CoapPdu::ContentType m_contentType;
    CoapPdu::MessageType m_messageType;
    CoapPdu::ReqRspCode m_reqRspCode;

    // data for the request
    void setHostAddress(const QHostAddress &address);
    QHostAddress hostAddress() const;

    void setPort(int port);
    int port() const;

    void setRequestPayload(const QByteArray &requestPayload);
    QByteArray requestPayload() const;

    void setRequestMethod(CoapPdu::ReqRspCode method);
    CoapPdu::ReqRspCode requestMethod() const;

    void setRequestData(const QByteArray &requestData);
    QByteArray requestData() const;

    int messageId() const;
    void setMessageId(const int &messageId);

    QByteArray messageToken() const;
    void setMessageToken(const QByteArray &messageToken);

    bool observation() const;
    void setObservation(const bool &observation);

    bool observationEnable() const;
    void setObservationEnable(const bool &observationEnable);

    QHostAddress m_hostAddress;
    int m_port;
    CoapPdu::ReqRspCode m_requestMethod;
    QByteArray m_requestPayload;
    QByteArray m_requestData;
    bool m_lockedUp;
    int m_messageId;
    QByteArray m_messageToken;

    bool m_observation = false;
    bool m_observationEnable = false;

signals:
    void timeout();
    void finished();
    void error(const Error &code);
};

QDebug operator<<(QDebug debug, CoapReply *reply);

#endif // COAPREPLY_H
