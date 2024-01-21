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
    \class CoapPdu
    \brief Represents a CoAP protocol data unit (PDU).

    \ingroup coap-group
    \inmodule libnymea

*/

/*! \enum CoapPdu::MessageType

    \value Confirmable
    \value NonConfirmable
    \value Acknowledgement
    \value Reset

*/

/*! \enum CoapPdu::StatusCode

    The CoAP status codes.

    Methods: \l{https://tools.ietf.org/html/rfc7252#section-5.8}

    Status codes: \l{https://tools.ietf.org/html/rfc7252#section-12.1.2}

    \value Empty
        0.00 Empty (i.e. response to ping request)
    \value Get
        The GET method
    \value Post
        The POST method
    \value Put
        The PUT method
    \value Delete
        The DELETE method
    \value Created
        2.01 Created
    \value Deleted
        2.02 Deleted
    \value Valid
        2.03 Valid
    \value Changed
        2.04 Changed
    \value Content
        2.05 Content
    \value Continue
        2.31 Continue (from \l{https://tools.ietf.org/html/draft-ietf-core-block-18}{Blockwise V18})
    \value BadRequest
        4.00 Bad Request
    \value Unauthorized
        4.01 Unauthorized
    \value BadOption
        4.02 Bad Option
    \value Forbidden
        4.03 Forbidden
    \value NotFound
        4.04 Not Found
    \value MethodNotAllowed
        4.05 Method Not Allowed
    \value NotAcceptable
        4.06 Not Acceptable
    \value RequestEntityIncomplete
        4.08 Request Entity Incomplete (from \l{https://tools.ietf.org/html/draft-ietf-core-block-18}{Blockwise V18})
    \value PreconditionFailed
        4.12 Precondition Failed
    \value RequestEntityTooLarge
        4.13 Request Entity Too Large (from \l{https://tools.ietf.org/html/draft-ietf-core-block-18}{Blockwise V18})
    \value UnsupportedContentFormat
        4.15 UnsupportedContentFormat
    \value InternalServerError
        5.00 Internal Server Error
    \value NotImplemented
        5.01 Not Implemented
    \value BadGateway
        5.02 Bad Gateway
    \value ServiceUnavailabl
        5.03 Service Unavailabl
    \value GatewayTimeout
        5.04 Gateway Timeout
    \value ProxyingNotSupported
        5.05 Proxying Not Supported

*/

/*! \enum CoapPdu::ContentType

    The CoAP content types.

    \value TextPlain
    \value ApplicationLink
    \value ApplicationXml
    \value ApplicationOctet
    \value ApplicationExi
    \value ApplicationJson
*/

/*! \enum CoapPdu::Error

    \value NoError
    \value InvalidTokenError
    \value InvalidPduSizeError
    \value InvalidOptionDeltaError
    \value InvalidOptionLengthError
    \value UnknownOptionError
*/


/*! \fn void CoapPdu::getStatusCodeString(const StatusCode &statusCode);
    Returns the human readable status code for the given \a statusCode.
*/

/*! \fn void CoapPdu::setContentType(const ContentType &contentType);
    Sets the content type of this \l{CoapPdu} to the given \a contentType.

    \sa CoapPdu::ContentType
*/

/*! \fn void CoapPdu::setMessageType(const MessageType &messageType);
    Sets the message type of this \l{CoapPdu} to the given \a messageType.

    \sa CoapPdu::MessageType
*/

/*! \fn void CoapPdu::setStatusCode(const StatusCode &statusCode);
    Sets the message type of this \l{CoapPdu} to the given \a statusCode.

    \sa CoapPdu::StatusCode
*/

#include "coappdu.h"
#include "coapoption.h"

#include <QMetaEnum>
#include <QTime>
#include <QLoggingCategory>
#include <QDataStream>

Q_DECLARE_LOGGING_CATEGORY(dcCoap)

