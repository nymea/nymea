#include "jsonrpcserver.h"
#include "jsontypes.h"

#include "tcpserver.h"
#include "jsonhandler.h"

#include "hivecore.h"
#include "devicemanager.h"
#include "deviceplugin.h"
#include "deviceclass.h"
#include "device.h"
#include "rule.h"
#include "ruleengine.h"

#include "devicehandler.h"
#include "actionhandler.h"
#include "ruleshandler.h"

#include <QJsonDocument>
#include <QStringList>

JsonRPCServer::JsonRPCServer(QObject *parent):
    QObject(parent),
    m_tcpServer(new TcpServer(this))
{
    connect(m_tcpServer, &TcpServer::jsonDataAvailable, this, &JsonRPCServer::processData);
    m_tcpServer->startServer();


    registerHandler(new DeviceHandler(this));
    registerHandler(new ActionHandler(this));
    registerHandler(new RulesHandler(this));
}

void JsonRPCServer::processData(int clientId, const QByteArray &jsonData)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &error);

    if(error.error != QJsonParseError::NoError) {
        qDebug() << "failed to parse data" << jsonData << ":" << error.errorString();
    }
    qDebug() << "-------------------------\n" << jsonDoc.toJson();

    QVariantMap message = jsonDoc.toVariant().toMap();

    bool success;
    int commandId = message.value("id").toInt(&success);
    if (!success) {
        qWarning() << "Error parsing command. Missing \"id\":" << jsonData;
        return;
    }

    QStringList commandList = message.value("method").toString().split('.');
    if (commandList.count() != 2) {
        qWarning() << "Error parsing method.\nGot:" << message.value("method").toString() << "\nExpected: \"Namespace.method\"";
        return;
    }

    QString targetNamespace = commandList.first();
    QString method = commandList.last();
    QVariantMap params = message.value("params").toMap();

    qDebug() << "got:" << targetNamespace << method << params;
    emit commandReceived(targetNamespace, method, params);

    if (targetNamespace == "JSONRPC") {
        if (method == "Introspect") {
            QVariantMap data;
            data.insert("types", JsonTypes::allTypes());
            QVariantMap methods;
            foreach (JsonHandler *handler, m_handlers) {
                qDebug() << "got handler" << handler->name() << handler->introspect();
                methods.unite(handler->introspect());
            }
            data.insert("methods", methods);
            sendResponse(clientId, commandId, data);
            return;
        }
    }

    JsonHandler *handler = m_handlers.value(targetNamespace);
    if (!handler) {
        sendErrorResponse(clientId, commandId, "No such namespace");
        return;
    }
    if (!handler->hasMethod(method)) {
        sendErrorResponse(clientId, commandId, "No such method");
        return;
    }
    if (!handler->validateParams(method, params)) {
        sendErrorResponse(clientId, commandId, "Invalid params");
        return;
    }
    QVariantMap returns;
    QMetaObject::invokeMethod(handler, method.toLatin1().data(), Q_RETURN_ARG(QVariantMap, returns), Q_ARG(QVariantMap, params));
    Q_ASSERT(handler->validateReturns(method, returns));
    sendResponse(clientId, commandId, returns);
}

void JsonRPCServer::registerHandler(JsonHandler *handler)
{
    m_handlers.insert(handler->name(), handler);
}

void JsonRPCServer::sendResponse(int clientId, int commandId, const QVariantMap &params)
{
    QVariantMap rsp;
    rsp.insert("id", commandId);
    rsp.insert("status", "success");
    rsp.insert("params", params);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(rsp);
    m_tcpServer->sendResponse(clientId, jsonDoc.toJson());
}

void JsonRPCServer::sendErrorResponse(int clientId, int commandId, const QString &error)
{
    QVariantMap rsp;
    rsp.insert("id", commandId);
    rsp.insert("status", "error");
    rsp.insert("error", error);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(rsp);
    m_tcpServer->sendResponse(clientId, jsonDoc.toJson());
}


