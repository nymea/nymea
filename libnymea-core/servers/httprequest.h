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

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QByteArray>
#include <QUrlQuery>
#include <QString>
#include <QHash>

namespace nymeaserver {

class HttpRequest
{
public:
    enum RequestMethod {
        Get,
        Post,
        Put,
        Delete,
        Options,
        Unhandled
    };

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

QDebug operator<< (QDebug debug, const HttpRequest &httpRequest);

}
#endif // HTTPREQUEST_H