/*! Constructs a CoapPdu with the given \a parent. */
CoapPdu::CoapPdu(QObject *parent) :
    QObject(parent),
    m_version(1),
    m_messageType(Confirmable),
    m_reqRspCode(Empty),
    m_messageId(0),
    m_contentType(TextPlain),
    m_payload(QByteArray()),
    m_error(NoError)
{
    std::srand(QDateTime::currentMSecsSinceEpoch());
}

/*! Constructs a CoapPdu from the given \a data with the given \a parent. */
CoapPdu::CoapPdu(const QByteArray &data, QObject *parent) :
    QObject(parent),
    m_version(1),
    m_messageType(Confirmable),
    m_reqRspCode(Empty),
    m_messageId(0),
    m_contentType(TextPlain),
    m_payload(QByteArray()),
    m_error(NoError)
{
    std::srand(QDateTime::currentMSecsSinceEpoch());
    unpack(data);
}

/*! Returns the human readable string for the given \a reqRspCode. */
QString CoapPdu::getReqRspCodeString(CoapPdu::ReqRspCode reqRspCode)
{
    QString statusCodeString;
    const QMetaObject &metaObject = CoapPdu::staticMetaObject;
    QMetaEnum statusCodeEnum = metaObject.enumerator(metaObject.indexOfEnumerator("ReqRspCode"));
    int classNumber = (reqRspCode & 0xE0) >> 5;
    int detailNumber = reqRspCode & 0x1F;
    statusCodeString.append(QString::number(classNumber) + ".");
    if (detailNumber < 10)
        statusCodeString.append("0");

    statusCodeString.append(QString::number(detailNumber) + " ");
    statusCodeString.append(statusCodeEnum.valueToKey(reqRspCode));
    return statusCodeString;
}

/*! Returns the version of this \l{CoapPdu}. */
quint8 CoapPdu::version() const
{
    return m_version;
}

/*! Sets the version of this \l{CoapPdu} to the given \a version. */
void CoapPdu::setVersion(quint8 version)
{
    m_version = version;
}

/*! Returns the \l{CoapPdu::MessageType} of this \l{CoapPdu}. */
CoapPdu::MessageType CoapPdu::messageType() const
{
    return m_messageType;
}

/*! Sets the \l{CoapPdu::MessageType} of this \l{CoapPdu} to the given \a messageType. */
void CoapPdu::setMessageType(CoapPdu::MessageType messageType)
{
    m_messageType = messageType;
}

/*! Returns the \l{CoapPdu::ReqRspCode} of this \l{CoapPdu}. */
CoapPdu::ReqRspCode CoapPdu::reqRspCode() const
{
    return m_reqRspCode;
}

/*! Sets the \l{CoapPdu::ReqRspCode} of this \l{CoapPdu} to the given \a reqRspCode. */
void CoapPdu::setReqRspCode(CoapPdu::ReqRspCode reqRspCode)
{
    m_reqRspCode = reqRspCode;
}

/*! Returns the messageId of this \l{CoapPdu}. */
quint16 CoapPdu::messageId() const
{
    return m_messageId;
}

/*! Creates a random message id for this \l{CoapPdu} and sets the
    message id to the created value.

    \sa setMessageId()
*/
void CoapPdu::createMessageId()
{
    setMessageId((quint16)std::rand() % 65536);
}

/*! Sets the messageId of this \l{CoapPdu} to the given \a messageId. */
void CoapPdu::setMessageId(quint16 messageId)
{
    m_messageId = messageId;
}

/*! Returns the \l{CoapPdu::ContentType} of this \l{CoapPdu}. */
CoapPdu::ContentType CoapPdu::contentType() const
{
    return m_contentType;
}

/*! Sets the content type of this \l{CoapPdu} to the given \a contentType.

    \sa CoapPdu::ContentType
*/
void CoapPdu::setContentType(ContentType contentType)
{
    m_contentType = contentType;
}

