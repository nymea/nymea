/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::RestServer
    \brief This class provides the REST API interface to the \l{WebServer}{WebServers}.

    \ingroup server
    \inmodule core

    The \l{RestServer} class provides the server interface for a REST API call. The \l{RestServer}
    will create a \l{WebServer} object. The \l{WebServer} will parse the \l{HttpRequest} and emits
    the signal \l{WebServer::httpRequestReady()}. This signal will be handled by this \l{RestServer}
    and processed by the corresponding \l{RestResource}. Once the \l{HttpRequest} is finished, the
    \l{RestServer} will send a \l{HttpReply} back to the client using \l{WebServer::sendHttpReply()}.

    \sa ServerManager, WebServer, HttpRequest, HttpReply
*/

#include "restserver.h"
#include "loggingcategories.h"
#include "servers/httprequest.h"
#include "servers/httpreply.h"

#include <QJsonDocument>
#include <QSslConfiguration>

namespace nymeaserver {

/*! Constructs a \l{RestServer} with the given \a sslConfiguration and \a parent. */
RestServer::RestServer(const QSslConfiguration &sslConfiguration, QObject *parent) :
    QObject(parent)
{
    Q_UNUSED(sslConfiguration)

    QMetaObject::invokeMethod(this, "setup", Qt::QueuedConnection);
}

/*! Register the given \a webServer in this \l{RestServer}.
 *
 * \sa WebServer
 *
 */
void RestServer::registerWebserver(WebServer *webServer)
{
    connect(webServer, &WebServer::clientConnected, this, &RestServer::clientConnected);
    connect(webServer, &WebServer::clientDisconnected, this, &RestServer::clientDisconnected);
    connect(webServer, &WebServer::httpRequestReady, this, &RestServer::processHttpRequest);
}

void RestServer::setup()
{
    // Create resources
    m_deviceResource = new DevicesResource(this);
    m_deviceClassesResource = new DeviceClassesResource(this);
    m_vendorsResource = new VendorsResource(this);
    m_pluginsResource = new PluginsResource(this);
    m_rulesResource = new RulesResource(this);
    m_logsResource = new LogsResource(this);

    m_resources.insert(m_deviceResource->name(), m_deviceResource);
    m_resources.insert(m_deviceClassesResource->name(), m_deviceClassesResource);
    m_resources.insert(m_vendorsResource->name(), m_vendorsResource);
    m_resources.insert(m_pluginsResource->name(), m_pluginsResource);
    m_resources.insert(m_rulesResource->name(), m_rulesResource);
    m_resources.insert(m_logsResource->name(), m_logsResource);
}

void RestServer::clientConnected(const QUuid &clientId)
{
    WebServer *webserver = dynamic_cast<WebServer*>(sender());
    m_clientList.insert(clientId, webserver);
}

void RestServer::clientDisconnected(const QUuid &clientId)
{
    m_clientList.take(clientId);
}

void RestServer::processHttpRequest(const QUuid &clientId, const HttpRequest &request)
{
    QStringList urlTokens = request.url().path().split("/");
    urlTokens.removeAll(QString());

    WebServer *webserver = dynamic_cast<WebServer*>(sender());
    Q_ASSERT(webserver);

    // check token count
    if (urlTokens.count() < 3) {
        HttpReply *reply = RestResource::createErrorReply(HttpReply::BadRequest);
        reply->setClientId(clientId);
        webserver->sendHttpReply(reply);
        reply->deleteLater();
        return;
    }

    // check resource
    QString resourceName = urlTokens.at(2);
    if (!m_resources.contains(resourceName)) {
        HttpReply *reply = RestResource::createErrorReply(HttpReply::BadRequest);
        reply->setClientId(clientId);
        webserver->sendHttpReply(reply);
        reply->deleteLater();
        return;
    }

    // check CORS call for main resource
    if (request.method() == HttpRequest::Options && urlTokens.count() == 3) {
        HttpReply *reply = RestResource::createCorsSuccessReply();
        reply->setClientId(clientId);
        webserver->sendHttpReply(reply);
        reply->deleteLater();
        return;
    }


    // process request in corresponding resource
    RestResource *resource = m_resources.value(resourceName);
    HttpReply *reply = resource->proccessRequest(request, urlTokens);
    reply->setClientId(clientId);
    if (reply->type() == HttpReply::TypeAsync) {
        connect(reply, &HttpReply::finished, this, &RestServer::asyncReplyFinished);
        m_asyncReplies.insert(clientId, reply);
        reply->startWait();
        return;
    }
    webserver->sendHttpReply(reply);
    reply->deleteLater();
}

void RestServer::asyncReplyFinished()
{
    HttpReply *reply = qobject_cast<HttpReply*>(sender());

    if (!m_asyncReplies.values().contains(reply)) {
        qCWarning(dcWebServer()) << "Reply for async request does no longer exist";
        reply->deleteLater();
        return;
    }

    QUuid clientId = m_asyncReplies.key(reply);
    m_asyncReplies.remove(clientId);

    qCDebug(dcWebServer()) << "Async reply finished";

    // check if the reply timeouted
    if (reply->timedOut()) {
        reply->clear();
        reply->setHttpStatusCode(HttpReply::GatewayTimeout);
    }

    // check if client is still connected
    if (!m_clientList.contains(clientId)) {
        qCWarning(dcWebServer()) << "Client for async reply not longer connected.";
    } else {
        reply->setClientId(clientId);
        WebServer *webserver = m_clientList.value(clientId);
        webserver->sendHttpReply(reply);
    }
    reply->deleteLater();
}

}
