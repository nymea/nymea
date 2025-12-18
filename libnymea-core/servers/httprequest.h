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

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QUrlQuery>

namespace nymeaserver {

class HttpRequest
{
public:
    enum RequestMethod { Get, Post, Put, Delete, Options, Unhandled };

    HttpRequest();
    HttpRequest(QByteArray rawData);

    QByteArray rawHeader() const;
    QHash<QByteArray, QByteArray> rawHeaderList() const;

    RequestMethod method() const;
    QString methodString() const;
    QByteArray httpVersion() const;

    QUrl url() const;
    QUrlQuery urlQuery() const;

    QByteArray payload() const;

    bool isValid() const;
    bool isComplete() const;
    bool hasPayload() const;

    void appendData(const QByteArray &data);

private:
    QByteArray m_rawData;
    QByteArray m_rawHeader;
    QHash<QByteArray, QByteArray> m_rawHeaderList;

    RequestMethod m_method;
    QString m_methodString;
    QByteArray m_httpVersion;

    QUrl m_url;
    QUrlQuery m_urlQuery;

    QByteArray m_payload;

    bool m_valid;
    bool m_isComplete;

    void validate();
    RequestMethod getRequestMethodType(const QString &methodString);
};

QDebug operator<<(QDebug debug, const HttpRequest &httpRequest);

} // namespace nymeaserver
#endif // HTTPREQUEST_H
