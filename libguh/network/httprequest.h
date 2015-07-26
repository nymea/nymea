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

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QByteArray>
#include <QUrlQuery>
#include <QString>
#include <QHash>

class HttpRequest
{
public:
    enum RequestMethod {
        Get,
        Post,
        Put,
        Delete,
        Unhandled
    };

    explicit HttpRequest(QByteArray rawData);

    QByteArray rawHeader() const;
    QHash<QByteArray, QByteArray> rawHeaderList() const;

    RequestMethod method() const;
    QByteArray httpVersion() const;
    QUrlQuery urlQuery() const;

    QByteArray payload() const;

    bool isValid() const;
    bool hasPayload() const;

private:
    QByteArray m_rawData;
    QByteArray m_rawHeader;
    QHash<QByteArray, QByteArray> m_rawHeaderList;

    RequestMethod m_method;
    QByteArray m_httpVersion;
    QUrlQuery m_urlQuery;

    QByteArray m_payload;

    bool m_valid;

    RequestMethod getRequestMethodType(const QString &methodString);
};

QDebug operator<< (QDebug debug, const HttpRequest &httpRequest);

#endif // HTTPREQUEST_H
