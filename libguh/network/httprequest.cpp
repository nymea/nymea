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

#include "httprequest.h"
#include "loggingcategories.h"

#include <QUrlQuery>


HttpRequest::HttpRequest(QByteArray rawData) :
    m_rawData(rawData),
    m_valid(false)
{
    // Parese the HTTP request. The request is invalid, until the end of the parse process.
    if (m_rawData.isEmpty())
        return;

    // split the data into header and payload
    int headerEndIndex = m_rawData.indexOf("\r\n\r\n");
    if (headerEndIndex < 0) {
        qCWarning(dcWebServer) << "Could not parse end of HTTP header (empty line between header and body):" << rawData;
        return;
    }
    m_rawHeader = m_rawData.left(headerEndIndex);
    m_payload = m_rawData.right(m_rawData.length() - headerEndIndex).simplified();

    // parse status line
    QStringList headerLines = QString(m_rawHeader).split(QRegExp("\r\n"));
    QString statusLine = headerLines.takeFirst();
    QStringList statusLineTokens = statusLine.split(QRegExp("[ \r\n][ \r\n]*"));
    if (statusLineTokens.count() != 3) {
        qCWarning(dcWebServer) << "Could not parse HTTP status line:" << statusLine;
        return;
    }

    m_method = statusLineTokens.at(0).toUtf8().simplified();
    m_urlQuery = QUrlQuery(statusLineTokens.at(1).simplified());
    m_httpVersion = statusLineTokens.at(2).toUtf8().simplified();

    if (!m_httpVersion.contains("HTTP")) {
        qCWarning(dcWebServer) << "Unknown HTTP version:" << m_httpVersion;
        return;
    }

    foreach (const QString &line, headerLines) {
        if (!line.contains(":")) {
            qCWarning(dcWebServer) << "Invalid HTTP header:" << line;
            return;
        }
        int index = line.indexOf(":");
        QByteArray key = line.left(index).toUtf8().simplified();
        QByteArray value = line.right(line.count() - index - 1).toUtf8().simplified();
        m_rawHeaderList.insert(key, value);
    }

    m_valid = true;
}

QByteArray HttpRequest::rawHeader() const
{
    return m_rawHeader;
}

QHash<QByteArray, QByteArray> HttpRequest::rawHeaderList() const
{
    return m_rawHeaderList;
}

QByteArray HttpRequest::method() const
{
    return m_method;
}

QByteArray HttpRequest::httpVersion() const
{
    return m_httpVersion;
}

QUrlQuery HttpRequest::urlQuery() const
{
    return m_urlQuery;
}

QByteArray HttpRequest::payload() const
{
    return m_payload;
}

bool HttpRequest::isValid() const
{
    return m_valid;
}

bool HttpRequest::hasPayload() const
{
    return !m_payload.isEmpty();
}

QDebug operator<<(QDebug debug, const HttpRequest &httpRequest)
{
    debug << "===================================" << "\n";
    debug << "  http version: " << httpRequest.httpVersion() << "\n";
    debug << "        method: " << httpRequest.method() << "\n";
    debug << "     URL query: " << httpRequest.urlQuery().query() << "\n";
    debug << "      is valid: " << httpRequest.isValid() << "\n";
    debug << "-----------------------------------" << "\n";
    debug << httpRequest.rawHeader() << "\n";
    debug << "-----------------------------------" << "\n";
    debug << httpRequest.payload() << "\n";
    debug << "-----------------------------------" << "\n";
    return debug;
}
