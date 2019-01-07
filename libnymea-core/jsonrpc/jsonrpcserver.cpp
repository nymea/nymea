/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014-2017 Michael Zanetti <michael.zanetti@guh.io>       *
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
    \class nymeaserver::JsonRPCServer
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
#include "nymeacore.h"
#include "devicemanager.h"
#include "plugin/deviceplugin.h"
#include "types/deviceclass.h"
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
#include "configurationhandler.h"
#include "networkmanagerhandler.h"
#include "tagshandler.h"

#include <QJsonDocument>
#include <QStringList>
#include <QSslConfiguration>

namespace nymeaserver {

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
    setDescription("Hello", "Upon first connection, nymea will automatically send a welcome message containing information about the setup. If this message is lost for whatever reason (connections with multiple hops might drop this if nymea sends it too early), the exact same message can be retrieved multiple times by calling this Hello method. Note that the contents might change if the system changed its state in the meantime, e.g. initialSetupRequired might turn false if the initial setup has been performed in the meantime.");
    setParams("Hello", params);
    returns.insert("id", JsonTypes::basicTypeToString(JsonTypes::Int));
    returns.insert("server", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("version", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("uuid", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    returns.insert("language", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("protocol version", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("initialSetupRequired", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("authenticationRequired", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("pushButtonAuthAvailable", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("Hello", returns);

    params.clear(); returns.clear();
    setDescription("Introspect", "Introspect this API.");
    setParams("Introspect", params);
    returns.insert("methods", JsonTypes::basicTypeToString(JsonTypes::Object));
    returns.insert("notifications", JsonTypes::basicTypeToString(JsonTypes::Object));
    returns.insert("types", JsonTypes::basicTypeToString(JsonTypes::Object));
    setReturns("Introspect", returns);

    params.clear(); returns.clear();
    setDescription("Version", "Version of this nymea/JSONRPC interface.");
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
    setDescription("CreateUser", "Create a new user in the API. Currently this is only allowed to be called once when a new nymea instance is set up. Call Authenticate after this to obtain a device token for this user.");
    params.insert("username", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("password", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("CreateUser", params);
    returns.insert("error", JsonTypes::userErrorRef());
    setReturns("CreateUser", returns);

    params.clear(); returns.clear();
    setDescription("Authenticate", "Authenticate a client to the api via user & password challenge. Provide "
                   "a device name which allows the user to identify the client and revoke the token in case "
                   "the device is lost or stolen. This will return a new token to be used to authorize a "
                   "client at the API.");
    params.insert("username", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("password", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("deviceName", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("Authenticate", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("o:token", JsonTypes::basicTypeToString(JsonTypes::String));
    setReturns("Authenticate", returns);

    params.clear(); returns.clear();
    setDescription("RequestPushButtonAuth", "Authenticate a client to the api via Push Button method. "
                   "Provide a device name which allows the user to identify the client and revoke the "
                   "token in case the device is lost or stolen. If push button hardware is available, "
                   "this will return with success and start listening for push button presses. When the "
                   "push button is pressed, the PushButtonAuthFinished notification will be sent to the "
                   "requesting client. The procedure will be cancelled when the connection is interrupted. "
                   "If another client requests push button authentication while a procedure is still going "
                   "on, the second call will take over and the first one will be notified by the "
                   "PushButtonAuthFinished signal about the error. The application should make it clear "
                   "to the user to not press the button when the procedure fails as this can happen for 2 "
                   "reasons: a) a second user is trying to auth at the same time and only the currently "
                   "active user should press the button or b) it might indicate an attacker trying to take "
                   "over and snooping in for tokens.");
    params.insert("deviceName", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("RequestPushButtonAuth", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("transactionId", JsonTypes::basicTypeToString(JsonTypes::Int));
    setReturns("RequestPushButtonAuth", returns);

    params.clear(); returns.clear();
    setDescription("Tokens", "Return a list of TokenInfo objects of all the tokens for the current user.");
    setParams("Tokens", params);
    returns.insert("tokenInfoList", QVariantList() << JsonTypes::tokenInfoRef());
    setReturns("Tokens", returns);

    params.clear(); returns.clear();
    setDescription("RemoveToken", "Revoke access for a given token.");
    params.insert("tokenId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("RemoveToken", params);
    returns.insert("error", JsonTypes::userErrorRef());
    setReturns("RemoveToken", returns);

    params.clear(); returns.clear();
    setDescription("SetupCloudConnection", "Sets up the cloud connection by deploying a certificate and its configuration.");
    params.insert("rootCA", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("certificatePEM", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("publicKey", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("privateKey", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("endpoint", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("SetupCloudConnection", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    setReturns("SetupCloudConnection", returns);

    params.clear(); returns.clear();
    setDescription("SetupRemoteAccess", "Setup the remote connection by providing AWS token information. This requires the cloud to be connected.");
    params.insert("idToken", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("userId", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("SetupRemoteAccess", params);
    returns.insert("status", JsonTypes::basicTypeToString(JsonTypes::Int));
    returns.insert("message", JsonTypes::basicTypeToString(JsonTypes::String));
    setReturns("SetupRemoteAccess", returns);

    params.clear(); returns.clear();
    setDescription("IsCloudConnected", "Check whether the cloud is currently connected. \"connected\" will be true whenever connectionState equals CloudConnectionStateConnected and is deprecated. Please use the connectionState value instead.");
    setParams("IsCloudConnected", params);
    returns.insert("connected", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("connectionState", JsonTypes::cloudConnectionStateRef());
    setReturns("IsCloudConnected", returns);

    params.clear(); returns.clear();
    setDescription("KeepAlive", "This is basically a Ping/Pong mechanism a client app may use to check server connectivity. Currently, the server does not actually do anything with this information and will return the call providing the given sessionId back to the caller. It is up to the client whether to use this or not and not required by the server to keep the connection alive.");
    params.insert("sessionId", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("KeepAlive", params);
    returns.insert("success", JsonTypes::basicTypeToString(JsonTypes::Bool));
    returns.insert("sessionId", JsonTypes::basicTypeToString(JsonTypes::String));
    setReturns("KeepAlive", returns);

    // Notifications
    params.clear(); returns.clear();
    setDescription("CloudConnectedChanged", "Emitted whenever the cloud connection status changes.");
    params.insert("connected", JsonTypes::basicTypeToString(JsonTypes::Bool));
    params.insert("connectionState", JsonTypes::cloudConnectionStateRef());
    setParams("CloudConnectedChanged", params);

    params.clear();
    setDescription("PushButtonAuthFinished", "Emitted when a push button authentication reaches final state. NOTE: This notification is special. It will only be emitted to connections that did actively request a push button authentication, but also it will be emitted regardless of the notification settings. ");
    params.insert("status", JsonTypes::userErrorRef());
    params.insert("transactionId", JsonTypes::basicTypeToString(JsonTypes::Int));
    params.insert("o:token", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("PushButtonAuthFinished", params);

    QMetaObject::invokeMethod(this, "setup", Qt::QueuedConnection);

    connect(NymeaCore::instance()->userManager(), &UserManager::pushButtonAuthFinished, this, &JsonRPCServer::onPushButtonAuthFinished);
}

/*! Returns the \e namespace of \l{JsonHandler}. */
QString JsonRPCServer::name() const
{
    return QStringLiteral("JSONRPC");
}

JsonReply *JsonRPCServer::Hello(const QVariantMap &params) const
{
    Q_UNUSED(params);
    TransportInterface *interface = reinterpret_cast<TransportInterface*>(property("transportInterface").toLongLong());
    return createReply(createWelcomeMessage(interface));
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
    data.insert("version", NYMEA_VERSION_STRING);
    data.insert("protocol version", JSON_PROTOCOL_VERSION);
    return createReply(data);
}

JsonReply* JsonRPCServer::SetNotificationStatus(const QVariantMap &params)
{
    QUuid clientId = this->property("clientId").toUuid();
    m_clientNotifications[clientId] = params.value("enabled").toBool();
    QVariantMap returns;
    returns.insert("enabled", m_clientNotifications[clientId]);
    return createReply(returns);
}

JsonReply *JsonRPCServer::CreateUser(const QVariantMap &params)
{
    QString username = params.value("username").toString();
    QString password = params.value("password").toString();

    UserManager::UserError status = NymeaCore::instance()->userManager()->createUser(username, password);

    QVariantMap returns;
    returns.insert("error", JsonTypes::userErrorToString(status));
    return createReply(returns);
}

JsonReply *JsonRPCServer::Authenticate(const QVariantMap &params)
{
    QString username = params.value("username").toString();
    QString password = params.value("password").toString();
    QString deviceName = params.value("deviceName").toString();

    QByteArray token = NymeaCore::instance()->userManager()->authenticate(username, password, deviceName);
    QVariantMap ret;
    ret.insert("success", !token.isEmpty());
    if (!token.isEmpty()) {
        ret.insert("token", token);
    }
    return createReply(ret);
}

JsonReply *JsonRPCServer::RequestPushButtonAuth(const QVariantMap &params)
{
    QString deviceName = params.value("deviceName").toString();
    QUuid clientId = this->property("clientId").toUuid();

    int transactionId = NymeaCore::instance()->userManager()->requestPushButtonAuth(deviceName);
    m_pushButtonTransactions.insert(transactionId, clientId);

    QVariantMap data;
    data.insert("transactionId", transactionId);
    // TODO: return false if pushbutton auth is disabled in settings
    data.insert("success", true);
    return createReply(data);
}

JsonReply *JsonRPCServer::Tokens(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QByteArray token = property("token").toByteArray();

    QString username = NymeaCore::instance()->userManager()->userForToken(token);
    if (username.isEmpty()) {
        // There *really* should be a user for the token in the DB
        Q_ASSERT(false);
    }
    QList<TokenInfo> tokens = NymeaCore::instance()->userManager()->tokens(username);
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
    UserManager::UserError error = NymeaCore::instance()->userManager()->removeToken(tokenId);
    QVariantMap ret;
    ret.insert("error", JsonTypes::userErrorToString(error));
    return createReply(ret);
}

JsonReply *JsonRPCServer::SetupCloudConnection(const QVariantMap &params)
{
    if (NymeaCore::instance()->cloudManager()->connectionState() != CloudManager::CloudConnectionStateUnconfigured) {
        qCDebug(dcCloud) << "Cloud already configured. Not changing configuration as it won't work anyways. If you want to reconfigure this instance to a different cloud, change the system UUID and wipe the cloud settings from the config.";
        QVariantMap data;
        data.insert("success", false);
        return createReply(data);
    }
    QByteArray rootCA = params.value("rootCA").toByteArray();
    QByteArray certificatePEM = params.value("certificatePEM").toByteArray();
    QByteArray publicKey = params.value("publicKey").toByteArray();
    QByteArray privateKey = params.value("privateKey").toByteArray();
    QString endpoint = params.value("endpoint").toString();
    bool status = NymeaCore::instance()->cloudManager()->installClientCertificates(rootCA, certificatePEM, publicKey, privateKey, endpoint);
    QVariantMap ret;
    ret.insert("success", status);
    return createReply(ret);
}

JsonReply *JsonRPCServer::SetupRemoteAccess(const QVariantMap &params)
{
    QString idToken = params.value("idToken").toString();
    QString userId = params.value("userId").toString();
    NymeaCore::instance()->cloudManager()->pairDevice(idToken, userId);
    JsonReply *reply = createAsyncReply("SetupRemoteAccess");
    m_pairingRequests.insert(userId, reply);
    connect(reply, &JsonReply::finished, [this, userId](){
        m_pairingRequests.remove(userId);
    });
    return reply;
}

JsonReply *JsonRPCServer::IsCloudConnected(const QVariantMap &params)
{
    Q_UNUSED(params)
    bool connected = NymeaCore::instance()->cloudManager()->connectionState() == CloudManager::CloudConnectionStateConnected;
    QVariantMap data;
    data.insert("connected", connected);
    data.insert("connectionState", JsonTypes::cloudConnectionStateToString(NymeaCore::instance()->cloudManager()->connectionState()));
    return createReply(data);
}

/*! A client may use this as a ping/pong mechanism to check server connectivity. */
JsonReply *JsonRPCServer::KeepAlive(const QVariantMap &params)
{
    QString sessionId = params.value("sessionId").toString();
    qCDebug(dcJsonRpc()) << "KeepAlive received" << sessionId;
    QVariantMap resultMap;
    resultMap.insert("success", true);
    resultMap.insert("sessionId", sessionId);
    return createReply(resultMap);
}

/*! Returns the list of registered \l{JsonHandler}{JsonHandlers} and their name.*/
QHash<QString, JsonHandler *> JsonRPCServer::handlers() const
{
    return m_handlers;
}

/*! Register a new \l{TransportInterface} to the JSON server. If the given interface is already registered, just the authenticationRequired flag will be updated. */
void JsonRPCServer::registerTransportInterface(TransportInterface *interface, bool authenticationRequired)
{
    if (!m_interfaces.contains(interface)) {
        connect(interface, &TransportInterface::clientConnected, this, &JsonRPCServer::clientConnected);
        connect(interface, &TransportInterface::clientDisconnected, this, &JsonRPCServer::clientDisconnected);
        connect(interface, &TransportInterface::dataAvailable, this, &JsonRPCServer::processData);
        m_interfaces.insert(interface, authenticationRequired);
    } else {
        m_interfaces[interface] = authenticationRequired;
    }
}

void JsonRPCServer::unregisterTransportInterface(TransportInterface *interface)
{
    disconnect(interface, &TransportInterface::clientConnected, this, &JsonRPCServer::clientConnected);
    disconnect(interface, &TransportInterface::clientDisconnected, this, &JsonRPCServer::clientDisconnected);
    disconnect(interface, &TransportInterface::dataAvailable, this, &JsonRPCServer::processData);
    m_interfaces.take(interface);
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

    QByteArray data = QJsonDocument::fromVariant(response).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpcTraffic()) << "Sending data:" << data;
    interface->sendData(clientId, data);
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

    QByteArray data = QJsonDocument::fromVariant(errorResponse).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpcTraffic()) << "Sending data:" << data;
    interface->sendData(clientId, data);
}

void JsonRPCServer::sendUnauthorizedResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error)
{
    QVariantMap errorResponse;
    errorResponse.insert("id", commandId);
    errorResponse.insert("status", "unauthorized");
    errorResponse.insert("error", error);

    QByteArray data = QJsonDocument::fromVariant(errorResponse).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpcTraffic()) << "Sending data:" << data;
    interface->sendData(clientId, data);
}

QVariantMap JsonRPCServer::createWelcomeMessage(TransportInterface *interface) const
{
    QVariantMap handshake;
    handshake.insert("id", 0);
    handshake.insert("server", "nymea");
    handshake.insert("name", NymeaCore::instance()->configuration()->serverName());
    handshake.insert("version", NYMEA_VERSION_STRING);
    handshake.insert("uuid", NymeaCore::instance()->configuration()->serverUuid().toString());
    handshake.insert("language", NymeaCore::instance()->configuration()->locale().name());
    handshake.insert("protocol version", JSON_PROTOCOL_VERSION);
    handshake.insert("initialSetupRequired", (interface->configuration().authenticationEnabled ? NymeaCore::instance()->userManager()->initRequired() : false));
    handshake.insert("authenticationRequired", interface->configuration().authenticationEnabled);
    handshake.insert("pushButtonAuthAvailable", NymeaCore::instance()->userManager()->pushButtonAuthAvailable());
    return handshake;
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
    registerHandler(new TagsHandler(this));

    connect(NymeaCore::instance()->cloudManager(), &CloudManager::pairingReply, this, &JsonRPCServer::pairingFinished);
    connect(NymeaCore::instance()->cloudManager(), &CloudManager::connectionStateChanged, this, &JsonRPCServer::onCloudConnectionStateChanged);
}

void JsonRPCServer::processData(const QUuid &clientId, const QByteArray &data)
{
    qCDebug(dcJsonRpcTraffic()) << "Incoming data:" << data;

    TransportInterface *interface = qobject_cast<TransportInterface *>(sender());

    // Handle packet fragmentation
    QByteArray buffer = m_clientBuffers[clientId];
    buffer.append(data);
    int splitIndex = buffer.indexOf("}\n{");
    while (splitIndex > -1) {
        processJsonPacket(interface, clientId, buffer.left(splitIndex + 1));
        buffer = buffer.right(buffer.length() - splitIndex - 2);
        splitIndex = buffer.indexOf("}\n{");
    }
    if (buffer.trimmed().endsWith("}")) {
        processJsonPacket(interface, clientId, buffer);
        buffer.clear();
    }
    m_clientBuffers[clientId] = buffer;

    if (buffer.size() > 1024 * 10) {
        qCWarning(dcJsonRpc()) << "Client buffer larger than 10KB and no valid data. Dropping client connection.";
        interface->terminateClientConnection(clientId);
    }
}

void JsonRPCServer::processJsonPacket(TransportInterface *interface, const QUuid &clientId, const QByteArray &data)
{
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

    // check if authentication is required for this transport
    if (m_interfaces.value(interface)) {
        QByteArray token = message.value("token").toByteArray();
        QStringList authExemptMethodsNoUser = {"Introspect", "Hello", "CreateUser", "RequestPushButtonAuth"};
        QStringList authExemptMethodsWithUser = {"Introspect", "Hello", "Authenticate", "RequestPushButtonAuth"};
        // if there is no user in the system yet, let's fail unless this is special method for authentication itself
        if (NymeaCore::instance()->userManager()->initRequired()) {
            if (!(targetNamespace == "JSONRPC" && authExemptMethodsNoUser.contains(method)) && (token.isEmpty() || !NymeaCore::instance()->userManager()->verifyToken(token))) {
                sendUnauthorizedResponse(interface, clientId, commandId, "Initial setup required. Call CreateUser first.");
                return;
            }
        } else {
            // ok, we have a user. if there isn't a valid token, let's fail unless this is a Authenticate, Introspect  Hello call
            if (!(targetNamespace == "JSONRPC" && authExemptMethodsWithUser.contains(method)) && (token.isEmpty() || !NymeaCore::instance()->userManager()->verifyToken(token))) {
                sendUnauthorizedResponse(interface, clientId, commandId, "Forbidden: Invalid token.");
                return;
            }
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

    // Hack: attach some properties to the handler to be able to handle the JSONRPC methods. Do not use this outside of jsonrpcserver
    handler->setProperty("clientId", clientId);
    handler->setProperty("token", message.value("token").toByteArray());
    handler->setProperty("transportInterface", reinterpret_cast<qint64>(interface));

    qCDebug(dcJsonRpc()) << "Invoking method" << targetNamespace << method.toLatin1().data();

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
                   ,"validating return value", formatAssertion(targetNamespace, method, QMetaMethod::Method, handler, reply->data()).toLatin1().data());
        sendResponse(interface, clientId, commandId, reply->data());
        reply->deleteLater();
    }
}

QString JsonRPCServer::formatAssertion(const QString &targetNamespace, const QString &method, QMetaMethod::MethodType methodType, JsonHandler *handler, const QVariantMap &data) const
{
    QJsonDocument doc = QJsonDocument::fromVariant(handler->introspect(methodType).value(targetNamespace + "." + method));
    QJsonDocument doc2 = QJsonDocument::fromVariant(data);
    return QString("\nMethod: %1\nTemplate: %2\nValue: %3")
            .arg(targetNamespace + "." + method)
            .arg(QString(doc.toJson(QJsonDocument::Indented)))
            .arg(QString(doc2.toJson(QJsonDocument::Indented)));
}

void JsonRPCServer::sendNotification(const QVariantMap &params)
{
    JsonHandler *handler = qobject_cast<JsonHandler *>(sender());
    QMetaMethod method = handler->metaObject()->method(senderSignalIndex());

    QVariantMap notification;
    notification.insert("id", m_notificationId++);
    notification.insert("notification", handler->name() + "." + method.name());
    notification.insert("params", params);

    Q_ASSERT_X(handler->validateParams(method.name(), params).first, "validating return value", formatAssertion(handler->name(), method.name(), QMetaMethod::Signal, handler, notification).toLatin1().data());
    QByteArray data = QJsonDocument::fromVariant(notification).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpcTraffic()) << "Sending notification:" << data;

    foreach (const QUuid &clientId, m_clientNotifications.keys(true)) {
        m_clientTransports.value(clientId)->sendData(clientId, data);
    }
}

void JsonRPCServer::asyncReplyFinished()
{
    JsonReply *reply = qobject_cast<JsonReply *>(sender());
    TransportInterface *interface = m_asyncReplies.take(reply);
    if (!interface) {
        qCWarning(dcJsonRpc()) << "Got an async reply but the requesting connection has vanished.";
        reply->deleteLater();
        return;
    }
    if (!reply->timedOut()) {
        Q_ASSERT_X(reply->handler()->validateReturns(reply->method(), reply->data()).first
                   ,"validating return value", formatAssertion(reply->handler()->name(), reply->method(), QMetaMethod::Method, reply->handler(), reply->data()).toLatin1().data());
        sendResponse(interface, reply->clientId(), reply->commandId(), reply->data());
    } else {
        sendErrorResponse(interface, reply->clientId(), reply->commandId(), "Command timed out");
    }

    reply->deleteLater();
}

void JsonRPCServer::pairingFinished(QString cognitoUserId, int status, const QString &message)
{
    JsonReply *reply = m_pairingRequests.take(cognitoUserId);
    if (!reply) {
        return;
    }
    QVariantMap returns;
    returns.insert("status", status);
    returns.insert("message", message);
    reply->setData(returns);
    reply->finished();
}

void JsonRPCServer::onCloudConnectionStateChanged()
{
    QVariantMap params;
    params.insert("connected", NymeaCore::instance()->cloudManager()->connectionState() == CloudManager::CloudConnectionStateConnected);
    params.insert("connectionState", JsonTypes::cloudConnectionStateToString(NymeaCore::instance()->cloudManager()->connectionState()));
    emit CloudConnectedChanged(params);
}

void JsonRPCServer::onPushButtonAuthFinished(int transactionId, bool success, const QByteArray &token)
{
    QUuid clientId = m_pushButtonTransactions.take(transactionId);
    if (clientId.isNull()) {
        qCDebug(dcJsonRpc()) << "Received a PushButton reply but wasn't expecting it.";
        return;
    }

    TransportInterface *transport = m_clientTransports.value(clientId);
    if (!transport) {
        qCWarning(dcJsonRpc()) << "No transport for given clientId";
        return;
    }

    QVariantMap params;
    params.insert("transactionId", transactionId);
    params.insert("success", success);
    if (success) {
        params.insert("token", token);
    }

    QVariantMap notification;
    notification.insert("id", transactionId);
    notification.insert("notification", "JSONRPC.PushButtonAuthFinished");
    notification.insert("params", params);

    transport->sendData(clientId, QJsonDocument::fromVariant(notification).toJson(QJsonDocument::Compact));
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
    TransportInterface *interface = qobject_cast<TransportInterface *>(sender());

    m_clientTransports.insert(clientId, interface);

    // If authentication is required, notifications are disabled by default. Clients must enable them with a valid token
    m_clientNotifications.insert(clientId, !interface->configuration().authenticationEnabled);

    interface->sendData(clientId, QJsonDocument::fromVariant(createWelcomeMessage(interface)).toJson(QJsonDocument::Compact));
}

void JsonRPCServer::clientDisconnected(const QUuid &clientId)
{
    qCDebug(dcJsonRpc()) << "Client disconnected:" << clientId;
    m_clientTransports.remove(clientId);
    m_clientNotifications.remove(clientId);
    m_clientBuffers.remove(clientId);
    if (m_pushButtonTransactions.values().contains(clientId)) {
        NymeaCore::instance()->userManager()->cancelPushButtonAuth(m_pushButtonTransactions.key(clientId));
    }
}

}
