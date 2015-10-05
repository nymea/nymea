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
    \class guhserver::LogsResource
    \brief This subclass of \l{RestResource} processes the REST requests for the \tt Logs namespace.

    \ingroup json
    \inmodule core

    This \l{RestResource} will be created in the \l{RestServer} and used to handle REST requests
    for the \tt {Logs} namespace of the API.

    \code
        http://localhost:3333/api/v1/logs
    \endcode

    \sa LogEntry, RestResource, RestServer
*/

#include "logsresource.h"
#include "httprequest.h"
#include "loggingcategories.h"
#include "guhcore.h"
#include "logging/logengine.h"

#include <QJsonDocument>

namespace guhserver {

/*! Constructs a \l LogsResource with the given \a parent. */
LogsResource::LogsResource(QObject *parent) :
    RestResource(parent)
{
}

/*! Returns the name of the \l{RestResource}. In this case \b logs.

    \sa RestResource::name()
*/
QString LogsResource::name() const
{
    return "logs";
}

/*! This method will be used to process the given \a request and the given \a urlTokens. The request
    has to be in this namespace. Returns the resulting \l HttpReply.

    \sa HttpRequest, HttpReply, RestResource::proccessRequest()
*/
HttpReply *LogsResource::proccessRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    // check method
    HttpReply *reply;
    switch (request.method()) {
    case HttpRequest::Get:
        reply = proccessGetRequest(request, urlTokens);
        break;
    default:
        reply = createErrorReply(HttpReply::BadRequest);
        break;
    }
    return reply;
}

HttpReply *LogsResource::proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    // GET /api/v1/logs?filter={ogFilter}
    if (urlTokens.count() == 3) {
        // check filter
        QString filterString;
        if (request.url().hasQuery()) {
            if (request.urlQuery().hasQueryItem("filter")) {
                filterString = request.urlQuery().queryItemValue("filter");
            }
        }
        return getLogEntries(filterString);
    }
    return createErrorReply(HttpReply::NotImplemented);}

HttpReply *LogsResource::getLogEntries(const QString &filterString)
{
    qCDebug(dcRest) << "Get log entries";

    QPair<bool, QVariant> verification = RestResource::verifyPayload(filterString.toUtf8());
    if (!verification.first)
        return createErrorReply(HttpReply::BadRequest);

    QVariantMap filterMap = verification.second.toMap();

    LogFilter filter = JsonTypes::unpackLogFilter(filterMap);

    QVariantList entries;
    foreach (const LogEntry &entry, GuhCore::instance()->logEngine()->logEntries(filter)) {
        entries.append(JsonTypes::packLogEntry(entry));
    }
    HttpReply *reply = createSuccessReply();
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    reply->setPayload(QJsonDocument::fromVariant(entries).toJson());
    return reply;
}

}

