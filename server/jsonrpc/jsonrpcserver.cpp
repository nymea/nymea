/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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
#include "cloudhandler.h"

#ifndef TESTING_ENABLED
#include "tcpserver.h"
#else
#include "mocktcpserver.h"
#endif

#include <QJsonDocument>
#include <QStringList>
#include <QSslConfiguration>

namespace guhserver {

/*! Constructs a \l{JsonRPCServer} with the given \a sslConfiguration and \a parent. */
JsonRPCServer::JsonRPCServer(const QSslConfiguration &sslConfiguration, QObject *parent):
    JsonHandler(parent),
    #ifdef TESTING_ENABLED
    m_tcpServer(new MockTcpServer(this)),
    #else
    m_tcpServer(new TcpServer(this)),
    #endif
    m_websocketServer(new WebSocketServer(sslConfiguration, this)),
    m_notificationId(0)
{
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

    // Now set up the logic
    connect(m_tcpServer, SIGNAL(clientConnected(const QUuid &)), this, SLOT(clientConnected(const QUuid &)));
    connect(m_tcpServer, SIGNAL(clientDisconnected(const QUuid &)), this, SLOT(clientDisconnected(const QUuid &)));
    connect(m_tcpServer, SIGNAL(dataAvailable(QUuid, QString, QString, QVariantMap)), this, SLOT(processData(QUuid, QString, QString, QVariantMap)));
    m_tcpServer->startServer();

    m_interfaces.append(m_tcpServer);

    connect(m_websocketServer, SIGNAL(clientConnected(const QUuid &)), this, SLOT(clientConnected(const QUuid &)));
    connect(m_websocketServer, SIGNAL(clientDisconnected(const QUuid &)), this, SLOT(clientDisconnected(const QUuid &)));
    connect(m_websocketServer, SIGNAL(dataAvailable(QUuid, QString, QString, QVariantMap)), this, SLOT(processData(QUuid, QString, QString, QVariantMap)));

    m_websocketServer->startServer();
    m_interfaces.append(m_websocketServer);

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
    foreach (JsonHandler *handler, m_handlers) {
        methods.unite(handler->introspect(QMetaMethod::Method));
    }
    data.insert("methods", methods);

    QVariantMap signalsMap;
    foreach (JsonHandler *handler, m_handlers) {
        signalsMap.unite(handler->introspect(QMetaMethod::Signal));
    }
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

/*! Returns the list of registred \l{JsonHandler}{JsonHandlers} and their name.*/
QHash<QString, JsonHandler *> JsonRPCServer::handlers() const
{
    return m_handlers;
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
    registerHandler(new CloudHandler(this));
}

void JsonRPCServer::processData(const QUuid &clientId, const QString &targetNamespace, const QString &method, const QVariantMap &message)
{
    TransportInterface *interface = qobject_cast<TransportInterface *>(sender());

    // Note: id, targetNamespace and method already checked in TcpServer
    int commandId = message.value("id").toInt();
    QVariantMap params = message.value("params").toMap();

    JsonHandler *handler = m_handlers.value(targetNamespace);
    QPair<bool, QString> validationResult = handler->validateParams(method, params);
    if (!validationResult.first) {
        interface->sendErrorResponse(clientId, commandId, "Invalid params: " + validationResult.second);
        return;
    }

    // Hack: attach clientId to handler to be able to handle the JSONRPC methods. Do not use this outside of jsonrpcserver
    handler->setProperty("clientId", clientId);

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
        interface->sendResponse(clientId, commandId, reply->data());
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
        interface->sendData(m_clients.keys(true), notification);
    }
}

void JsonRPCServer::asyncReplyFinished()
{
    JsonReply *reply = qobject_cast<JsonReply *>(sender());
    TransportInterface *interface = m_asyncReplies.take(reply);
    if (!reply->timedOut()) {
        Q_ASSERT_X(reply->handler()->validateReturns(reply->method(), reply->data()).first
                   ,"validating return value", formatAssertion(reply->handler()->name(), reply->method(), reply->handler(), reply->data()).toLatin1().data());
        interface->sendResponse(reply->clientId(), reply->commandId(), reply->data());
    } else {
        interface->sendErrorResponse(reply->clientId(), reply->commandId(), "Command timed out");
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
    // Notifications enabled by default
    m_clients.insert(clientId, true);

    TransportInterface *interface = qobject_cast<TransportInterface *>(sender());

    QVariantMap handshake;
    handshake.insert("id", 0);
    handshake.insert("server", "guh JSONRPC interface");
    handshake.insert("version", GUH_VERSION_STRING);
    handshake.insert("protocol version", JSON_PROTOCOL_VERSION);
    interface->sendData(clientId, handshake);
}

void JsonRPCServer::clientDisconnected(const QUuid &clientId)
{
    m_clients.remove(clientId);
}

}
