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


#include "jsonrpcserverimplementation.h"
#include "jsonrpc/jsonhandler.h"
#include "jsonvalidator.h"
#include "nymeacore.h"
#include "devices/devicemanager.h"
#include "devices/deviceplugin.h"
#include "devices/device.h"
#include "types/deviceclass.h"
#include "ruleengine/rule.h"
#include "ruleengine/ruleengine.h"
#include "loggingcategories.h"
#include "platform/platform.h"

#include "devicehandler.h"
#include "actionhandler.h"
#include "ruleshandler.h"
#include "eventhandler.h"
#include "logginghandler.h"
#include "statehandler.h"
#include "configurationhandler.h"
#include "networkmanagerhandler.h"
#include "tagshandler.h"
#include "systemhandler.h"

#include <QJsonDocument>
#include <QStringList>
#include <QSslConfiguration>

namespace nymeaserver {

/*! Constructs a \l{JsonRPCServer} with the given \a sslConfiguration and \a parent. */
JsonRPCServerImplementation::JsonRPCServerImplementation(const QSslConfiguration &sslConfiguration, QObject *parent):
    JsonHandler(parent),
    m_notificationId(0)
{
    Q_UNUSED(sslConfiguration)
    // First, define our own JSONRPC API

    // Enums
    registerEnum<BasicType>();
    registerEnum<UserManager::UserError>();
    registerEnum<CloudManager::CloudConnectionState>();

    // Objects
    QVariantMap tokenInfo;
    tokenInfo.insert("id", enumValueName(Uuid));
    tokenInfo.insert("userName", enumValueName(String));
    tokenInfo.insert("deviceName", enumValueName(String));
    tokenInfo.insert("creationTime", enumValueName(Uint));
    registerObject("TokenInfo", tokenInfo);

    // Methods
    QString description; QVariantMap returns; QVariantMap params;
    description = "Initiates a connection. Use this method to perform an initial handshake of the "
                            "connection. Optionally, a parameter \"locale\" is can be passed to set up the used "
                            "locale for this connection. Strings such as DeviceClass displayNames etc will be "
                            "localized to this locale. If this parameter is omitted, the default system locale "
                            "(depending on the configuration) is used. The reply of this method contains information "
                            "about this core instance such as version information, uuid and its name. The locale value"
                            "indicates the locale used for this connection. Note: This method can be called multiple "
                            "times. The locale used in the last call for this connection will be used. Other values, "
                            "like initialSetupRequired might change if the setup has been performed in the meantime.";
    params.insert("o:locale", enumValueName(String));
    returns.insert("server", enumValueName(String));
    returns.insert("name", enumValueName(String));
    returns.insert("version", enumValueName(String));
    returns.insert("uuid", enumValueName(Uuid));
    returns.insert("language", enumValueName(String));
    returns.insert("locale", enumValueName(String));
    returns.insert("protocol version", enumValueName(String));
    returns.insert("initialSetupRequired", enumValueName(Bool));
    returns.insert("authenticationRequired", enumValueName(Bool));
    returns.insert("pushButtonAuthAvailable", enumValueName(Bool));
    registerMethod("Hello", description, params, returns);

    params.clear(); returns.clear();
    description = "Introspect this API.";
    returns.insert("methods", enumValueName(Object));
    returns.insert("notifications", enumValueName(Object));
    returns.insert("types", enumValueName(Object));
    registerMethod("Introspect", description, params, returns);

    params.clear(); returns.clear();
    description = "Version of this nymea/JSONRPC interface.";
    returns.insert("version", enumValueName(String));
    returns.insert("protocol version", enumValueName(String));
    registerMethod("Version", description, params, returns);

    params.clear(); returns.clear();
    description = "Enable/Disable notifications for this connections. Either \"enabled\" or """
                                            "\"namespaces\" needs to be given but not both of them. The boolean based "
                                            "\"enabled\" parameter will enable/disable all notifications at once. If "
                                            "instead the list-based \"namespaces\" parameter is provided, all given namespaces"
                                            "will be enabled, the others will be disabled. The return value of \"success\" will "
                                            "indicate success of the operation. The \"enabled\" property in the return value is "
                                            "deprecated and used for legacy compatibilty only. It will be set to true if at least "
                                            "one namespace has been enabled.";
    params.insert("o:namespaces", enumValueName(StringList));
    params.insert("o:enabled", enumValueName(Bool));
    returns.insert("namespaces", enumValueName(StringList));
    returns.insert("enabled", enumValueName(Bool));
    registerMethod("SetNotificationStatus", description, params, returns);

    params.clear(); returns.clear();
    description = "Create a new user in the API. Currently this is only allowed to be called once when a new nymea instance is set up. Call Authenticate after this to obtain a device token for this user.";
    params.insert("username", enumValueName(String));
    params.insert("password", enumValueName(String));
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("CreateUser", description, params, returns);

    params.clear(); returns.clear();
    description = "Authenticate a client to the api via user & password challenge. Provide "
                   "a device name which allows the user to identify the client and revoke the token in case "
                   "the device is lost or stolen. This will return a new token to be used to authorize a "
                   "client at the API.";
    params.insert("username", enumValueName(String));
    params.insert("password", enumValueName(String));
    params.insert("deviceName", enumValueName(String));
    returns.insert("success", enumValueName(Bool));
    returns.insert("o:token", enumValueName(String));
    registerMethod("Authenticate", description, params, returns);

    params.clear(); returns.clear();
    description = "Authenticate a client to the api via Push Button method. "
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
                   "over and snooping in for tokens.";
    params.insert("deviceName", enumValueName(String));
    returns.insert("success", enumValueName(Bool));
    returns.insert("transactionId", enumValueName(Int));
    registerMethod("RequestPushButtonAuth", description, params, returns);

    params.clear(); returns.clear();
    description = "Return a list of TokenInfo objects of all the tokens for the current user.";
    returns.insert("tokenInfoList", QVariantList() << objectRef("TokenInfo"));
    registerMethod("Tokens", description, params, returns);

    params.clear(); returns.clear();
    description = "Revoke access for a given token.";
    params.insert("tokenId", enumValueName(Uuid));
    returns.insert("error", enumRef<UserManager::UserError>());
    registerMethod("RemoveToken", description, params, returns);

    params.clear(); returns.clear();
    description = "Sets up the cloud connection by deploying a certificate and its configuration.";
    params.insert("rootCA", enumValueName(String));
    params.insert("certificatePEM", enumValueName(String));
    params.insert("publicKey", enumValueName(String));
    params.insert("privateKey", enumValueName(String));
    params.insert("endpoint", enumValueName(String));
    returns.insert("success", enumValueName(Bool));
    registerMethod("SetupCloudConnection", description, params, returns);

    params.clear(); returns.clear();
    description = "Setup the remote connection by providing AWS token information. This requires the cloud to be connected.";
    params.insert("idToken", enumValueName(String));
    params.insert("userId", enumValueName(String));
    returns.insert("status", enumValueName(Int));
    returns.insert("message", enumValueName(String));
    registerMethod("SetupRemoteAccess", description, params, returns);

    params.clear(); returns.clear();
    description = "Check whether the cloud is currently connected. \"connected\" will be true whenever connectionState equals CloudConnectionStateConnected and is deprecated. Please use the connectionState value instead.";
    returns.insert("connected", enumValueName(Bool));
    returns.insert("connectionState", enumRef<CloudManager::CloudConnectionState>());
    registerMethod("IsCloudConnected", description, params, returns);

    params.clear(); returns.clear();
    description = "This is basically a Ping/Pong mechanism a client app may use to check server connectivity. Currently, the server does not actually do anything with this information and will return the call providing the given sessionId back to the caller. It is up to the client whether to use this or not and not required by the server to keep the connection alive.";
    params.insert("sessionId", enumValueName(String));
    returns.insert("success", enumValueName(Bool));
    returns.insert("sessionId", enumValueName(String));
    registerMethod("KeepAlive", description, params, returns);

    // Notifications
    params.clear(); returns.clear();
    description = "Emitted whenever the cloud connection status changes.";
    params.insert("connected", enumValueName(Bool));
    params.insert("connectionState", enumRef<CloudManager::CloudConnectionState>());
    registerNotification("CloudConnectedChanged", description, params);

    params.clear();
    description = "Emitted when a push button authentication reaches final state. NOTE: This notification is special. It will only be emitted to connections that did actively request a push button authentication, but also it will be emitted regardless of the notification settings. ";
    params.insert("success", enumValueName(Bool));
    params.insert("transactionId", enumValueName(Int));
    params.insert("o:token", enumValueName(String));
    registerNotification("PushButtonAuthFinished", description, params);

    QMetaObject::invokeMethod(this, "setup", Qt::QueuedConnection);

    connect(NymeaCore::instance()->userManager(), &UserManager::pushButtonAuthFinished, this, &JsonRPCServerImplementation::onPushButtonAuthFinished);
}

/*! Returns the \e namespace of \l{JsonHandler}. */
QString JsonRPCServerImplementation::name() const
{
    return QStringLiteral("JSONRPC");
}

JsonReply *JsonRPCServerImplementation::Hello(const QVariantMap &params)
{
    TransportInterface *interface = reinterpret_cast<TransportInterface*>(property("transportInterface").toLongLong());

    qCDebug(dcJsonRpc()) << params;
    QUuid clientId = this->property("clientId").toUuid();
    if (params.contains("locale")) {
        m_clientLocales.insert(clientId, QLocale(params.value("locale").toString()));
    }

    qCDebug(dcJsonRpc()) << "Client" << clientId << "initiated handshake." << m_clientLocales.value(clientId);

    // If we waited for the handshake, here it is. Remove the timer...
    if (m_newConnectionWaitTimers.contains(clientId)) {
        delete m_newConnectionWaitTimers.take(clientId);
    }

    return createReply(createWelcomeMessage(interface, clientId));
}

JsonReply* JsonRPCServerImplementation::Introspect(const QVariantMap &params) const
{
    Q_UNUSED(params)
    return createReply(m_api);
}

JsonReply* JsonRPCServerImplementation::Version(const QVariantMap &params) const
{
    Q_UNUSED(params)

    QVariantMap data;
    data.insert("version", NYMEA_VERSION_STRING);
    data.insert("protocol version", JSON_PROTOCOL_VERSION);
    return createReply(data);
}

JsonReply* JsonRPCServerImplementation::SetNotificationStatus(const QVariantMap &params)
{
    QUuid clientId = this->property("clientId").toUuid();
    Q_ASSERT_X(m_clientTransports.contains(clientId), "JsonRPCServer", "Invalid client ID.");

    QStringList enabledNamespaces;
    foreach (const QString &namespaceName, m_handlers.keys()) {
        if (params.contains("enabled")) {
            if (params.value("enabled").toBool()) {
                enabledNamespaces.append(namespaceName);
            }
        } else {
            if (params.value("namespaces").toList().contains(namespaceName)) {
                enabledNamespaces.append(namespaceName);
            }
        }
    }
    qCDebug(dcJsonRpc()) << "Notification settings for client" << clientId << ":" << enabledNamespaces;
    m_clientNotifications[clientId] = enabledNamespaces;

    QVariantMap returns;
    returns.insert("namespaces", m_clientNotifications[clientId]);
    // legacy, deprecated
    returns.insert("enabled", m_clientNotifications[clientId].count() > 0);
    return createReply(returns);
}

JsonReply *JsonRPCServerImplementation::CreateUser(const QVariantMap &params)
{
    QString username = params.value("username").toString();
    QString password = params.value("password").toString();

    UserManager::UserError status = NymeaCore::instance()->userManager()->createUser(username, password);

    QVariantMap returns;
    returns.insert("error", enumValueName<UserManager::UserError>(status));
    return createReply(returns);
}

JsonReply *JsonRPCServerImplementation::Authenticate(const QVariantMap &params)
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

JsonReply *JsonRPCServerImplementation::RequestPushButtonAuth(const QVariantMap &params)
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

JsonReply *JsonRPCServerImplementation::Tokens(const QVariantMap &params) const
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
        retList << packTokenInfo(tokenInfo);
    }
    QVariantMap retMap;
    retMap.insert("tokenInfoList", retList);
    return createReply(retMap);
}