/*! Returns the token of this \l{CoapPdu}. */
QByteArray CoapPdu::token() const
{
    return m_token;
}
/*! Creates a random token for this \l{CoapPdu} and sets the
    token to the created value.

    \sa setToken()
*/
void CoapPdu::createToken()
{
    m_token.clear();
    // make sure that the toke has a minimum size of 1
    quint8 length = (quint8)(std::rand() % 7) + 1;
    for (int i = 0; i < length; i++) {
        m_token.append((char)std::rand() % 256);
    }
}

/*! Sets the token of this \l{CoapPdu} to the given \a token. */
void CoapPdu::setToken(const QByteArray &token)
{
    m_token = token;
}

/*! Returns the payload of this \l{CoapPdu}. */
QByteArray CoapPdu::payload() const
{
    return m_payload;
}

/*! Sets the payload of this \l{CoapPdu} to the given \a payload. */
void CoapPdu::setPayload(const QByteArray &payload)
{
    m_payload = payload;
}

/*! Returns the list of \l{CoapOption}{CoapOptions} of this \l{CoapPdu}. */
QList<CoapOption> CoapPdu::options() const
{
    return m_options;
}


/*! Adds the given \a option with the given \a data to this \l{CoapPdu}.

    \sa CoapOption
*/
void CoapPdu::addOption(CoapOption::Option option, const QByteArray &data)
{
    // set pdu data from the option
    switch (option) {
    case CoapOption::ContentFormat: {
        if (data.isEmpty()) {
            setContentType(TextPlain);
        } else {
            setContentType(static_cast<ContentType>(data.toHex().toInt(0, 16)));
        }
        break;
    }
    case CoapOption::Block1: {
        m_block = CoapPduBlock(data);
        break;
    }
    case CoapOption::Block2: {
        m_block = CoapPduBlock(data);
        break;
    }
    default:
        break;
    }

    // insert option (keep the list sorted to ensure a positiv option delta)
    int index = 0;
    for (int i = 0; i < m_options.length(); i ++) {
        index = i;
        if (m_options.at(i).option() <= option) {
            continue;
        } else {
            break;
        }
    }

    CoapOption o;
    o.setOption(option);
    o.setData(data);
    m_options.insert(qMin(m_options.length(), index + 1), o);
}

/*! Returns the block of this \l{CoapPdu}. */
CoapPduBlock CoapPdu::block() const
{
    return m_block;
}

/*! Returns true if this \l{CoapPdu} has the given \a option. */
bool CoapPdu::hasOption(CoapOption::Option option) const
{
    foreach (const CoapOption &o, m_options) {
        if (o.option() == option)
            return true;
    }
    return false;
}

CoapOption CoapPdu::option(CoapOption::Option option) const
{
    foreach (const CoapOption &o, m_options) {
        if (o.option() == option) {
            return o;
        }
    }
    return CoapOption();
}

/*! Resets this \l{CoapPdu} to the default values. */
void CoapPdu::clear()
{
    m_version = 1;
    m_messageType = Confirmable;
    m_reqRspCode = Empty;
    m_messageId = 0;
    m_contentType = TextPlain;
    m_token.clear();
    m_payload.clear();
    m_options.clear();
    m_error = NoError;
}

/*! Returns true if this \l{CoapPdu} has no errors. */
bool CoapPdu::isValid() const
{
    return (m_error == NoError);
}

