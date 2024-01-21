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
    \class CoapReply
    \brief Represents a reply of a CoAP request.

    \ingroup coap-group
    \inmodule libnymea

    The CoapReply class contains the data and headers for a request sent with \l{Coap} client.

    \note Please don't forget to delete the reply once it is finished.

    \section2 Example

    \code
        Coap *coap = new Coap(this);
        connect(coap, SIGNAL(replyFinished(CoapReply*)), this, SLOT(onReplyFinished(CoapReply*)));

        CoapRequest request(QUrl("coap://example.com/"));

        CoapReply *reply = coap->ping(request);
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

    \sa Coap, CoapRequest

*/

/*! \fn void CoapReply::timeout();
    This signal is emitted when the reply took to long.
*/

/*! \fn void CoapReply::finished();
    This signal is emitted when the reply is finished.
*/

/*! \fn void CoapReply::error(const Error &code);
    This signal is emitted when an error occurred. The given \a code represents the \l{CoapReply::Error}.

    \sa error(), errorString()
*/

/*! \enum CoapReply::Error

    \value NoError
        No error occurred. Everything ok.
    \value HostNotFoundError
        The remote host name was not found (invalid hostname).
    \value TimeoutError
        The server did not respond after 4 retransmissions.
    \value InvalidUrlSchemeError
        The given URL does not have a valid scheme.
    \value InvalidPduError
        The package data unit (PDU) could not be parsed successfully.
*/


#include "coapreply.h"
#include "coappdu.h"

#include <QMetaEnum>

/*! Returns the request for this \l{CoapReply}. */
CoapRequest CoapReply::request() const
{
    return m_request;
}

/*! Returns the payload of this \l{CoapReply}. The payload will be available once the \l{CoapReply} is finished.

    \sa isFinished
*/
QByteArray CoapReply::payload() const
{
    return m_payload;
}

/*! Returns true if the \l{CoapReply} is finished.

    \sa finished()
*/
bool CoapReply::isFinished() const
{
    return m_isFinished;
}

/*! Returns true if the \l{CoapReply} is running.

    \sa finished()
*/
bool CoapReply::isRunning() const
{
    return m_timer->isActive();
}

/*! Returns error \l{CoapReply::Error} of the \l{CoapReply}.

    \sa errorString()
*/
CoapReply::Error CoapReply::error() const
{
    return m_error;
}

/*! Returns error string of the \l{CoapReply}.

    \sa error()
*/
QString CoapReply::errorString() const
{
    QString errorString;
    switch (m_error) {
    case NoError:
        break;
    case HostNotFoundError:
        errorString = "The remote host name was not found (invalid hostname).";
        break;
    case TimeoutError:
        errorString = "The server did not respond after 4 retransmissions.";
        break;
    case InvalidUrlSchemeError:
        errorString = "The given URL does not have a valid scheme.";
        break;
    case InvalidPduError:
        errorString = "The package data unit (PDU) could not be parsed successfully.";
        break;
    default:
        break;
    }
    return errorString;
}

/*! Returns the \l{CoapPdu::ContentType} of this \l{CoapReply}. */
CoapPdu::ContentType CoapReply::contentType() const
{
    return m_contentType;
}

/*! Returns the \l{CoapPdu::MessageType} of this \l{CoapReply}. */
CoapPdu::MessageType CoapReply::messageType() const
{
    return m_messageType;
}

/*! Returns the \l{CoapPdu::ReqRspCode} of this \l{CoapReply}. */
CoapPdu::ReqRspCode CoapReply::reqRspCode() const
{
    return m_reqRspCode;
}

CoapReply::CoapReply(const CoapRequest &request, QObject *parent) :
    QObject(parent),
    m_request(request),
    m_error(NoError),
    m_isFinished(false),
    m_retransmissions(1),
    m_contentType(CoapPdu::TextPlain),
    m_messageType(CoapPdu::Acknowledgement),
    m_reqRspCode(CoapPdu::Empty),
    m_lockedUp(false)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->setInterval(2000);

    connect(m_timer, &QTimer::timeout, this, &CoapReply::timeout);
}

