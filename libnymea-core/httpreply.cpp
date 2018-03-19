/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
    setRawHeader("Keep-Alive", "timeout=12, max=50");
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
    setRawHeader("Keep-Alive", "timeout=12, max=50");
    packReply();
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
    m_data = m_rawHeader.append(m_payload);
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
    switch (statusCode) {
    case HttpStatusCode::Ok:
        return "Ok";
    case Created:
        return "Created";
    case Accepted:
        return "Accepted";
    case NoContent:
        return "No Content";
    case Found:
        return "Found";
    case PermanentRedirect:
        return "Permanent Redirect";
    case BadRequest:
        return "Bad Request";
    case Forbidden:
        return "Forbidden";
    case NotFound:
        return "NotFound";
    case MethodNotAllowed:
        return "Method Not Allowed";
    case RequestTimeout:
        return "Request Timeout";
    case Conflict:
        return "Conflict";
    case InternalServerError:
        return "Internal Server Error";
    case NotImplemented:
        return "Not Implemented";
    case BadGateway:
        return "Bad Gateway";
    case ServiceUnavailable:
        return "Service Unavailable";
    case GatewayTimeout:
        return "Gateway Timeout";
    case HttpVersionNotSupported:
        return "HTTP Version Not Supported";
    default:
        return QByteArray();
    }
}

QByteArray HttpReply::getHeaderType(const HttpReply::HttpHeaderType &headerType)
{
    switch (headerType) {
    case ContentTypeHeader:
        return "Content-Type";
    case ContentLenghtHeader:
        return "Content-Length";
    case CacheControlHeader:
        return "Cache-Control";
    case LocationHeader:
        return "Location";
    case ConnectionHeader:
        return "Connection";
    case UserAgentHeader:
        return "User-Agent";
    case AllowHeader:
        return "Allow";
    case DateHeader:
        return "Date";
    case ServerHeader:
        return "Server";
    default:
        return QByteArray();
    }
}

/*! Starts the timer for an async \l{HttpReply}.
 *
 *  \sa finished()
 */
void HttpReply::startWait()
{
    m_timer->start(10000);
}

void HttpReply::timeout()
{
    qCDebug(dcWebServer) << "Http reply timeout";
    m_timedOut = true;
    emit finished();
}

QDebug operator<<(QDebug debug, const HttpReply &httpReply)
{
    debug << "-----------------------------------" << "\n";
    debug << httpReply.rawHeader() << "\n";
    debug << "-----------------------------------" << "\n";
    debug << httpReply.payload() << "\n";
    debug << "-----------------------------------" << "\n";
    return debug;
}

}