/*! Returns the packed \l{CoapPdu} as byte array which are ready to send to the server.*/
QByteArray CoapPdu::pack() const
{
    QByteArray pduData;

    // header
    QByteArray header;
    header.resize(4);
    header.fill('0');
    quint8 *rawHeader = (quint8 *)header.data();
    rawHeader[0] = m_version << 6;
    rawHeader[0] |= (quint8)m_messageType << 4;
    rawHeader[0] |= (quint8)m_token.size();
    rawHeader[1] = (quint8)m_reqRspCode;
    rawHeader[2] = (quint8)(m_messageId >> 8);
    rawHeader[3] = (quint8)(m_messageId & 0xff);
    pduData = QByteArray::fromRawData((char *)rawHeader, 4);

    // token
    pduData.append(m_token);

    // options
    QByteArray optionsData;
    quint8 prevOption = 0;
    foreach (const CoapOption &option, m_options) {
        quint8 optionByte = 0;

        // encode option delta
        quint16 optionDelta = (quint8)option.option() - prevOption;
        prevOption = (quint8)option.option();

        quint8 extendedOptionDeltaByte = 0;
        quint16 bigExtendedOptionDeltaByte = 0;
        if (optionDelta < 13) {
            optionByte = optionDelta << 4;
        } else if (optionDelta < 270) {
            // extended 8 bit option delta
            optionByte = 13 << 4;
            extendedOptionDeltaByte = optionDelta - 13;
        } else {
            // extended 16 bit option delta
            optionByte = 14 << 4;
            bigExtendedOptionDeltaByte = ((optionDelta - 269) >> 8) & 0xff;
            bigExtendedOptionDeltaByte = (optionDelta - 269) & 0xff;
        }

        // encode option length
        int optionLength = option.data().length();
        quint8 extendedOptionLengthByte = 0;
        quint16 bigExtendedOptionLengthByte = 0;
        if (optionLength < 13) {
            optionByte |= optionLength;
        } else if (optionLength < 270) {
            // extended 8 bit option length
            optionByte |= 13;
            extendedOptionLengthByte = optionLength - 13;
        } else {
            // extended 16 bit option length
            optionByte |= 14;
            bigExtendedOptionLengthByte = ((optionLength - 269) >> 8) & 0xff;
            bigExtendedOptionLengthByte = (optionLength - 269) & 0xff;
        }

        // add obligatory option byte
        pduData.append((char)optionByte);

        // check extended option delta bytes
        if (extendedOptionDeltaByte != 0)
            pduData.append((char)extendedOptionDeltaByte);

        if (bigExtendedOptionDeltaByte != 0)
            pduData.append((char)bigExtendedOptionDeltaByte);

        // check extended option length bytes
        if (extendedOptionLengthByte != 0)
            pduData.append((char)extendedOptionLengthByte);

        if (bigExtendedOptionLengthByte != 0)
            pduData.append((char)extendedOptionLengthByte);

        // add the option data
        pduData.append(option.data());
    }
    pduData.append(optionsData);

    if (!m_payload.isEmpty()) {
        pduData.append((char)255);
        pduData.append(m_payload.data());
    }

    return pduData;
}

