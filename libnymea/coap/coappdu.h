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

#ifndef COAPPDU_H
#define COAPPDU_H

#include <QDebug>
#include <QObject>

#include "libnymea.h"
#include "coapoption.h"
#include "coappdublock.h"

// PDU = Protocol Data Unit

/*         0                   1                   2                   3
 *       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      |Ver| T |  TKL  |      Code     |          Message ID           |
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      |   Token (if any, TKL bytes) ...
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      |   Options (if any) ...
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *      |1 1 1 1 1 1 1 1|    Payload (if any) ...
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

class LIBNYMEA_EXPORT CoapPdu : public QObject
{
    Q_OBJECT
    Q_ENUMS(MessageType)
    Q_ENUMS(StatusCode)
    Q_ENUMS(ContentType)

public:

    enum MessageType {
        Confirmable     = 0x00,
        NonConfirmable  = 0x01,
        Acknowledgement = 0x02,
        Reset           = 0x03
    };
    Q_ENUM(MessageType)

    // Methods:       https://tools.ietf.org/html/rfc7252#section-5.8
    // Respond codes: https://tools.ietf.org/html/rfc7252#section-12.1.2
    enum ReqRspCode {
        Empty                    = 0x00,  // Empty message (ping)
        Get                      = 0x01,  // Method GET
        Post                     = 0x02,  // Method POST
        Put                      = 0x03,  // Method PUT
        Delete                   = 0x04,  // Method DELETE
        Created                  = 0x41,  // 2.01
        Deleted                  = 0x42,  // 2.02
        Valid                    = 0x43,  // 2.03
        Changed                  = 0x44,  // 2.04
        Content                  = 0x45,  // 2.05
        Continue                 = 0x5f,  // 2.31 (Block)
        BadRequest               = 0x80,  // 4.00
        Unauthorized             = 0x81,  // 4.01
        BadOption                = 0x82,  // 4.02
        Forbidden                = 0x83,  // 4.03
        NotFound                 = 0x84,  // 4.04
        MethodNotAllowed         = 0x85,  // 4.05
        NotAcceptable            = 0x86,  // 4.06
        RequestEntityIncomplete  = 0x88,  // 4.08 (Block)
        PreconditionFailed       = 0x8c,  // 4.12
        RequestEntityTooLarge    = 0x8d,  // 4.13 (Block)
        UnsupportedContentFormat = 0x8f,  // 4.15
        InternalServerError      = 0xa0,  // 5.00
        NotImplemented           = 0xa1,  // 5.01
        BadGateway               = 0xa2,  // 5.02
        ServiceUnavailabl        = 0xa3,  // 5.03
        GatewayTimeout           = 0xa4,  // 5.04
        ProxyingNotSupported     = 0xa5   // 5.05
    };
    Q_ENUM(ReqRspCode)

    // https://tools.ietf.org/html/rfc7252#section-12.3
    enum ContentType {
        TextPlain        = 0,
        ApplicationLink  = 40,
        ApplicationXml   = 41,
        ApplicationOctet = 42,
        ApplicationExi   = 47,
        ApplicationJson  = 50
    };
    Q_ENUM(ContentType)

    enum Error {
        NoError,
        InvalidTokenError,
        InvalidPduSizeError,
        InvalidOptionDeltaError,
        InvalidOptionLengthError,
        UnknownOptionError
    };
    Q_ENUM(Error)

    CoapPdu(QObject *parent = 0);
    CoapPdu(const QByteArray &data, QObject *parent = 0);

    static QString getReqRspCodeString(CoapPdu::ReqRspCode reqRspCode);

    // header fields
    quint8 version() const;
    void setVersion(quint8 version);

    MessageType messageType() const;
    void setMessageType(MessageType messageType);

    ReqRspCode reqRspCode() const;
    void setReqRspCode(ReqRspCode reqRspCode);

    quint16 messageId() const;
    void createMessageId();
    void setMessageId(quint16 messageId);

    ContentType contentType() const;
    void setContentType(ContentType contentType);

    QByteArray token() const;
    void createToken();
    void setToken(const QByteArray &token);

    // payload
    QByteArray payload() const;
    void setPayload(const QByteArray &payload);

    QList<CoapOption> options() const;
    void addOption(CoapOption::Option option, const QByteArray &data);

    CoapPduBlock block() const;

    bool hasOption(CoapOption::Option option) const;
    CoapOption option(CoapOption::Option option) const;

    void clear();
    bool isValid() const;

    QByteArray pack() const;

private:
    quint8 m_version;
    MessageType m_messageType;
    ReqRspCode m_reqRspCode;
    quint16 m_messageId;
    ContentType m_contentType;
    QByteArray m_token;
    QByteArray m_payload;
    QList<CoapOption> m_options;

    CoapPduBlock m_block;

    Error m_error;

    void unpack(const QByteArray &data);
};

QDebug operator<<(QDebug debug, const CoapPdu &coapPdu);

#endif // COAPPDU_H
