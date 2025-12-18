// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef HTTPREPLY_H
#define HTTPREPLY_H

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QTimer>
#include <QUuid>

// Note: RFC 7231 HTTP/1.1 Semantics and Content -> http://tools.ietf.org/html/rfc7231

namespace nymeaserver {

class HttpReply : public QObject
{
    Q_OBJECT
public:
    enum HttpStatusCode {
        Ok = 200,
        Created = 201,
        Accepted = 202,
        NoContent = 204,
        Found = 302,
        PermanentRedirect = 308,
        BadRequest = 400,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        RequestTimeout = 408,
        Conflict = 409,
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503,
        GatewayTimeout = 504,
        HttpVersionNotSupported = 505
    };

    enum HttpHeaderType { ContentTypeHeader, ContentLenghtHeader, ConnectionHeader, LocationHeader, UserAgentHeader, CacheControlHeader, AllowHeader, DateHeader, ServerHeader };

    enum Type { TypeSync, TypeAsync };

    HttpReply(QObject *parent = nullptr);
    HttpReply(const HttpStatusCode &statusCode = HttpStatusCode::Ok, const Type &type = TypeSync, QObject *parent = nullptr);

    static HttpReply *createSuccessReply();
    static HttpReply *createErrorReply(const HttpReply::HttpStatusCode &statusCode);
    static HttpReply *createAsyncReply();

    void setHttpStatusCode(const HttpStatusCode &statusCode);
    HttpStatusCode httpStatusCode() const;

    QByteArray httpReasonPhrase() const;

    Type type() const;

    void setClientId(const QUuid &clientId);
    QUuid clientId() const;

    void setPayload(const QByteArray &data);
    QByteArray payload() const;

    void setRawHeader(const QByteArray headerType, const QByteArray &value);
    void setHeader(const HttpHeaderType &headerType, const QByteArray &value);
    QHash<QByteArray, QByteArray> rawHeaderList() const;
    QByteArray rawHeader() const;

    void setCloseConnection(const bool &close);
    bool closeConnection() const;

    bool isEmpty() const;
    void clear();

    void packReply();

    QByteArray data() const;

    bool timedOut() const;

private:
    HttpStatusCode m_statusCode;
    QByteArray m_reasonPhrase;
    Type m_type;
    QUuid m_clientId;

    QByteArray m_rawHeader;
    QByteArray m_payload;
    QByteArray m_data;

    QHash<QByteArray, QByteArray> m_rawHeaderList;

    bool m_closeConnection;

    QTimer *m_timer = nullptr;
    int m_timeout = 60000;
    bool m_timedOut;

    QByteArray getHttpReasonPhrase(const HttpStatusCode &statusCode);
    QByteArray getHeaderType(const HttpHeaderType &headerType);

private slots:
    void timeout();

public slots:
    void startWait();

signals:
    void finished();
};

QDebug operator<<(QDebug debug, HttpReply *httpReply);

} // namespace nymeaserver

#endif // HTTPREPLY_H