JsonReply *JsonRPCServerImplementation::RemoveToken(const QVariantMap &params)
{
    QUuid tokenId = params.value("tokenId").toUuid();
    UserManager::UserError error = NymeaCore::instance()->userManager()->removeToken(tokenId);
    QVariantMap ret;
    ret.insert("error", enumValueName<UserManager::UserError>(error));
    return createReply(ret);
}

JsonReply *JsonRPCServerImplementation::SetupCloudConnection(const QVariantMap &params)
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

JsonReply *JsonRPCServerImplementation::SetupRemoteAccess(const QVariantMap &params)
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

JsonReply *JsonRPCServerImplementation::IsCloudConnected(const QVariantMap &params)
{
    Q_UNUSED(params)
    bool connected = NymeaCore::instance()->cloudManager()->connectionState() == CloudManager::CloudConnectionStateConnected;
    QVariantMap data;
    data.insert("connected", connected);
    data.insert("connectionState", enumValueName<CloudManager::CloudConnectionState>(NymeaCore::instance()->cloudManager()->connectionState()));
    return createReply(data);
}

/*! A client may use this as a ping/pong mechanism to check server connectivity. */
JsonReply *JsonRPCServerImplementation::KeepAlive(const QVariantMap &params)
{
    QString sessionId = params.value("sessionId").toString();
    qCDebug(dcJsonRpc()) << "KeepAlive received" << sessionId;
    QVariantMap resultMap;
    resultMap.insert("success", true);
    resultMap.insert("sessionId", sessionId);
    return createReply(resultMap);
}

