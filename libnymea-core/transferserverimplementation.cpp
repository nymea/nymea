// SPDX-License-Identifier: LGPL-3.0-or-later

#include "transferserverimplementation.h"
#include "loggingcategories.h"

#include <QJsonDocument>

namespace nymeaserver {

TransferServerImplementation::TransferServerImplementation(TransferManager *transferManager, QObject *parent) :
    QObject(parent),
    m_transferManager(transferManager)
{
}

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
    if (!m_clients.contains(clientId)) {
        return;
    }

    ClientState &state = m_clients[clientId];
    state.buffer.append(data);
    int splitIndex = state.buffer.indexOf("}\n{");
    while (splitIndex > -1) {
        processPacket(state.transport, clientId, state.buffer.left(splitIndex + 1));
        state.buffer = state.buffer.right(state.buffer.length() - splitIndex - 2);
        splitIndex = state.buffer.indexOf("}\n{");
    }

    if (state.buffer.trimmed().endsWith('}')) {
        processPacket(state.transport, clientId, state.buffer);
        state.buffer.clear();
    }
}

void TransferServerImplementation::processPacket(TransportInterface *interface, const QUuid &clientId, const QByteArray &data)
{
    QJsonParseError error;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        sendErrorResponse(interface, clientId, -1, QStringLiteral("Failed to parse transfer data"));
        return;
    }

    const QVariantMap message = jsonDoc.toVariant().toMap();
    bool success = false;
    const int commandId = message.value("id").toInt(&success);
    if (!success) {
        sendErrorResponse(interface, clientId, -1, QStringLiteral("Missing command id"));
        return;
    }

    const QString method = message.value("method").toString();
    const QVariantMap params = message.value("params").toMap();
    ClientState &state = m_clients[clientId];

    if (method == QLatin1String("Transfer.Connect")) {
        const QString transferId = params.value("transferId").toString();
        const QString transferToken = params.value("transferToken").toString();
        if (!m_transferManager->validateTransferToken(transferId, transferToken)) {
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
        sendResponse(interface, clientId, commandId, ret);
        return;
    }

    if (!state.connected || state.transferId.isEmpty()) {
        sendErrorResponse(interface, clientId, commandId, QStringLiteral("Transfer handshake required"));
        interface->terminateClientConnection(clientId);
        return;
    }

    if (method == QLatin1String("Transfer.UploadChunk")) {
        QString errorString;
        const QByteArray chunk = QByteArray::fromBase64(params.value("data").toByteArray());
        if (!m_transferManager->appendUploadData(state.transferId, chunk, &errorString)) {
            sendErrorResponse(interface, clientId, commandId, errorString);
            return;
        }

        QVariantMap ret;
        ret.insert("bytesReceived", m_transferManager->transferOffset(state.transferId));
        sendResponse(interface, clientId, commandId, ret);
        return;
    }

    if (method == QLatin1String("Transfer.FinishUpload")) {
        QString errorString;
        const auto info = m_transferManager->finishUpload(state.transferId, &errorString);
        if (info.downloadId.isEmpty()) {
            sendErrorResponse(interface, clientId, commandId, errorString);
            return;
        }

        QVariantMap ret;
        ret.insert("downloadId", info.downloadId);
        ret.insert("fileName", info.fileName);
        ret.insert("size", info.size);
        sendResponse(interface, clientId, commandId, ret);
        return;
    }

    if (method == QLatin1String("Transfer.RequestChunk")) {
        QString errorString;
        bool finished = false;
        const int chunkSize = params.value("maxBytes", 64 * 1024).toInt();
        const QByteArray chunk = m_transferManager->readDownloadChunk(state.transferId, chunkSize, &finished, &errorString);
        if (!errorString.isEmpty()) {
            sendErrorResponse(interface, clientId, commandId, errorString);
            return;
        }

        QVariantMap ret;
        ret.insert("data", chunk.toBase64());
        ret.insert("finished", finished);
        sendResponse(interface, clientId, commandId, ret);
        if (finished) {
            interface->terminateClientConnection(clientId);
        }
        return;
    }

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

}
