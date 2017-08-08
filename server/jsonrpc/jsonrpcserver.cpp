/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014-2017 Michael Zanetti <michael.zanetti@guh.io>       *
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
    \class guhserver::JsonRPCServer
    \brief This class provides a JSON-RPC API interface to the \l{TransportInterface}{TransportInterfaces}.

    \ingroup server
    \inmodule core

    The \l{JsonRPCServer} class provides the server interface for a JSON-RPC API call. This class
    communicates with \l{TransportInterface}{TransportInterfaces} and processes the
    JSON-RPC request in the corresponding \l{JsonHandler}. The \l{JsonRPCServer} it self is also
    an \l{JsonHandler} and provides the introspection, version and notification control methods
    for the \l{JSON-RPC API}.

    \sa ServerManager, TransportInterface, TcpServer, WebSocketServer
*/


#include "jsonrpcserver.h"
#include "jsontypes.h"
#include "jsonhandler.h"
#include "guhcore.h"
#include "devicemanager.h"
#include "plugin/deviceplugin.h"
#include "plugin/deviceclass.h"
#include "plugin/device.h"
#include "rule.h"
#include "ruleengine.h"
#include "loggingcategories.h"

#include "devicehandler.h"
#include "actionhandler.h"
#include "ruleshandler.h"
#include "eventhandler.h"
#include "logginghandler.h"
#include "statehandler.h"
#include "websocketserver.h"
#include "configurationhandler.h"
#include "networkmanagerhandler.h"

#include <QJsonDocument>
#include <QStringList>
#include <QSslConfiguration>