/*! Returns the list of registered \l{JsonHandler}{JsonHandlers} and their name.*/
QHash<QString, JsonHandler *> JsonRPCServerImplementation::handlers() const
{
    return m_handlers;
}

/*! Register a new \l{TransportInterface} to the JSON server. If the given interface is already registered, just the authenticationRequired flag will be updated. */
void JsonRPCServerImplementation::registerTransportInterface(TransportInterface *interface, bool authenticationRequired)
{
    if (!m_interfaces.contains(interface)) {
        connect(interface, &TransportInterface::clientConnected, this, &JsonRPCServerImplementation::clientConnected);
        connect(interface, &TransportInterface::clientDisconnected, this, &JsonRPCServerImplementation::clientDisconnected);
        connect(interface, &TransportInterface::dataAvailable, this, &JsonRPCServerImplementation::processData);
        m_interfaces.insert(interface, authenticationRequired);
    } else {
        m_interfaces[interface] = authenticationRequired;
    }
}

void JsonRPCServerImplementation::unregisterTransportInterface(TransportInterface *interface)
{
    disconnect(interface, &TransportInterface::clientConnected, this, &JsonRPCServerImplementation::clientConnected);
    disconnect(interface, &TransportInterface::clientDisconnected, this, &JsonRPCServerImplementation::clientDisconnected);
    disconnect(interface, &TransportInterface::dataAvailable, this, &JsonRPCServerImplementation::processData);
    m_interfaces.take(interface);
}

