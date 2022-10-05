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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::HttpReply
    \brief Represents a reply of the nymea webserver to a \l{HttpRequest}.

    \ingroup api
    \inmodule core

    This class holds the header and the payload data of a network reply and represents a response
    from the nymea webserver to a \l{HttpRequest}.

    \note RFC 7231 HTTP/1.1 Semantics and Content -> \l{http://tools.ietf.org/html/rfc7231}{http://tools.ietf.org/html/rfc7231}
*/

/*! \enum nymeaserver::HttpReply::HttpStatusCode

    This enum type specifies the status code of a HTTP webserver reply.

    You can find more information here: \l{http://tools.ietf.org/html/rfc7231#section-6.1}{http://tools.ietf.org/html/rfc7231#section-6.1}

    \value Ok
        The request was understood and everything is Ok.
    \value Created
        The resource was created successfully.
    \value Accepted
        The resource was accepted.
    \value NoContent
        The request has no content but it was expected.
    \value Found
        The resource was found.
    \value PermanentRedirect
        The resource redirects permanent to given url.
    \value BadRequest
        The request was bad formatted. Also if a \l{Param} was not understood or the header is not correct.
    \value Forbidden
        The request tries to get access to a forbidden space.
    \value NotFound
        The requested resource could not be found.
    \value MethodNotAllowed
        The request method is not allowed. See "Allowed-Methods" header for the allowed methods.
    \value RequestTimeout
        The request method timed out. Default timeout = 5s.
    \value Conflict
        The request resource conflicts with an other.
    \value InternalServerError
        There was an internal server error.
    \value NotImplemented
        The requestet method call is not implemented.
    \value BadGateway
        The gateway is not correct.
    \value ServiceUnavailable
        The service is not available at the moment.
    \value GatewayTimeout
        The gateway timed out.
    \value HttpVersionNotSupported
        The HTTP version is not supported. The only supported version is HTTP/1.1.
*/

/*! \enum nymeaserver::HttpReply::HttpHeaderType
    This enum type specifies the known type of a header in a HTTP webserver reply.
    You can find more information here: \l{http://tools.ietf.org/html/rfc7231#section-5}

    \value ContentTypeHeader
        The content type header i.e. application/json.
    \value ContentLenghtHeader
        The length of the sent content.
    \value ConnectionHeader
        The connection header.
    \value LocationHeader
        The location header.
    \value UserAgentHeader
        The user agent of the client.
    \value CacheControlHeader
        The cache control header.
    \value AllowHeader
        The allowed methods header.
    \value DateHeader
        The server date header.
    \value ServerHeader
        The name of the server i.e. "Server: nymea/0.6.0"
*/

/*! \enum nymeaserver::HttpReply::Type

    This enum type describes the type of this \l{HttpReply}. There are two types:

    \value TypeSync
        The \l{HttpReply} can be responded imediatly.
    \value TypeAsync
        The \l{HttpReply} is asynchron and has to be responded later.
*/

/*! \fn void nymeaserver::HttpReply::finished();
    This signal is emitted when this async \l{HttpReply} is finished.
*/

/*! \fn nymeaserver::HttpReply::HttpReply(QObject *parent);
    Construct an empty \l{HttpReply} with the given \a parent.
*/

/*! \fn nymeaserver::HttpReply::HttpReply(const HttpStatusCode &statusCode, const Type &type, QObject *parent);
    Construct a \l{HttpReply} with the given \a statusCode, \a type and \a parent.
*/

/*! \fn void nymeaserver::HttpReply::setHttpStatusCode(const HttpStatusCode &statusCode);
    Set the \l{HttpStatusCode} of this \l{HttpReply} to the given \a statusCode.
*/

/*! \fn QDebug nymeaserver::operator<< (QDebug debug, const HttpReply &httpReply);
    Writes the given \l{HttpReply} \a httpReply to the given \a debug. This method gets used just for debugging.
*/



#include "httpreply.h"
#include "loggingcategories.h"
#include "nymeacore.h"
#include "version.h"

#include <QDateTime>
#include <QPair>
#include <QDebug>

namespace nymeaserver {

HttpReply::HttpReply(QObject *parent) :
    QObject(parent),
    m_statusCode(HttpReply::Ok),
    m_type(HttpReply::TypeSync),
    m_payload(QByteArray()),
    m_closeConnection(false),
    m_timedOut(false)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &HttpReply::timedOut);

    m_reasonPhrase = getHttpReasonPhrase(m_statusCode);

    // set known headers
    setHeader(HttpReply::ContentTypeHeader, "text/plain; charset=\"utf-8\";");
    setHeader(HttpHeaderType::ServerHeader, "nymea/" + QByteArray(NYMEA_VERSION_STRING));
    setHeader(HttpHeaderType::DateHeader, NymeaCore::instance()->timeManager()->currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss").toUtf8());
    setHeader(HttpHeaderType::CacheControlHeader, "no-cache");
    setHeader(HttpHeaderType::ConnectionHeader, "Keep-Alive");
    setRawHeader("Access-Control-Allow-Origin","*");
    setRawHeader("Keep-Alive", QString("timeout=%1, max=50").arg(m_timeout).toUtf8());
    packReply();
}

HttpReply::HttpReply(const HttpReply::HttpStatusCode &statusCode, const HttpReply::Type &type, QObject *parent):
    QObject(parent),
    m_statusCode(statusCode),
    m_type(type),
    m_payload(QByteArray()),
    m_timedOut(false)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &HttpReply::timeout);

    m_reasonPhrase = getHttpReasonPhrase(m_statusCode);

    // set known / default headers
    setHeader(HttpReply::ContentTypeHeader, "text/plain; charset=\"utf-8\";");
    setHeader(HttpHeaderType::ServerHeader, "nymea/" + QByteArray(NYMEA_VERSION_STRING));
    setHeader(HttpHeaderType::DateHeader, NymeaCore::instance()->timeManager()->currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss").toUtf8());
    setHeader(HttpHeaderType::CacheControlHeader, "no-cache");
    setHeader(HttpHeaderType::ConnectionHeader, "Keep-Alive");
    setRawHeader("Access-Control-Allow-Origin","*");
    setRawHeader("Keep-Alive", QString("timeout=%1, max=50").arg(m_timeout).toUtf8());
    packReply();
}

