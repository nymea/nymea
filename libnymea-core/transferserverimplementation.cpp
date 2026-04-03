// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2026, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "transferserverimplementation.h"
#include "loggingcategories.h"

#include <QJsonDocument>

namespace nymeaserver {

TransferServerImplementation::TransferServerImplementation(TransferManager *transferManager, QObject *parent)
    : QObject(parent)
    , m_transferManager(transferManager)
{}

void TransferServerImplementation::registerTransportInterface(TransportInterface *interface)
{
    connect(interface, &TransportInterface::clientConnected, this, &TransferServerImplementation::clientConnected);
    connect(interface, &TransportInterface::clientDisconnected, this, &TransferServerImplementation::clientDisconnected);
    connect(interface, &TransportInterface::dataAvailable, this, &TransferServerImplementation::processData);
}

void TransferServerImplementation::unregisterTransportInterface(TransportInterface *interface)
{
    disconnect(interface, &TransportInterface::clientConnected, this, &TransferServerImplementation::clientConnected);
    disconnect(interface, &TransportInterface::clientDisconnected, this, &TransferServerImplementation::clientDisconnected);
    disconnect(interface, &TransportInterface::dataAvailable, this, &TransferServerImplementation::processData);

    for (auto it = m_clients.begin(); it != m_clients.end();) {
        if (it->transport == interface) {
            it = m_clients.erase(it);
        } else {
            ++it;
        }
    }
}

void TransferServerImplementation::clientConnected(const QUuid &clientId)
{
    ClientState state;
    state.transport = qobject_cast<TransportInterface *>(sender());
    m_clients.insert(clientId, state);
}

void TransferServerImplementation::clientDisconnected(const QUuid &clientId)
{
    m_clients.remove(clientId);
}

void TransferServerImplementation::processData(const QUuid &clientId, const QByteArray &data)
{
    if (!m_clients.contains(clientId))
        return;

    TransportInterface *interface = qobject_cast<TransportInterface *>(sender());
    if (!interface)
        interface = m_clients.value(clientId).transport;

    QByteArray buffer = m_clients.value(clientId).buffer;
    buffer.append(data);

    int splitIndex = buffer.indexOf("}\n{");
    while (splitIndex > -1) {
        processPacket(interface, clientId, buffer.left(splitIndex + 1));
        if (!m_clients.contains(clientId))
            return;

        buffer = buffer.right(buffer.length() - splitIndex - 2);
        splitIndex = buffer.indexOf("}\n{");
    }

    if (buffer.trimmed().endsWith('}')) {
        processPacket(interface, clientId, buffer);
        if (!m_clients.contains(clientId))
            return;

        buffer.clear();
    }

    m_clients[clientId].buffer = buffer;
}

void TransferServerImplementation::processPacket(TransportInterface *interface, const QUuid &clientId, const QByteArray &data)
{
    if (!interface)
        return;

    qCDebug(dcTransferTraffic()) << "Incoming data from interface" << interface->configuration() << clientId.toString() << data;

    QJsonParseError error;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcJsonRpc()) << "Failed to parse transfer json data data" << error.errorString();
        sendErrorResponse(interface, clientId, -1, QStringLiteral("Failed to parse transfer data"));
        return;
    }

    const QVariantMap message = jsonDoc.toVariant().toMap();
    bool success = false;
    const int commandId = message.value("id").toInt(&success);
    if (!success) {
        qCWarning(dcJsonRpc()) << "Error parsing command. Missing command \"id\":" << message;
        sendErrorResponse(interface, clientId, -1, QStringLiteral("Missing command id"));
        return;
    }

    const QString method = message.value("method").toString();
    const QVariantMap params = message.value("params").toMap();
    auto clientIt = m_clients.find(clientId);
    if (clientIt == m_clients.end())
        return;

    ClientState &state = clientIt.value();
    if (method == QLatin1String("Transfer.Connect")) {
        const QString transferId = params.value("transferId").toString();
        const QString transferToken = params.value("transferToken").toString();
        if (!m_transferManager->validateTransferToken(transferId, transferToken)) {
            qCWarning(dcTransfer()) << "Invalid transfer token from" << clientId.toString() << "Closing client connection.";
            sendErrorResponse(interface, clientId, commandId, QStringLiteral("Invalid transfer token"));
            interface->terminateClientConnection(clientId);
            return;
        }

        state.transferId = transferId;
        state.connected = true;

        QVariantMap ret;
        ret.insert("direction", m_transferManager->transferDirection(transferId) == TransferManager::Direction::Upload ? "upload" : "download");
        ret.insert("fileName", m_transferManager->transferFileName(transferId));
        ret.insert("size", m_transferManager->transferSize(transferId));
        ret.insert("offset", m_transferManager->transferOffset(transferId));
        qCDebug(dcTransfer()) << "Transfer connected:" << qUtf8Printable(QJsonDocument::fromVariant(ret).toJson());
        sendResponse(interface, clientId, commandId, ret);
        return;
    }

    if (!state.connected || state.transferId.isEmpty()) {
        qCWarning(dcTransfer()) << "Transfer handshake missing from client" << clientId.toString() << "Closing client connection.";
        sendErrorResponse(interface, clientId, commandId, QStringLiteral("Transfer handshake required"));
        interface->terminateClientConnection(clientId);
        return;
    }

    if (method == QLatin1String("Transfer.UploadChunk")) {
        QString errorString;
        const QByteArray chunk = QByteArray::fromBase64(params.value("data").toByteArray());
        if (!m_transferManager->appendUploadData(state.transferId, chunk, &errorString)) {
            qCWarning(dcTransfer()) << "Error occurred in" << method << errorString;
            sendErrorResponse(interface, clientId, commandId, errorString);
            return;
        }

        QVariantMap ret;
        qCDebug(dcTransfer()) << "Transfer upload chunk received:" << m_transferManager->transferOffset(state.transferId) << "/"
                              << m_transferManager->transferSize(state.transferId);
        ret.insert("bytesReceived", m_transferManager->transferOffset(state.transferId));
        sendResponse(interface, clientId, commandId, ret);
        return;
    }

    if (method == QLatin1String("Transfer.FinishUpload")) {
        QString errorString;
        const auto info = m_transferManager->finishUpload(state.transferId, &errorString);
        if (info.downloadId.isEmpty() && !info.restoreTriggered) {
            qCWarning(dcTransfer()) << "Error occurred in" << method << errorString;
            sendErrorResponse(interface, clientId, commandId, errorString);
            return;
        }

        QVariantMap ret;
        ret.insert("fileName", info.fileName);
        ret.insert("size", info.size);
        if (!info.downloadId.isEmpty())
            ret.insert("downloadId", info.downloadId);

        if (info.restoreTriggered)
            ret.insert("restoreTriggered", true);

        qCDebug(dcTransfer()) << "Transfer donwload finished:" << info.fileName << info.size;
        sendResponse(interface, clientId, commandId, ret);
        return;
    }

    if (method == QLatin1String("Transfer.RequestChunk")) {
        QString errorString;
        bool finished = false;
        const int chunkSize = params.value("maxBytes", 64 * 1024).toInt();
        const QByteArray chunk = m_transferManager->readDownloadChunk(state.transferId, chunkSize, &finished, &errorString);
        if (!errorString.isEmpty()) {
            qCWarning(dcTransfer()) << "Error occurred in" << method << errorString;
            sendErrorResponse(interface, clientId, commandId, errorString);
            return;
        }

        qCDebug(dcTransfer()) << "Transfer download chunk:" << chunk.size();

        QVariantMap ret;
        ret.insert("data", chunk.toBase64());
        ret.insert("finished", finished);
        sendResponse(interface, clientId, commandId, ret);
        if (finished)
            interface->terminateClientConnection(clientId);

        return;
    }

    qCWarning(dcTransfer()) << "Unknwon transfere method received" << method;
    sendErrorResponse(interface, clientId, commandId, QStringLiteral("Unknown transfer method"));
}

void TransferServerImplementation::sendResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QVariantMap &params)
{
    QVariantMap response;
    response.insert("id", commandId);
    response.insert("status", "success");
    response.insert("params", params);
    interface->sendData(clientId, QJsonDocument::fromVariant(response).toJson(QJsonDocument::Compact));
}

void TransferServerImplementation::sendErrorResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error)
{
    QVariantMap response;
    response.insert("id", commandId);
    response.insert("status", "error");
    response.insert("error", error);
    interface->sendData(clientId, QJsonDocument::fromVariant(response).toJson(QJsonDocument::Compact));
}

} // namespace nymeaserver