/*! Send a JSON success response to the client with the given \a clientId,
 * \a commandId and \a params to the inerted \l{TransportInterface}.
 */
void JsonRPCServerImplementation::sendResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QVariantMap &params)
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
void JsonRPCServerImplementation::sendErrorResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error)
{
    QVariantMap errorResponse;
    errorResponse.insert("id", commandId);
    errorResponse.insert("status", "error");
    errorResponse.insert("error", error);

    QByteArray data = QJsonDocument::fromVariant(errorResponse).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpcTraffic()) << "Sending data:" << data;
    interface->sendData(clientId, data);
}

void JsonRPCServerImplementation::sendUnauthorizedResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error)
{
    QVariantMap errorResponse;
    errorResponse.insert("id", commandId);
    errorResponse.insert("status", "unauthorized");
    errorResponse.insert("error", error);

    QByteArray data = QJsonDocument::fromVariant(errorResponse).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpcTraffic()) << "Sending data:" << data;
    interface->sendData(clientId, data);
}

QVariantMap JsonRPCServerImplementation::createWelcomeMessage(TransportInterface *interface, const QUuid &clientId) const
{
    QVariantMap handshake;
    handshake.insert("server", "nymea");
    handshake.insert("name", NymeaCore::instance()->configuration()->serverName());
    handshake.insert("version", NYMEA_VERSION_STRING);
    handshake.insert("uuid", NymeaCore::instance()->configuration()->serverUuid().toString());
    // "language" is deprecated
    handshake.insert("language", m_clientLocales.value(clientId).name());
    handshake.insert("locale", m_clientLocales.value(clientId).name());
    handshake.insert("protocol version", JSON_PROTOCOL_VERSION);
    handshake.insert("initialSetupRequired", (interface->configuration().authenticationEnabled ? NymeaCore::instance()->userManager()->initRequired() : false));
    handshake.insert("authenticationRequired", interface->configuration().authenticationEnabled);
    handshake.insert("pushButtonAuthAvailable", NymeaCore::instance()->userManager()->pushButtonAuthAvailable());
    return handshake;
}

