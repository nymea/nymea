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

#ifndef HTTPREPLY_H
#define HTTPREPLY_H

#include <QByteArray>
#include <QHash>

// Note: RFC 7231 HTTP/1.1 Semantics and Content -> http://tools.ietf.org/html/rfc7231

class HttpReply
{
public:

    enum HttpStatusCode {
        Ok                      = 200,
        Created                 = 201,
        Accepted                = 202,
        NoContent               = 204,
        Found                   = 302,
        BadRequest              = 400,
        Forbidden               = 403,
        NotFound                = 404,
        MethodNotAllowed        = 405,
        RequestTimeout          = 408,
        Conflict                = 409,
        InternalServerError     = 500,
        NotImplemented          = 501,
        BadGateway              = 502,
        ServiceUnavailable      = 503,
        GatewayTimeout          = 504,
        HttpVersionNotSupported = 505
    };

    enum HttpHeaderType {
        ContentTypeHeader,
        ContentLenghtHeader,
        ConnectionHeader,
        LocationHeader,
        UserAgentHeader,
        CacheControlHeader,
        AllowHeader,
        DateHeader,
        ServerHeader
    };

    explicit HttpReply(const HttpStatusCode &statusCode);

    void setHttpStatusCode(const HttpStatusCode &statusCode);
    HttpStatusCode httpStatusCode() const;

    void setPayload(const QByteArray &data);
    QByteArray payload() const;

    void setRawHeader(const QByteArray headerType, const QByteArray &value);
    void setHeader(const HttpHeaderType &headerType, const QByteArray &value);
    QHash<QByteArray, QByteArray> rawHeaderList() const;
    QByteArray rawHeader() const;

    bool isEmpty() const;
    void clear();

    QByteArray packReply();

private:
    HttpStatusCode m_statusCode;
    QByteArray m_rawHeader;
    QByteArray m_payload;

    QHash<QByteArray, QByteArray> m_rawHeaderList;

    QByteArray getHttpReasonPhrase(const HttpStatusCode &statusCode);
    QByteArray getHeaderType(const HttpHeaderType &headerType);

    QByteArray packHeader() const;
};

#endif // HTTPREPLY_H
