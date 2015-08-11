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

#ifndef RESTRESOURCE_H
#define RESTRESOURCE_H

#include <QObject>
#include <QPair>

#include "httpreply.h"
#include "httprequest.h"

class QVariant;

namespace guhserver {

class RestResource : public QObject
{
    Q_OBJECT
public:
    explicit RestResource(QObject *parent = 0);
    virtual ~RestResource() = 0;

    virtual QString name() const = 0;

    virtual HttpReply *proccessRequest(const HttpRequest &request, const QStringList &urlTokens) = 0;

    static HttpReply *createSuccessReply();
    static HttpReply *createErrorReply(const HttpReply::HttpStatusCode &statusCode);
    static HttpReply *createAsyncReply();
    static QPair<bool, QVariant> verifyPayload(const QByteArray &payload);

private:
    virtual HttpReply *proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens);
    virtual HttpReply *proccessDeleteRequest(const HttpRequest &request, const QStringList &urlTokens);
    virtual HttpReply *proccessPutRequest(const HttpRequest &request, const QStringList &urlTokens);
    virtual HttpReply *proccessPostRequest(const HttpRequest &request, const QStringList &urlTokens);

    QHash<QPair<HttpRequest::RequestMethod, QString>, QString> m_descriptions;
    QHash<QString, QVariantMap> m_params;
    QHash<QString, QVariantMap> m_returns;

};

}

#endif // RESTRESOURCE_H