void JsonRPCServerImplementation::setup()
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
    registerHandler(new SystemHandler(NymeaCore::instance()->platform(), this));

    connect(NymeaCore::instance()->cloudManager(), &CloudManager::pairingReply, this, &JsonRPCServerImplementation::pairingFinished);
    connect(NymeaCore::instance()->cloudManager(), &CloudManager::connectionStateChanged, this, &JsonRPCServerImplementation::onCloudConnectionStateChanged);
}

void JsonRPCServerImplementation::processData(const QUuid &clientId, const QByteArray &data)
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

void JsonRPCServerImplementation::processJsonPacket(TransportInterface *interface, const QUuid &clientId, const QByteArray &data)
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
                qCWarning(dcJsonRpc()) << "Initial setup required but client does not call the setup. Dropping connection.";
                interface->terminateClientConnection(clientId);
                return;
            }
        } else {
            // ok, we have a user. if there isn't a valid token, let's fail unless this is a Authenticate, Introspect  Hello call
            if (!(targetNamespace == "JSONRPC" && authExemptMethodsWithUser.contains(method)) && (token.isEmpty() || !NymeaCore::instance()->userManager()->verifyToken(token))) {
                sendUnauthorizedResponse(interface, clientId, commandId, "Forbidden: Invalid token.");
                qCWarning(dcJsonRpc()) << "Client did not not present a valid token. Dropping connection.";
                interface->terminateClientConnection(clientId);
                return;
            }
        }
    }
    // At this point we can assume all the calls are authorized

    JsonHandler *handler = m_handlers.value(targetNamespace);
    if (!handler) {
        qCWarning(dcJsonRpc()) << "JSON RPC method called for invalid namespace:" << targetNamespace;
        sendErrorResponse(interface, clientId, commandId, "No such namespace");
        return;
    }
    if (!handler->jsonMethods().contains(method)) {
        qCWarning(dcJsonRpc()) << QString("JSON RPC method called for invalid method: %1.%2").arg(targetNamespace).arg(method);
        sendErrorResponse(interface, clientId, commandId, "No such method");
        return;
    }

    QVariantMap params = message.value("params").toMap();

    QVariantMap definition = handler->jsonMethods().value(method).toMap().value("params").toMap();
    JsonValidator validator;
    JsonValidator::Result validationResult = validator.validateParams(params, targetNamespace + '.' + method, m_api);
    if (!validationResult.success()) {
        qCWarning(dcJsonRpc()) << "JSON RPC parameter verification failed for method" << targetNamespace + '.' + method;
        qCWarning(dcJsonRpc()) << validationResult.errorString() << "in" << validationResult.where();
        sendErrorResponse(interface, clientId, commandId, "Invalid params: " + validationResult.errorString() + " in " + validationResult.where());
        return;
    }

    // Hack: attach some properties to the handler to be able to handle the JSONRPC methods. Do not use this outside of jsonrpcserver
    handler->setProperty("clientId", clientId);
    handler->setProperty("token", message.value("token").toByteArray());
    handler->setProperty("transportInterface", reinterpret_cast<qint64>(interface));

    qCDebug(dcJsonRpc()) << "Invoking method" << targetNamespace << method.toLatin1().data();

    if (!(targetNamespace == "JSONRPC" && method == "Hello")) {
        // This is not the handshake message. If we've waited for it, consider this a protocol violation and drop connection
        if (m_newConnectionWaitTimers.contains(clientId)) {
            sendErrorResponse(interface, clientId, commandId, "Handshake required. Call JSONRPC.Hello first.");
            qCWarning(dcJsonRpc()) << "Connection requires a handshake but client did not initiate handshake. Dropping connection";
            interface->terminateClientConnection(clientId);
            return;
        }


        // Unless this is the Hello message, which allows setting the locale explicitly, attach the locale
        // for this connection
        // If the client did request a locale in the Hello message, use that locale
        params.insert("locale", m_clientLocales.value(clientId));
    }

    JsonReply *reply;
    QMetaObject::invokeMethod(handler, method.toLatin1().data(), Q_RETURN_ARG(JsonReply*, reply), Q_ARG(QVariantMap, params));
    if (reply->type() == JsonReply::TypeAsync) {
        m_asyncReplies.insert(reply, interface);
        reply->setClientId(clientId);
        reply->setCommandId(commandId);
        connect(reply, &JsonReply::finished, this, &JsonRPCServerImplementation::asyncReplyFinished);
        reply->startWait();
    } else {
        JsonValidator validator;
        Q_ASSERT_X((targetNamespace == "JSONRPC" && method == "Introspect") || validator.validateReturns(reply->data(), targetNamespace + '.' + method, m_api).success(),
                   validator.result().where().toUtf8(),
                   validator.result().errorString().toUtf8() + "\nReturn value:\n" + QJsonDocument::fromVariant(reply->data()).toJson());
        sendResponse(interface, clientId, commandId, reply->data());
        reply->deleteLater();
    }
}

