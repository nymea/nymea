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

#include "httpreply.h"

#include <QDateTime>
#include <QPair>

// Note: RFC 7231 HTTP/1.1 Semantics and Content -> http://tools.ietf.org/html/rfc7231

HttpReply::HttpReply(const HttpStatusCode &statusCode) :
    m_statusCode(statusCode),
    m_payload(QByteArray())
{
    // set known headers
    //setHeader(HeaderType::ContentTypeHeader, "application/x-www-form-urlencoded; charset=\"utf-8\"");
    setHeader(HeaderType::ServerHeader, "guh/" + QByteArray(GUH_VERSION_STRING));
    setHeader(HeaderType::UserAgentHeader, "guh/" + QByteArray(REST_API_VERSION));
    setHeader(HeaderType::DateHeader, QDateTime::currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss").toUtf8() + " GMT");
    setHeader(HeaderType::CacheControlHeader, "no-cache");
}

void HttpReply::setHttpStatusCode(const HttpReply::HttpStatusCode &statusCode)
{
    m_statusCode = statusCode;
}

HttpReply::HttpStatusCode HttpReply::httpStatusCode() const
{
    return m_statusCode;
}

void HttpReply::setPayload(const QByteArray &data)
{
    m_payload = data;
    setHeader(HeaderType::ContentLenghtHeader, QByteArray::number(data.length()));
}

QByteArray HttpReply::payload() const
{
    return m_payload;
}

void HttpReply::setRawHeader(const QByteArray headerType, const QByteArray &value)
{
    // if the header is already set, overwrite it
    if (m_rawHeaderList.keys().contains(headerType)) {
        m_rawHeaderList.remove(headerType);
    }

    m_rawHeaderList.insert(headerType, value);
}

void HttpReply::setHeader(const HttpReply::HeaderType &headerType, const QByteArray &value)
{
    setRawHeader(getHeaderType(headerType), value);
}

QHash<QByteArray, QByteArray> HttpReply::rawHeaderList() const
{
    return m_rawHeaderList;
}

QByteArray HttpReply::rawHeader() const
{
    return m_rawHeader;
}

bool HttpReply::isValid() const
{
    // TODO: verify if header is valid and payload is valid
    return true;
}

bool HttpReply::isEmpty() const
{
    return m_rawHeader.isEmpty() && m_payload.isEmpty() && m_rawHeaderList.isEmpty();
}

void HttpReply::clear()
{
    m_rawHeader.clear();
    m_payload.clear();
    m_rawHeaderList.clear();
}

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

QByteArray HttpReply::getHeaderType(const HttpReply::HeaderType &headerType)
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
