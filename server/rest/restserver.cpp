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

#include "restserver.h"
#include "loggingcategories.h"
#include "network/httprequest.h"
#include "network/httpreply.h"
#include "guhcore.h"

#include "devicehandler.h"
#include "actionhandler.h"
#include "ruleshandler.h"
#include "eventhandler.h"
#include "logginghandler.h"
#include "statehandler.h"

#include <QJsonDocument>

namespace guhserver {

RestServer::RestServer(QObject *parent) :
    QObject(parent)
{
    m_webserver = new WebServer(this);
    connect(m_webserver, &WebServer::clientConnected, this, &RestServer::clientConnected);
    connect(m_webserver, &WebServer::clientDisconnected, this, &RestServer::clientDisconnected);
    connect(m_webserver, &WebServer::httpRequestReady, this, &RestServer::processHttpRequest);

    m_webserver->startServer();
}

void RestServer::clientConnected(const QUuid &clientId)
{
    m_clientList.append(clientId);
}

void RestServer::clientDisconnected(const QUuid &clientId)
{
    m_clientList.removeAll(clientId);
}

void RestServer::processHttpRequest(const QUuid &clientId, const HttpRequest &request)
{
    qCDebug(dcWebServer) << "process http request" << clientId << request.method() << request.urlQuery().query();

    QString targetNamespace;
    QString method;
    QVariantMap params;

    if (request.urlQuery().hasQueryItem("devices")) {
        qCDebug(dcWebServer) << "devices resource";

    }

    if (request.method() == "GET" && request.urlQuery().query() == "/api/v1/devices.json") {
        targetNamespace = "Devices";
        method = "GetConfiguredDevices";
    } else if (request.method() == "GET" && request.urlQuery().query() == "/api/v1/devices.json") {
        targetNamespace = "Devices";
        method = "GetConfiguredDevices";
    } else {
        HttpReply httpReply(HttpReply::BadRequest);
        httpReply.setPayload("400 Bad Request.");
        httpReply.packReply();
        m_webserver->sendHttpReply(clientId, httpReply);
        return;
    }

    JsonHandler *handler = GuhCore::instance()->jsonRPCServer()->handlers().value(targetNamespace);

    QPair<bool, QString> validationResult = handler->validateParams(method, params);
    if (!validationResult.first) {
        qCWarning(dcWebServer) << "Invalid params: " << validationResult.second;
        return;
    }

    JsonReply *jsonReply;
    QMetaObject::invokeMethod(handler, method.toLatin1().data(), Q_RETURN_ARG(JsonReply*, jsonReply), Q_ARG(QVariantMap, params));
    if (jsonReply->type() == JsonReply::TypeAsync) {
        jsonReply->setClientId(clientId);
        connect(jsonReply, &JsonReply::finished, this, &RestServer::asyncReplyFinished);
        jsonReply->startWait();
        m_asyncReplies.insert(clientId, jsonReply);
        return;
    }

    HttpReply httpReply(HttpReply::Ok);
    httpReply.setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    httpReply.setPayload(QJsonDocument::fromVariant(jsonReply->data()).toJson());
    httpReply.packReply();

    m_webserver->sendHttpReply(clientId, httpReply);

    jsonReply->deleteLater();
}

void RestServer::asyncReplyFinished()
{
    JsonReply *jsonReply = qobject_cast<JsonReply*>(sender());
    QUuid clientId = m_asyncReplies.key(jsonReply);

    if (!jsonReply->timedOut()) {
        HttpReply httpReply(HttpReply::Ok);
        httpReply.setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
        httpReply.setPayload(QJsonDocument::fromVariant(jsonReply->data()).toJson());
        httpReply.packReply();
        m_webserver->sendHttpReply(clientId, httpReply);
    } else {
        HttpReply httpReply(HttpReply::GatewayTimeout);
        httpReply.setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
        httpReply.setPayload(QJsonDocument::fromVariant(jsonReply->data()).toJson());
        httpReply.packReply();
        m_webserver->sendHttpReply(clientId, httpReply);
    }
    jsonReply->deleteLater();
}


}