namespace guhserver {

/*! Constructs a \l{JsonRPCServer} with the given \a sslConfiguration and \a parent. */
JsonRPCServer::JsonRPCServer(const QSslConfiguration &sslConfiguration, QObject *parent):
    JsonHandler(parent),
    m_notificationId(0)
{
    Q_UNUSED(sslConfiguration)
    // First, define our own JSONRPC methods
    QVariantMap returns;
    QVariantMap params;

    params.clear(); returns.clear();
    setDescription("Introspect", "Introspect this API.");
    setParams("Introspect", params);
    returns.insert("methods", JsonTypes::basicTypeToString(JsonTypes::Object));
    returns.insert("types", JsonTypes::basicTypeToString(JsonTypes::Object));
    setReturns("Introspect", returns);

    params.clear(); returns.clear();
    setDescription("Version", "Version of this Guh/JSONRPC interface.");
    setParams("Version", params);
    returns.insert("version", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("protocol version", JsonTypes::basicTypeToString(JsonTypes::String));
    setReturns("Version", returns);

    params.clear(); returns.clear();
    setDescription("SetNotificationStatus", "Enable/Disable notifications for this connections.");
    params.insert("enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setParams("SetNotificationStatus", params);
    returns.insert("enabled", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("SetNotificationStatus", returns);

    params.clear(); returns.clear();
    setDescription("CreateUser", "Create a new user in the API. Currently this is only allowed to be called once when a new guh instance is set up. Call Authenticate after this to obtain a device token for this user.");
    params.insert("username", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("password", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("CreateUser", params);
    returns.insert("error", JsonTypes::userErrorRef());
    setReturns("CreateUser", returns);

    params.clear(); returns.clear();
    setDescription("Authenticate", "Authenticate a client to the api. This will return a new token to be used to authorize a client at the API.");
    params.insert("username", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("password", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("deviceName", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("Authenticate", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("o:token", JsonTypes::basicTypeToString(JsonTypes::String));
    setReturns("Authenticate", returns);

    params.clear(); returns.clear();
    setDescription("Tokens", "Return a list of if TokenInfo objects all the tokens for the current user.");
    setParams("Tokens", params);
    returns.insert("tokenInfoList", QVariantList() << JsonTypes::tokenInfoRef());
    setReturns("Tokens", returns);

    params.clear(); returns.clear();
    setDescription("RemoveToken", "Revoke access for a given token.");
    params.insert("tokenId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("RemoveToken", params);
    returns.insert("error", JsonTypes::userErrorRef());
    setReturns("RemoveToken", returns);

    QMetaObject::invokeMethod(this, "setup", Qt::QueuedConnection);
}

/*! Returns the \e namespace of \l{JsonHandler}. */
QString JsonRPCServer::name() const
{
    return QStringLiteral("JSONRPC");
}

JsonReply* JsonRPCServer::Introspect(const QVariantMap &params) const
{
    Q_UNUSED(params)

    QVariantMap data;
    data.insert("types", JsonTypes::allTypes());
    QVariantMap methods;
    foreach (JsonHandler *handler, m_handlers)
        methods.unite(handler->introspect(QMetaMethod::Method));

    data.insert("methods", methods);

    QVariantMap signalsMap;
    foreach (JsonHandler *handler, m_handlers)
        signalsMap.unite(handler->introspect(QMetaMethod::Signal));

    data.insert("notifications", signalsMap);

    return createReply(data);
}

JsonReply* JsonRPCServer::Version(const QVariantMap &params) const
{
    Q_UNUSED(params)

    QVariantMap data;
    data.insert("version", GUH_VERSION_STRING);
    data.insert("protocol version", JSON_PROTOCOL_VERSION);
    return createReply(data);
}

JsonReply* JsonRPCServer::SetNotificationStatus(const QVariantMap &params)
{
    QUuid clientId = this->property("clientId").toUuid();
    m_clients[clientId] = params.value("enabled").toBool();
    QVariantMap returns;
    returns.insert("enabled", m_clients[clientId]);
    return createReply(returns);
}

JsonReply *JsonRPCServer::CreateUser(const QVariantMap &params)
{
    QString username = params.value("username").toString();
    QString password = params.value("password").toString();

    UserManager::UserError status = GuhCore::instance()->userManager()->createUser(username, password);

    QVariantMap returns;
    returns.insert("error", JsonTypes::userErrorToString(status));
    return createReply(returns);
}

JsonReply *JsonRPCServer::Authenticate(const QVariantMap &params)
{
    QString username = params.value("username").toString();
    QString password = params.value("password").toString();
    QString deviceName = params.value("deviceName").toString();

    QByteArray token = GuhCore::instance()->userManager()->authenticate(username, password, deviceName);
    QVariantMap ret;
    ret.insert("success", !token.isEmpty());
    if (!token.isEmpty()) {
        ret.insert("token", token);
    }
    return createReply(ret);
}

JsonReply *JsonRPCServer::Tokens(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QByteArray token = property("token").toByteArray();

    QString username = GuhCore::instance()->userManager()->userForToken(token);
    if (username.isEmpty()) {
        // There *really* should be a user for the token in the DB
        Q_ASSERT(false);
    }
    QList<TokenInfo> tokens = GuhCore::instance()->userManager()->tokens(username);
    QVariantList retList;
    foreach (const TokenInfo &tokenInfo, tokens) {
        retList << JsonTypes::packTokenInfo(tokenInfo);
    }
    QVariantMap retMap;
    retMap.insert("tokenInfoList", retList);
    return createReply(retMap);
}

JsonReply *JsonRPCServer::RemoveToken(const QVariantMap &params)
{
    QUuid tokenId = params.value("tokenId").toUuid();
    UserManager::UserError error = GuhCore::instance()->userManager()->removeToken(tokenId);
    QVariantMap ret;
    ret.insert("error", JsonTypes::userErrorToString(error));
    return createReply(ret);
}

/*! Returns the list of registred \l{JsonHandler}{JsonHandlers} and their name.*/
QHash<QString, JsonHandler *> JsonRPCServer::handlers() const
{
    return m_handlers;
}

/*! Register a new \l{TransportInterface} to the JSON server. The \a enabled flag indivates if the given \a interface sould be enebeld on startup. */
void JsonRPCServer::registerTransportInterface(TransportInterface *interface, const bool &enabled)
{
    connect(interface, &TransportInterface::clientConnected, this, &JsonRPCServer::clientConnected);
    connect(interface, &TransportInterface::clientDisconnected, this, &JsonRPCServer::clientDisconnected);
    connect(interface, &TransportInterface::dataAvailable, this, &JsonRPCServer::processData);

    if (enabled)
        QMetaObject::invokeMethod(interface, "startServer", Qt::QueuedConnection);

    m_interfaces.append(interface);
}

/*! Send a JSON success response to the client with the given \a clientId,
 * \a commandId and \a params to the inerted \l{TransportInterface}.
 */
void JsonRPCServer::sendResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QVariantMap &params)
{
    QVariantMap response;
    response.insert("id", commandId);
    response.insert("status", "success");
    response.insert("params", params);

    interface->sendData(clientId, QJsonDocument::fromVariant(response).toJson());
}

/*! Send a JSON error response to the client with the given \a clientId,
 * \a commandId and \a error to the inerted \l{TransportInterface}.
 */
void JsonRPCServer::sendErrorResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error)
{
    QVariantMap errorResponse;
    errorResponse.insert("id", commandId);
    errorResponse.insert("status", "error");
    errorResponse.insert("error", error);

    interface->sendData(clientId, QJsonDocument::fromVariant(errorResponse).toJson());
}

void JsonRPCServer::sendUnauthorizedResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error)
{
    QVariantMap errorResponse;
    errorResponse.insert("id", commandId);
    errorResponse.insert("status", "unauthorized");
    errorResponse.insert("error", error);

    interface->sendData(clientId, QJsonDocument::fromVariant(errorResponse).toJson());
}

void JsonRPCServer::setup()
{
    registerHandler(this);
    registerHandler(new DeviceHandler(this));
    registerHandler(new ActionHandler(this));
    registerHandler(new RulesHandler(this));
    registerHandler(new EventHandler(this));
    registerHandler(new LoggingHandler(this));
    registerHandler(new StateHandler(this));
    registerHandler(new ConfigurationHandler(this));
    registerHandler(new NetworkManagerHandler(this));
}

void JsonRPCServer::processData(const QUuid &clientId, const QByteArray &data)
{
    TransportInterface *interface = qobject_cast<TransportInterface *>(sender());
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcJsonRpc) << "Failed to parse JSON data" << data << ":" << error.errorString();
        sendErrorResponse(interface, clientId, -1, QString("Failed to parse JSON data: %1").arg(error.errorString()));
        return;
    }

    QVariantMap message = jsonDoc.toVariant().toMap();

    bool success;
    int commandId = message.value("id").toInt(&success);
    if (!success) {
        qCWarning(dcJsonRpc) << "Error parsing command. Missing \"id\":" << message;
        sendErrorResponse(interface, clientId, commandId, "Error parsing command. Missing 'id'");
        return;
    }

    QStringList commandList = message.value("method").toString().split('.');
    if (commandList.count() != 2) {
        qCWarning(dcJsonRpc) << "Error parsing method.\nGot:" << message.value("method").toString() << "\nExpected: \"Namespace.method\"";
        sendErrorResponse(interface, clientId, commandId, QString("Error parsing method. Got: '%1'', Expected: 'Namespace.method'").arg(message.value("method").toString()));
        return;
    }
    QString targetNamespace = commandList.first();
    QString method = commandList.last();

    // if there is no user in the system yet, let's fail unless this is a CreateUser or Introspect call
    if (GuhCore::instance()->userManager()->users().isEmpty()) {
        if (!(targetNamespace == "JSONRPC" && (method == "CreateUser" || method == "Introspect"))) {
            sendUnauthorizedResponse(interface, clientId, commandId, "Initial setup required. Call CreateUser first.");
            return;
        }
    } else {
        // ok, we have a user. if there isn't a valid token, let's fail unless this is a Authenticate or Introspect call
        QByteArray token = message.value("token").toByteArray();
        if (!(targetNamespace == "JSONRPC" && (method == "Authenticate" || method == "Introspect")) && !GuhCore::instance()->userManager()->verifyToken(token)) {
            sendUnauthorizedResponse(interface, clientId, commandId, "Forbidden: Invalid token.");
            return;
        }
    }
    // At this point we can assume all the calls are authorized

    JsonHandler *handler = m_handlers.value(targetNamespace);
    if (!handler) {
        sendErrorResponse(interface, clientId, commandId, "No such namespace");
        return;
    }
    if (!handler->hasMethod(method)) {
        sendErrorResponse(interface, clientId, commandId, "No such method");
        return;
    }

    QVariantMap params = message.value("params").toMap();

    QPair<bool, QString> validationResult = handler->validateParams(method, params);
    if (!validationResult.first) {
        sendErrorResponse(interface, clientId, commandId, "Invalid params: " + validationResult.second);
        return;
    }

    // Hack: attach clientId to handler to be able to handle the JSONRPC methods. Do not use this outside of jsonrpcserver
    handler->setProperty("clientId", clientId);
    handler->setProperty("token", message.value("token").toByteArray());

    JsonReply *reply;
    QMetaObject::invokeMethod(handler, method.toLatin1().data(), Q_RETURN_ARG(JsonReply*, reply), Q_ARG(QVariantMap, params));
    if (reply->type() == JsonReply::TypeAsync) {
        m_asyncReplies.insert(reply, interface);
        reply->setClientId(clientId);
        reply->setCommandId(commandId);
        connect(reply, &JsonReply::finished, this, &JsonRPCServer::asyncReplyFinished);
        reply->startWait();
    } else {
        Q_ASSERT_X((targetNamespace == "JSONRPC" && method == "Introspect") || handler->validateReturns(method, reply->data()).first
                   ,"validating return value", formatAssertion(targetNamespace, method, handler, reply->data()).toLatin1().data());
        sendResponse(interface, clientId, commandId, reply->data());
        reply->deleteLater();
    }
}

QString JsonRPCServer::formatAssertion(const QString &targetNamespace, const QString &method, JsonHandler *handler, const QVariantMap &data) const
{
    QJsonDocument doc = QJsonDocument::fromVariant(handler->introspect(QMetaMethod::Method).value(targetNamespace + "." + method));
    QJsonDocument doc2 = QJsonDocument::fromVariant(data);
    return QString("\nMethod: %1\nTemplate: %2\nValue: %3")
            .arg(targetNamespace + "." + method)
            .arg(QString(doc.toJson()))
            .arg(QString(doc2.toJson()));
}

void JsonRPCServer::sendNotification(const QVariantMap &params)
{
    JsonHandler *handler = qobject_cast<JsonHandler *>(sender());
    QMetaMethod method = handler->metaObject()->method(senderSignalIndex());

    QVariantMap notification;
    notification.insert("id", m_notificationId++);
    notification.insert("notification", handler->name() + "." + method.name());
    notification.insert("params", params);

    foreach (TransportInterface *interface, m_interfaces) {
        interface->sendData(m_clients.keys(true), QJsonDocument::fromVariant(notification).toJson());
    }
}

void JsonRPCServer::asyncReplyFinished()
{
    JsonReply *reply = qobject_cast<JsonReply *>(sender());
    TransportInterface *interface = m_asyncReplies.take(reply);
    if (!reply->timedOut()) {
        Q_ASSERT_X(reply->handler()->validateReturns(reply->method(), reply->data()).first
                   ,"validating return value", formatAssertion(reply->handler()->name(), reply->method(), reply->handler(), reply->data()).toLatin1().data());
        sendResponse(interface, reply->clientId(), reply->commandId(), reply->data());
    } else {
        sendErrorResponse(interface, reply->clientId(), reply->commandId(), "Command timed out");
    }

    reply->deleteLater();
}

void JsonRPCServer::registerHandler(JsonHandler *handler)
{
    m_handlers.insert(handler->name(), handler);
    for (int i = 0; i < handler->metaObject()->methodCount(); ++i) {
        QMetaMethod method = handler->metaObject()->method(i);
        if (method.methodType() == QMetaMethod::Signal && QString(method.name()).contains(QRegExp("^[A-Z]"))) {
            QObject::connect(handler, method, this, metaObject()->method(metaObject()->indexOfSlot("sendNotification(QVariantMap)")));
        }
    }
}

void JsonRPCServer::clientConnected(const QUuid &clientId)
{
    // Notifications disabled by default. Clients must enable them with a valid token
    m_clients.insert(clientId, false);

    TransportInterface *interface = qobject_cast<TransportInterface *>(sender());

    QVariantMap handshake;
    handshake.insert("id", 0);
    handshake.insert("server", "guhIO");
    handshake.insert("name", GuhCore::instance()->configuration()->serverName());
    handshake.insert("version", GUH_VERSION_STRING);
    handshake.insert("uuid", GuhCore::instance()->configuration()->serverUuid().toString());
    handshake.insert("language", GuhCore::instance()->configuration()->locale().name());
    handshake.insert("protocol version", JSON_PROTOCOL_VERSION);
    handshake.insert("initialSetupRequired", GuhCore::instance()->userManager()->users().isEmpty());
    interface->sendData(clientId, QJsonDocument::fromVariant(handshake).toJson());
}

void JsonRPCServer::clientDisconnected(const QUuid &clientId)
{
    m_clients.remove(clientId);
}

}