HttpReply *HttpReply::createSuccessReply()
{
    HttpReply *reply = new HttpReply(HttpReply::Ok, HttpReply::TypeSync);
    reply->setPayload("200 Ok");
    return reply;
}

HttpReply *HttpReply::createErrorReply(const HttpReply::HttpStatusCode &statusCode)
{
    HttpReply *reply = new HttpReply(statusCode, HttpReply::TypeSync);
    reply->setPayload(QByteArray::number(reply->httpStatusCode()) + " " + reply->httpReasonPhrase());
    return reply;
}

HttpReply *HttpReply::createAsyncReply()
{
    HttpReply *reply = new HttpReply(HttpReply::Ok, HttpReply::TypeAsync);
    reply->setPayload(QByteArray::number(reply->httpStatusCode()) + " " + reply->httpReasonPhrase());
    return reply;
}

void HttpReply::setHttpStatusCode(const HttpReply::HttpStatusCode &statusCode)
{
    m_statusCode = statusCode;
    m_reasonPhrase = getHttpReasonPhrase(m_statusCode);
    packReply();
}

/*! Returns the status code of this \l{HttpReply}.*/
HttpReply::HttpStatusCode HttpReply::httpStatusCode() const
{
    return m_statusCode;
}

/*! Returns the error code reason phrase for the current \l{HttpStatusCode}.*/
QByteArray HttpReply::httpReasonPhrase() const
{
    return m_reasonPhrase;
}

/*! Returns the type of this \l{HttpReply}.
 * \sa Type
 */
HttpReply::Type HttpReply::type() const
{
    return m_type;
}

/*! Set the \a clientId of this \l{HttpReply}.*/
void HttpReply::setClientId(const QUuid &clientId)
{
    m_clientId = clientId;
    packReply();
}

/*! Returns the clientId of this \l{HttpReply}.*/
QUuid HttpReply::clientId() const
{
    return m_clientId;
}

/*! Set the payload of this \l{HttpReply} to the given \a data.*/
void HttpReply::setPayload(const QByteArray &data)
{
    m_payload = data;
    setHeader(HttpHeaderType::ContentLenghtHeader, QByteArray::number(data.length()));
    packReply();
}

/*! Returns the payload of this \l{HttpReply}.*/
QByteArray HttpReply::payload() const
{
    return m_payload;
}

/*! This method appends a raw header to the header list of this \l{HttpReply}.
    The Header will be set to \a headerType : \a value.
*/
void HttpReply::setRawHeader(const QByteArray headerType, const QByteArray &value)
{
    // if the header is already set, overwrite it
    if (m_rawHeaderList.keys().contains(headerType)) {
        m_rawHeaderList.remove(headerType);
    }
    m_rawHeaderList.insert(headerType, value);
    packReply();
}

/*! This method appends a known header to the header list of this \l{HttpReply}.
    The Header will be set to \a headerType : \a value.
*/
void HttpReply::setHeader(const HttpHeaderType &headerType, const QByteArray &value)
{
    setRawHeader(getHeaderType(headerType), value);
}

/*! Returns the list of all set headers in this \l{HttpReply}.*/
QHash<QByteArray, QByteArray> HttpReply::rawHeaderList() const
{
    return m_rawHeaderList;
}

/*! Returns the raw headers of this \l{HttpReply}.
    \note The header will be empty until the method \l{packReply()} was called.
    \sa packReply()
*/
QByteArray HttpReply::rawHeader() const
{
    return m_rawHeader;
}

/*! Sets the \a close parameter of this \l{HttpReply}. If \a close is true,
    the connection of the client will be closed after this reply was sent.
*/
void HttpReply::setCloseConnection(const bool &close)
{
    m_closeConnection = close;
}