QVariantMap JsonRPCServerImplementation::packTokenInfo(const TokenInfo &tokenInfo)
{
    QVariantMap ret;
    ret.insert("id", tokenInfo.id().toString());
    ret.insert("userName", tokenInfo.username());
    ret.insert("deviceName", tokenInfo.deviceName());
    ret.insert("creationTime", tokenInfo.creationTime().toTime_t());
    return ret;
}

void JsonRPCServerImplementation::sendNotification(const QVariantMap &params)
{
    JsonHandler *handler = qobject_cast<JsonHandler *>(sender());
    QMetaMethod method = handler->metaObject()->method(senderSignalIndex());

    QVariantMap notification;
    notification.insert("id", m_notificationId++);
    notification.insert("notification", handler->name() + "." + method.name());
    notification.insert("params", params);

    JsonValidator validator;
    Q_ASSERT_X(validator.validateNotificationParams(params, handler->name() + '.' + method.name(), m_api).success(),
               validator.result().where().toUtf8(),
               validator.result().errorString().toUtf8());
    QByteArray data = QJsonDocument::fromVariant(notification).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpc()) << "Sending notification:" << handler->name() + "." + method.name();
    qCDebug(dcJsonRpcTraffic()) << "Notification content:" << data;

    foreach (const QUuid &clientId, m_clientNotifications.keys()) {
        if (m_clientNotifications.value(clientId).contains(handler->name())) {
            m_clientTransports.value(clientId)->sendData(clientId, data);
        }
    }
}