void CoapPdu::unpack(const QByteArray &data)
{
    if (data.length() < 4) {
        m_error = InvalidPduSizeError;
    }

    QDataStream stream(data);
    quint8 flags;
    stream >> flags;

    setVersion((flags & 0xc0) >> 6);
//    qCDebug(dcCoap()) << "Version:" << m_version;

    setMessageType(static_cast<MessageType>((flags & 0x30) >> 4));
//    qCDebug(dcCoap()) << "Message Type:" << messageType();

    quint8 tokenLength = flags & 0x0f;
//    qCDebug(dcCoap()) << "Token length:" << tokenLength;
    if (tokenLength > 8) {
        qCWarning(dcCoap()) << "Inavalid token length" << tokenLength;
        m_error = InvalidTokenError;
        return;
    }

    quint8 reqRspCode;
    stream >> reqRspCode;
    setReqRspCode(static_cast<ReqRspCode>(reqRspCode));
//    qCDebug(dcCoap()) << "Req/Rsp code:" << reqRspCode();

    quint16 messageId;
    stream >> messageId;
    setMessageId(messageId);
//    qCDebug(dcCoap()) << "Message ID:" << messageId;
    char tokenData[tokenLength];
    if (stream.readRawData(tokenData, tokenLength) != tokenLength) {
        qCWarning(dcCoap()) << "Token data not complete.";
        m_error = InvalidTokenError;
        return;
    }
    QByteArray token(tokenData, tokenLength);
    setToken(token);
//    qCDebug(dcCoap()) << "Token:" << token.toHex();


    while (!stream.atEnd()) {
        quint8 optionByte;
        stream >> optionByte;
//        qCDebug(dcCoap()) << "OptionByte:" << optionByte;

        if (optionByte == 0xff) {
            char payloadData[65507]; // Max UDP datagram size
            int payloadLength = stream.readRawData(payloadData, 65507);
            if (payloadLength > 0) {
                setPayload(QByteArray(payloadData, payloadLength));
            }
            return;
        }

        quint16 optionDelta;
        optionDelta = (optionByte & 0xf0) >> 4;
//        qCDebug(dcCoap()) << "Option delta:" << optionDelta;
        quint16 optionLength = (optionByte & 0x0f);
//        qCDebug(dcCoap()) << "Option length:" << optionLength;

        if (optionDelta == 13) {
            quint8 optionDeltaExtended;
            stream >> optionDeltaExtended;
            optionDelta = optionDeltaExtended + 13;
//            qCDebug(dcCoap()).nospace() << "Extended option delta (8 bit): " << optionDelta << " (" << optionDeltaExtended << " + 13)";
        } else if (optionDelta == 14) {
            quint16 optionDeltaExtended;
            stream >> optionDeltaExtended;
            optionDelta = optionDeltaExtended + 269;
//            qCDebug(dcCoap()).nospace() << "Extended option delta (16 bit): " << optionDelta << " (" << optionDeltaExtended << " + 269)";
        }

        if (optionLength == 13) {
            quint8 optionLengthExtended;
            stream >> optionLengthExtended;
            optionLength = optionLengthExtended + 13;
//            qCDebug(dcCoap()).nospace() << "Extended option length (8 bit): " << optionLength << " (" << optionLengthExtended << " + 13)";
        } else if (optionLength == 14) {
            quint16 optionLengthExtended;
            stream >> optionLengthExtended;
            optionLength = optionLengthExtended + 269;
//            qCDebug(dcCoap()).nospace() << "Extended option kength (16 bit): " << optionDelta << " (" << optionLengthExtended << " + 269)";
        }

        char optionData[optionLength];
        stream.readRawData(optionData, optionLength);
//        qCDebug(dcCoap()) << "Option data:" << QByteArray(optionData, optionLength);

        addOption(static_cast<CoapOption::Option>(optionDelta), QByteArray(optionData, optionLength));
    }
}

/*! Writes the data of the given \a coapPdu to \a dbg.

    \sa CoapPdu
*/
QDebug operator<<(QDebug debug, const CoapPdu &coapPdu)
{
    QDebugStateSaver saver(debug);
    const QMetaObject &metaObject = CoapPdu::staticMetaObject;
    QMetaEnum messageTypeEnum = metaObject.enumerator(metaObject.indexOfEnumerator("MessageType"));
    debug.nospace() << "CoapPdu(" << messageTypeEnum.valueToKey(coapPdu.messageType()) << ")" << Qt::endl;
    debug.nospace() << "  Code: " << CoapPdu::getReqRspCodeString(coapPdu.reqRspCode()) << Qt::endl;
    debug.nospace() << "  Ver: " << coapPdu.version() << Qt::endl;
    debug.nospace() << "  Token: " << coapPdu.token().length() << " " << "0x"+ coapPdu.token().toHex() << Qt::endl;
    debug.nospace() << "  Message ID: " << coapPdu.messageId() << Qt::endl;
    debug.nospace() << "  Payload size: " << coapPdu.payload().size() << Qt::endl;
    foreach (const CoapOption &option, coapPdu.options()) {
        debug.nospace() << "  " << option;
    }

    if (!coapPdu.payload().isEmpty())
        debug.nospace() << Qt::endl << coapPdu.payload() << Qt::endl;

    return debug;
}