/*! Returns the connection close parameter of this \l{HttpReply}. If close is true, the connection
    of the client will be closed after this reply was sent.
*/
bool HttpReply::closeConnection() const
{
    return m_closeConnection;
}

/*! Returns true if the raw header and the payload of this \l{HttpReply} is empty.*/
bool HttpReply::isEmpty() const
{
    return m_rawHeader.isEmpty() && m_payload.isEmpty() && m_rawHeaderList.isEmpty();
}

/*! Clears all data of this \l{HttpReply}. */
void HttpReply::clear()
{
    m_closeConnection = false;
    m_type = TypeSync;
    m_statusCode = Ok;
    m_rawHeader.clear();
    m_payload.clear();
    m_rawHeaderList.clear();
}
/*! Packs the whole reply data of this \l{HttpReply}. The data can be accessed with \l{HttpReply::data()}.
    \sa data()
*/
void HttpReply::packReply()
{
    // set status code
    m_data.clear();
    m_rawHeader.clear();
    m_rawHeader.append("HTTP/1.1 " + QByteArray::number(m_statusCode) + " " + getHttpReasonPhrase(m_statusCode) + "\r\n");

    // write header
    foreach (const QByteArray &headerName, m_rawHeaderList.keys()) {
        m_rawHeader.append(headerName + ": " + m_rawHeaderList.value(headerName) + "\r\n" );
    }

    m_rawHeader.append("\r\n");
    m_data = QByteArray(m_rawHeader).append(m_payload);
}

/*! Returns the current raw data (header + payload) of this \l{HttpReply}.*/
QByteArray HttpReply::data() const
{
    return m_data;
}

/*! Return true if the response took to long for the request.*/
bool HttpReply::timedOut() const
{
    return m_timedOut;
}

QByteArray HttpReply::getHttpReasonPhrase(const HttpReply::HttpStatusCode &statusCode)
{
    QByteArray response;
    switch (statusCode) {
    case HttpStatusCode::Ok:
        response = QString("Ok").toUtf8();
        break;
    case Created:
        response = QString("Created").toUtf8();
        break;
    case Accepted:
        response = QString("Accepted").toUtf8();
        break;
    case NoContent:
        response = QString("No Content").toUtf8();
        break;
    case Found:
        response = QString("Found").toUtf8();
        break;
    case PermanentRedirect:
        response = QString("Permanent Redirect").toUtf8();
        break;
    case BadRequest:
        response = QString("Bad Request").toUtf8();
        break;
    case Forbidden:
        response = QString("Forbidden").toUtf8();
        break;
    case NotFound:
        response = QString("NotFound").toUtf8();
        break;
    case MethodNotAllowed:
        response = QString("Method Not Allowed").toUtf8();
        break;
    case RequestTimeout:
        response = QString("Request Timeout").toUtf8();
        break;
    case Conflict:
        response = QString("Conflict").toUtf8();
        break;
    case InternalServerError:
        response = QString("Internal Server Error").toUtf8();
        break;
    case NotImplemented:
        response = QString("Not Implemented").toUtf8();
        break;
    case BadGateway:
        response = QString("Bad Gateway").toUtf8();
        break;
    case ServiceUnavailable:
        response = QString("Service Unavailable").toUtf8();
        break;
    case GatewayTimeout:
        response = QString("Gateway Timeout").toUtf8();
        break;
    case HttpVersionNotSupported:
        response = QString("HTTP Version Not Supported").toUtf8();
        break;
    }

    return response;
}

QByteArray HttpReply::getHeaderType(const HttpReply::HttpHeaderType &headerType)
{
    QByteArray header;
    switch (headerType) {
    case ContentTypeHeader:
        header = QString("Content-Type").toUtf8();
        break;
    case ContentLenghtHeader:
        header = QString("Content-Length").toUtf8();
        break;
    case CacheControlHeader:
        header = QString("Cache-Control").toUtf8();
        break;
    case LocationHeader:
        header = QString("Location").toUtf8();
        break;
    case ConnectionHeader:
        header = QString("Connection").toUtf8();
        break;
    case UserAgentHeader:
        header = QString("User-Agent").toUtf8();
        break;
    case AllowHeader:
        header = QString("Allow").toUtf8();
        break;
    case DateHeader:
        header = QString("Date").toUtf8();
        break;
    case ServerHeader:
        header = QString("Server").toUtf8();
        break;
    }

    return header;
}

/*! Starts the timer for an async \l{HttpReply}.
 *
 *  \sa finished()
 */
void HttpReply::startWait()
{
    m_timer->start(m_timeout);
}

void HttpReply::timeout()
{
    qCDebug(dcWebServer()) << "Http reply timeout";
    m_timedOut = true;
    emit finished();
}

QDebug operator<<(QDebug debug, HttpReply *httpReply)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "HttpReply(" << httpReply->clientId().toString() << ")" << endl;
    debug << qUtf8Printable(httpReply->rawHeader());
    debug << qUtf8Printable(httpReply->payload());
    return debug;
}

}