void JsonRPCServerImplementation::asyncReplyFinished()
{
    JsonReply *reply = qobject_cast<JsonReply *>(sender());
    TransportInterface *interface = m_asyncReplies.take(reply);
    if (!interface) {
        qCWarning(dcJsonRpc()) << "Got an async reply but the requesting connection has vanished.";
        reply->deleteLater();
        return;
    }
    if (!reply->timedOut()) {
        JsonValidator validator;
        Q_ASSERT_X(validator.validateReturns(reply->data(), reply->handler()->name() + '.' + reply->method(), m_api).success()
                   ,validator.result().where().toUtf8()
                   ,validator.result().errorString().toUtf8());
        sendResponse(interface, reply->clientId(), reply->commandId(), reply->data());
    } else {
        qCWarning(dcJsonRpc()) << "RPC call timed out:" << reply->handler()->name() << ":" << reply->method();
        sendErrorResponse(interface, reply->clientId(), reply->commandId(), "Command timed out");
    }

    reply->deleteLater();
}

void JsonRPCServerImplementation::pairingFinished(QString cognitoUserId, int status, const QString &message)
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

void JsonRPCServerImplementation::onCloudConnectionStateChanged()
{
    QVariantMap params;
    params.insert("connected", NymeaCore::instance()->cloudManager()->connectionState() == CloudManager::CloudConnectionStateConnected);
    params.insert("connectionState", enumValueName<CloudManager::CloudConnectionState>(NymeaCore::instance()->cloudManager()->connectionState()));
    emit CloudConnectedChanged(params);
}

void JsonRPCServerImplementation::onPushButtonAuthFinished(int transactionId, bool success, const QByteArray &token)
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

