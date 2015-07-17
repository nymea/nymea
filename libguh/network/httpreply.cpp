/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
  \class HttpReply
  \brief Represents a reply of the guh webserver.

  \ingroup types
  \inmodule libguh

  This class holds the header and the payload data of a network reply and represents a response
  from the guh webserver.

  \note RFC 7231 HTTP/1.1 Semantics and Content -> \l{http://tools.ietf.org/html/rfc7231}{http://tools.ietf.org/html/rfc7231}


*/

/*! \enum HttpReply::HttpStatusCode

    This enum type specifies the status code of a HTTP webserver reply.

    \value Ok
        The request was understood and everything is Ok.
    \value Created
        ...
    \value Accepted
        ...
    \value NoContent
        ...
    \value Found
        ...
    \value BadRequest
        ...
    \value Forbidden
        ...
    \value NotFound
        ...
    \value MethodNotAllowed
        ...
    \value RequestTimeout
        ...
    \value Conflict
        ...
    \value InternalServerError
        ...
    \value NotImplemented
        ...
    \value BadGateway
        ...
    \value ServiceUnavailable
        ...
    \value GatewayTimeout
        ...
    \value HttpVersionNotSupported
        ...
*/

/*! \enum HttpReply::HttpHeaderType

    This enum type specifies the known type of a header in a HTTP webserver reply.

    \value ContentTypeHeader
        The request was understood and everything is Ok.
    \value ContentLenghtHeader
        ...
    \value ConnectionHeader
        ...
    \value LocationHeader
        ...
    \value UserAgentHeader
        ...
    \value CacheControlHeader
        ...
    \value AllowHeader
        ...
    \value DateHeader
        ...
    \value ServerHeader
        ...
*/

#include "httpreply.h"

#include <QDateTime>
#include <QPair>

/*! Construct a HttpReply with the given \a statusCode. */
HttpReply::HttpReply(const HttpStatusCode &statusCode) :
    m_statusCode(statusCode),
    m_payload(QByteArray())
{
    // set known headers
    setHeader(HttpHeaderType::ServerHeader, "guh/" + QByteArray(GUH_VERSION_STRING));
    setHeader(HttpHeaderType::UserAgentHeader, "guh/" + QByteArray(REST_API_VERSION));
    setHeader(HttpHeaderType::DateHeader, QDateTime::currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss").toUtf8() + " GMT");
    setHeader(HttpHeaderType::CacheControlHeader, "no-cache");
}

/*! Set the \a statusCode for this \l{HttpReply}.*/
void HttpReply::setHttpStatusCode(const HttpReply::HttpStatusCode &statusCode)
{
    m_statusCode = statusCode;
}

/*! Returns the status code of this \l{HttpReply}.*/
HttpReply::HttpStatusCode HttpReply::httpStatusCode() const
{
    return m_statusCode;
}

/*! Set the payload of this \l{HttpReply} to the given \a data.*/
void HttpReply::setPayload(const QByteArray &data)
{
    m_payload = data;
    setHeader(HttpHeaderType::ContentLenghtHeader, QByteArray::number(data.length()));
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

/*! Returns true if the raw header and the payload of this \l{HttpReply} is empty.*/
bool HttpReply::isEmpty() const
{
    return m_rawHeader.isEmpty() && m_payload.isEmpty() && m_rawHeaderList.isEmpty();
}

/*! Clears all data of this \l{HttpReply}. */
void HttpReply::clear()
{
    m_rawHeader.clear();
    m_payload.clear();
    m_rawHeaderList.clear();
}

/*! Returns the whole reply data of this \l{HttpReply}. The data contain the HTTP header and the payload. */
QByteArray HttpReply::packReply()
{
    // set status code
    m_rawHeader.clear();
    m_rawHeader.append("HTTP/1.1 " + QByteArray::number(m_statusCode) + " " + getHttpReasonPhrase(m_statusCode) + "\r\n");

    // write header
    foreach (const QByteArray &headerName, m_rawHeaderList.keys()) {
        m_rawHeader.append(headerName + ": " + m_rawHeaderList.value(headerName) + "\r\n" );
    }

    m_rawHeader.append("\r\n");
    m_rawHeader.append(m_payload);
    return m_rawHeader;
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
