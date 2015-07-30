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

#include "restresource.h"
#include "httprequest.h"
#include "loggingcategories.h"
#include "guhcore.h"

#include <QJsonDocument>
#include <QVariant>

namespace guhserver {

RestResource::RestResource(QObject *parent) :
    QObject(parent)
{
}

RestResource::~RestResource()
{
}

HttpReply *RestResource::createSuccessReply()
{
    HttpReply *reply = new HttpReply(HttpReply::Ok, HttpReply::TypeSync);
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    return reply;
}

HttpReply *RestResource::createErrorReply(const HttpReply::HttpStatusCode &statusCode)
{
    HttpReply *reply = new HttpReply(statusCode, HttpReply::TypeSync);
    reply->setPayload(QByteArray::number(reply->httpStatusCode()) + " " + reply->httpReasonPhrase());
    return reply;
}

HttpReply *RestResource::createAsyncReply()
{
    HttpReply *reply = new HttpReply(HttpReply::Ok, HttpReply::TypeAsync);
    return reply;
}

QPair<bool, QVariant> RestResource::verifyPayload(const QByteArray &payload)
{
    QVariant data;
    if (!payload.isEmpty()) {
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(payload, &error);

        if(error.error != QJsonParseError::NoError) {
            qCWarning(dcRest) << "Failed to parse JSON payload" << payload << ":" << error.errorString();
            return QPair<bool, QVariant>(true, QVariant());
        }

        data = jsonDoc.toVariant();
    }
    return QPair<bool, QVariant>(true, data);
}

HttpReply *RestResource::proccessPostRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)
    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RestResource::proccessPutRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)
    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RestResource::proccessDeleteRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)
    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *RestResource::proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)
    Q_UNUSED(urlTokens)
    return createErrorReply(HttpReply::NotImplemented);
}

}
