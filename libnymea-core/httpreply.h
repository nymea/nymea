/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include <QObject>
#include <QByteArray>
#include <QHash>
#include <QTimer>
#include <QUuid>

// Note: RFC 7231 HTTP/1.1 Semantics and Content -> http://tools.ietf.org/html/rfc7231

namespace guhserver {

class HttpReply: public QObject
{
    Q_OBJECT
public:

    enum HttpStatusCode {
        Ok                      = 200,
        Created                 = 201,
        Accepted                = 202,
        NoContent               = 204,
        Found                   = 302,
        PermanentRedirect       = 308,
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

    enum Type {
        TypeSync,
        TypeAsync
    };

    HttpReply(QObject *parent = 0);
    HttpReply(const HttpStatusCode &statusCode = HttpStatusCode::Ok, const Type &type = TypeSync, QObject *parent = 0);

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

    QTimer *m_timer;
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

QDebug operator<< (QDebug debug, const HttpReply &httpReply);

}

#endif // HTTPREPLY_H
