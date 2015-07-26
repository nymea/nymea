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

#include <QJsonDocument>

namespace guhserver {

RestServer::RestServer(QObject *parent) :
    QObject(parent)
{
    m_webserver = new WebServer(this);
    connect(m_webserver, &WebServer::clientConnected, this, &RestServer::clientConnected);
    connect(m_webserver, &WebServer::clientDisconnected, this, &RestServer::clientDisconnected);
    connect(m_webserver, &WebServer::httpRequestReady, this, &RestServer::processHttpRequest);

    // Resources
    m_deviceResource = new DevicesResource(this);


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
    qCDebug(dcRest) << "Process HTTP request" << clientId << request.method() << request.urlQuery().query();

    QStringList urlTokens = request.urlQuery().query(QUrl::FullyDecoded).split("/");
    urlTokens.removeAll(QString());

    qCDebug(dcRest) << urlTokens;

    if (urlTokens.count() < 3) {
        m_webserver->sendHttpReply(clientId, HttpReply(HttpReply::BadRequest));
        return;
    }

    if (urlTokens.at(2) == "devices") {
        HttpReply httpReply = m_deviceResource->proccessDeviceRequest(request, urlTokens);
        qCDebug(dcRest) << "sending header" << httpReply.rawHeader();
        m_webserver->sendHttpReply(clientId, httpReply);
        return;
    }


    //    QString targetNamespace;
    //    QString method;
    //    QVariantMap params;

    // // check filter
    //        QVariantList deviceList;
    //        if (!request.urlQuery().hasQueryItem("id")) {
    //            HttpReply httpReply = m_deviceResource->proccessDeviceRequest(request);
    //            m_webserver->sendHttpReply(clientId, httpReply);
    //            return;
    //        } else {
    //            foreach (const QString& idString, request.urlQuery().allQueryItemValues("id")) {
    //                Device *device = GuhCore::instance()->deviceManager()->findConfiguredDevice(DeviceId(idString));
    //                if (device == Device()) {

    //                }
    //            }
    //        }
    //    }


    //    if (request.method() == HttpRequest::Get && request.urlQuery().query() == "/api/v1/devices.json") {
    //        targetNamespace = "Devices";
    //        method = "GetConfiguredDevices";
    //    } else if (request.method() == HttpRequest::Get && request.urlQuery().query() == "/api/v1/devices.json") {
    //        targetNamespace = "Devices";
    //        method = "GetConfiguredDevices";
    //    } else {
    //        HttpReply httpReply(HttpReply::BadRequest);
    //        httpReply.setPayload("400 Bad Request.");
    //        httpReply.packReply();
    //        m_webserver->sendHttpReply(clientId, httpReply);
    //        return;
    //    }

    //    JsonHandler *handler = GuhCore::instance()->jsonRPCServer()->handlers().value(targetNamespace);

    //    QPair<bool, QString> validationResult = handler->validateParams(method, params);
    //    if (!validationResult.first) {
    //        qCWarning(dcWebServer) << "Invalid params: " << validationResult.second;
    //        return;
    //    }

    //    JsonReply *jsonReply;
    //    QMetaObject::invokeMethod(handler, method.toLatin1().data(), Q_RETURN_ARG(JsonReply*, jsonReply), Q_ARG(QVariantMap, params));
    //    if (jsonReply->type() == JsonReply::TypeAsync) {
    //        jsonReply->setClientId(clientId);
    //        connect(jsonReply, &JsonReply::finished, this, &RestServer::asyncReplyFinished);
    //        jsonReply->startWait();
    //        m_asyncReplies.insert(clientId, jsonReply);
    //        return;
    //    }

    //    HttpReply httpReply(HttpReply::Ok);
    //    httpReply.setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    //    httpReply.setPayload(QJsonDocument::fromVariant(jsonReply->data()).toJson());
    //    httpReply.packReply();

    //    m_webserver->sendHttpReply(clientId, httpReply);

    //    jsonReply->deleteLater();
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