QByteArray CoapReply::requestData() const
{
    return m_requestData;
}

int CoapReply::messageId() const
{
    return m_messageId;
}

void CoapReply::setMessageId(const int &messageId)
{
    m_messageId = messageId;
}

QByteArray CoapReply::messageToken() const
{
    return m_messageToken;
}

void CoapReply::setMessageToken(const QByteArray &messageToken)
{
    m_messageToken = messageToken;
}

bool CoapReply::observation() const
{
    return m_observation;
}

void CoapReply::setObservation(const bool &observation)
{
    m_observation = observation;
}

bool CoapReply::observationEnable() const
{
    return m_observationEnable;
}

void CoapReply::setObservationEnable(const bool &observationEnable)
{
    m_observationEnable = observationEnable;
}

void CoapReply::setFinished()
{
    m_isFinished = true;
    m_timer->stop();
    emit finished();
}

void CoapReply::setError(const CoapReply::Error &code)
{
    m_error = code;
    emit error(m_error);
}

void CoapReply::resend()
{
    m_retransmissions++;
    if (m_retransmissions > 5) {
        setError(CoapReply::TimeoutError);
        setFinished();
    }
}

void CoapReply::setContentType(CoapPdu::ContentType contentType)
{
    m_contentType = contentType;
}

void CoapReply::setMessageType(CoapPdu::MessageType messageType)
{
    m_messageType = messageType;
}

void CoapReply::setReqRspCode(CoapPdu::ReqRspCode reqRspCode)
{
    m_reqRspCode = reqRspCode;
}

void CoapReply::setHostAddress(const QHostAddress &address)
{
    m_hostAddress = address;
}

QHostAddress CoapReply::hostAddress() const
{
    return m_hostAddress;
}

void CoapReply::setPort(int port)
{
    m_port = port;
}

int CoapReply::port() const
{
    return m_port;
}

void CoapReply::setRequestPayload(const QByteArray &requestPayload)
{
    m_requestPayload = requestPayload;
}

QByteArray CoapReply::requestPayload() const
{
    return m_requestPayload;
}

void CoapReply::setRequestMethod(CoapPdu::ReqRspCode method)
{
    m_requestMethod = method;
}

CoapPdu::ReqRspCode CoapReply::requestMethod() const
{
    return m_requestMethod;
}

void CoapReply::appendPayloadData(const QByteArray &data)
{
    m_payload.append(data);
    m_timer->start();
    m_retransmissions = 1;
}

void CoapReply::setRequestData(const QByteArray &requestData)
{
    m_requestData = requestData;
}

/*! Writes the data of the given \a reply to \a dbg.

    \sa CoapReply
*/
QDebug operator<<(QDebug debug, CoapReply *reply)
{
    QDebugStateSaver saver(debug);
    const QMetaObject &metaObject = CoapPdu::staticMetaObject;
    QMetaEnum messageTypeEnum = metaObject.enumerator(metaObject.indexOfEnumerator("MessageType"));
    QMetaEnum contentTypeEnum = metaObject.enumerator(metaObject.indexOfEnumerator("ContentType"));
    debug.nospace() << "CoapReply(" << messageTypeEnum.valueToKey(reply->messageType()) << ")" << '\n';
    debug.nospace() << "  Status code: " << CoapPdu::getReqRspCodeString(reply->reqRspCode()) << '\n';
    debug.nospace() << "  Content type: " << contentTypeEnum.valueToKey(reply->contentType()) << '\n';
    debug.nospace() << "  Payload size: " << reply->payload().size() << '\n';

    if (!reply->payload().isEmpty())
        debug.nospace() << '\n' << reply->payload() << '\n';

    return debug;
}
