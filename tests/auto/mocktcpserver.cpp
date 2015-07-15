/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
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

#include "mocktcpserver.h"
#include "loggingcategories.h"
#include "jsonrpcserver.h"
#include "guhcore.h"

#include <QUuid>
#include <QHash>
#include <QJsonDocument>


namespace guhserver {

QList<MockTcpServer*> MockTcpServer::s_allServers;

MockTcpServer::MockTcpServer(QObject *parent):
    TransportInterface(parent)
{
    s_allServers.append(this);
}

MockTcpServer::~MockTcpServer()
{
    s_allServers.removeAll(this);
}

void MockTcpServer::sendData(const QUuid &clientId, const QVariantMap &data)
{
    emit outgoingData(clientId, QJsonDocument::fromVariant(data).toJson());
}

void MockTcpServer::sendData(const QList<QUuid> &clients, const QVariantMap &data)
{
    foreach (const QUuid &clientId, clients) {
        sendData(clientId, data);
    }
}

QList<MockTcpServer *> MockTcpServer::servers()
{
    return s_allServers;
}

void MockTcpServer::injectData(const QUuid &clientId, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcJsonRpc) << "Failed to parse JSON data" << data << ":" << error.errorString();
        sendErrorResponse(clientId, -1, QString("Failed to parse JSON data: %1").arg(error.errorString()));
        return;
    }

    QVariantMap message = jsonDoc.toVariant().toMap();

    bool success;
    int commandId = message.value("id").toInt(&success);
    if (!success) {
        qCWarning(dcJsonRpc) << "Error parsing command. Missing \"id\":" << message;
        sendErrorResponse(clientId, commandId, "Error parsing command. Missing 'id'");
        return;
    }

    QStringList commandList = message.value("method").toString().split('.');
    if (commandList.count() != 2) {
        qCWarning(dcJsonRpc) << "Error parsing method.\nGot:" << message.value("method").toString() << "\nExpected: \"Namespace.method\"";
        sendErrorResponse(clientId, commandId, QString("Error parsing method. Got: '%1'', Expected: 'Namespace.method'").arg(message.value("method").toString()));
        return;
    }

    QString targetNamespace = commandList.first();
    QString method = commandList.last();

    JsonHandler *handler = GuhCore::instance()->jsonRPCServer()->handlers().value(targetNamespace);
    if (!handler) {
        sendErrorResponse(clientId, commandId, "No such namespace");
        return;
    }

    if (!handler->hasMethod(method)) {
        sendErrorResponse(clientId, commandId, "No such method");
        return;
    }

    emit dataAvailable(clientId, targetNamespace, method, message);
}

void MockTcpServer::sendResponse(const QUuid &clientId, int commandId, const QVariantMap &params)
{
    QVariantMap response;
    response.insert("id", commandId);
    response.insert("status", "success");
    response.insert("params", params);

    sendData(clientId, response);
}

void MockTcpServer::sendErrorResponse(const QUuid &clientId, int commandId, const QString &error)
{
    QVariantMap errorResponse;
    errorResponse.insert("id", commandId);
    errorResponse.insert("status", "error");
    errorResponse.insert("error", error);

    sendData(clientId, errorResponse);
}

bool MockTcpServer::startServer()
{
    return true;
}

bool MockTcpServer::stopServer()
{
    return true;
}

}
