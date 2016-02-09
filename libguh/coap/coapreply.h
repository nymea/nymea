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

#ifndef COAPREPLY_H
#define COAPREPLY_H

#include <QObject>
#include <QTimer>

#include "libguh.h"

#include "coappdu.h"
#include "coapoption.h"
#include "coaprequest.h"

class LIBGUH_EXPORT CoapReply : public QObject
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
    CoapPdu::StatusCode statusCode() const;

private:
    CoapReply(const CoapRequest &request, QObject *parent = 0);

    void appendPayloadData(const QByteArray &data);

    void setFinished();
    void setError(const Error &error);

    void resend();

    void setContentType(const CoapPdu::ContentType contentType = CoapPdu::TextPlain);
    void setMessageType(const CoapPdu::MessageType &messageType);
    void setStatusCode(const CoapPdu::StatusCode &statusCode);

    QTimer *m_timer;
    CoapRequest m_request;
    QByteArray m_payload;

    Error m_error;

    bool m_isFinished;
    int m_retransmissions;

    CoapPdu::ContentType m_contentType;
    CoapPdu::MessageType m_messageType;
    CoapPdu::StatusCode m_statusCode;

    // data for the request
    void setHostAddress(const QHostAddress &address);
    QHostAddress hostAddress() const;

    void setPort(const int &port);
    int port() const;

    void setRequestPayload(const QByteArray &requestPayload);
    QByteArray requestPayload() const;

    void setRequestMethod(const CoapPdu::StatusCode &method);
    CoapPdu::StatusCode requestMethod() const;

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
    CoapPdu::StatusCode m_requestMethod;
    QByteArray m_requestPayload;
    QByteArray m_requestData;
    bool m_lockedUp;
    int m_messageId;
    QByteArray m_messageToken;

    bool m_observation;
    bool m_observationEnable;

signals:
    void timeout();
    void finished();
    void error(const Error &code);
};

QDebug operator<<(QDebug debug, CoapReply *reply);

#endif // COAPREPLY_H