bool JsonRPCServerImplementation::registerHandler(JsonHandler *handler)
{
    // Sanity checks on API:
    // * Make sure all $ref: entries are valid. A Handler can reference Types from previously loaded handlers or own ones.
    // * A handler must not register a type name that is already registered by a previously loaded handler.
    QVariantMap types = m_api.value("types").toMap();
    QVariantMap methods = m_api.value("methods").toMap();
    QVariantMap notifications = m_api.value("notifications").toMap();

    // Verify enums name clash
    foreach (const QString &enumName, handler->jsonEnums().keys()) {
        QVariantList list = handler->jsonEnums().value(enumName).toList();
        if (types.contains(enumName)) {
            qCWarning(dcJsonRpc()) << "Enum type" << enumName << "is already registered. Not registering handler" << handler->name();
            return false;
        }
        types.insert(enumName, list);
    }

    // Verify objects
    QVariantMap typesIncludingThis = types;
    typesIncludingThis.unite(handler->jsonObjects());
    foreach (const QString &objectName, handler->jsonObjects().keys()) {
        QVariantMap object = handler->jsonObjects().value(objectName).toMap();
        // Check for name clashes
        if (types.contains(objectName)) {
            qCWarning(dcJsonRpc()) << "Object type" << objectName << "is already registered. Not registering handler" << handler->name();
            return false;
        }
        // Check for invalid $ref: entries
        if (!JsonValidator::checkRefs(object, typesIncludingThis)) {
            qCWarning(dcJsonRpc()).nospace() << "Invalid reference in object type " << objectName << ". Not registering handler " << handler->name();
            return false;
        }
    }
    types = typesIncludingThis;

    // Verify methods
    QVariantMap newMethods;
    foreach (const QString &methodName, handler->jsonMethods().keys()) {
        QVariantMap method = handler->jsonMethods().value(methodName).toMap();
        if (handler->metaObject()->indexOfMethod(methodName.toUtf8() + "(QVariantMap)") < 0) {
            qCWarning(dcJsonRpc()).nospace().noquote() << "Invalid method \"" << methodName << "\". Method \"JsonReply* " + methodName + "(QVariantMap)\" does not exist. Not registering handler " << handler->name();
            return false;
        }
        if (!JsonValidator::checkRefs(method.value("params").toMap(), types)) {
            qCWarning(dcJsonRpc()).nospace() << "Invalid reference in params of method " << methodName << ". Not registering handler " << handler->name();
            return false;
        }
        if (!JsonValidator::checkRefs(method.value("returns").toMap(), types)) {
            qCWarning(dcJsonRpc()).nospace() << "Invalid reference in return value of method " << methodName << ". Not registering handler " << handler->name();
            return false;
        }
        newMethods.insert(handler->name() + '.' + methodName, method);
    }
    methods.unite(newMethods);

    // Verify notifications
    QVariantMap newNotifications;
    foreach (const QString &notificationName, handler->jsonNotifications().keys()) {
        QVariantMap notification = handler->jsonNotifications().value(notificationName).toMap();
        if (!JsonValidator::checkRefs(notification.value("params").toMap(), types)) {
            qCWarning(dcJsonRpc()).nospace() << "Invalid reference in params of notification " << notificationName << ". Not registering handler " << handler->name();
            return false;
        }
        newNotifications.insert(handler->name() + '.' + notificationName, notification);
    }
    notifications.unite(newNotifications);

    // Checks completed. Store new API
    qCDebug(dcJsonRpc()) << "Registering JSON RPC handler:" << handler->name();
    m_api["types"] = types;
    m_api["methods"] = methods;
    m_api["notifications"] = notifications;

    m_handlers.insert(handler->name(), handler);
    for (int i = 0; i < handler->metaObject()->methodCount(); ++i) {
        QMetaMethod method = handler->metaObject()->method(i);
        if (method.methodType() == QMetaMethod::Signal && QString(method.name()).contains(QRegExp("^[A-Z]"))) {
            QObject::connect(handler, method, this, metaObject()->method(metaObject()->indexOfSlot("sendNotification(QVariantMap)")));
        }
    }
    return true;
}

void JsonRPCServerImplementation::clientConnected(const QUuid &clientId)
{
    qCDebug(dcJsonRpc()) << "Client connected with uuid" << clientId.toString();
    TransportInterface *interface = qobject_cast<TransportInterface *>(sender());

    m_clientTransports.insert(clientId, interface);

    // Initialize the connection locale to the settings default
    m_clientLocales.insert(clientId, NymeaCore::instance()->configuration()->locale());

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, timer, clientId, interface](){
        qCDebug(dcJsonRpc()) << "Client" << clientId << "did not initiate the handshake within the required timeout. Dropping connection.";
        timer->deleteLater();
        m_newConnectionWaitTimers.remove(clientId);
        interface->terminateClientConnection(clientId);
    });
    m_newConnectionWaitTimers.insert(clientId, timer);
    timer->start(10000);
}

void JsonRPCServerImplementation::clientDisconnected(const QUuid &clientId)
{
    qCDebug(dcJsonRpc()) << "Client disconnected:" << clientId;
    m_clientTransports.remove(clientId);
    m_clientNotifications.remove(clientId);
    m_clientBuffers.remove(clientId);
    m_clientLocales.remove(clientId);
    if (m_pushButtonTransactions.values().contains(clientId)) {
        NymeaCore::instance()->userManager()->cancelPushButtonAuth(m_pushButtonTransactions.key(clientId));
    }
    if (m_newConnectionWaitTimers.contains(clientId)) {
        delete m_newConnectionWaitTimers.take(clientId);
    }
}

}
