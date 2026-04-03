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

#include "transportrouter.h"
#include "loggingcategories.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace nymeaserver {

RoutedTransportInterface::RoutedTransportInterface(TransportInterface *backend, QObject *parent)
    : TransportInterface(backend->configuration(), parent)
    , m_backend(backend)
{}

void RoutedTransportInterface::sendData(const QUuid &clientId, const QByteArray &data)
{
    m_backend->sendData(clientId, data);
}

void RoutedTransportInterface::sendData(const QList<QUuid> &clients, const QByteArray &data)
{
    m_backend->sendData(clients, data);
}

void RoutedTransportInterface::terminateClientConnection(const QUuid &clientId)
{
    m_backend->terminateClientConnection(clientId);
}

bool RoutedTransportInterface::startServer()
{
    return true;
}

bool RoutedTransportInterface::stopServer()
{
    return true;
}

void RoutedTransportInterface::forwardClientConnected(const QUuid &clientId)
{
    emit clientConnected(clientId);
}

void RoutedTransportInterface::forwardClientDisconnected(const QUuid &clientId)
{
    emit clientDisconnected(clientId);
}

void RoutedTransportInterface::forwardDataAvailable(const QUuid &clientId, const QByteArray &data)
{
    emit dataAvailable(clientId, data);
}

TransportRouter::TransportRouter(QObject *parent)
    : QObject(parent)
{}

RoutedTransportInterface *TransportRouter::registerTransportInterface(TransportInterface *transport, bool transferTransport)
{
    if (!m_transports.contains(transport)) {
        connect(transport, &TransportInterface::clientConnected, this, &TransportRouter::onClientConnected);
        connect(transport, &TransportInterface::clientDisconnected, this, &TransportRouter::onClientDisconnected);
        connect(transport, &TransportInterface::dataAvailable, this, &TransportRouter::onDataAvailable);
    }

    TransportPair &pair = m_transports[transport];
    RoutedTransportInterface *&proxy = transferTransport ? pair.transferTransport : pair.jsonRpcTransport;
    if (!proxy)
        proxy = new RoutedTransportInterface(transport, this);

    return proxy;
}

RoutedTransportInterface *TransportRouter::transportProxy(TransportInterface *transport, bool transferTransport) const
{
    if (!m_transports.contains(transport))
        return nullptr;

    const TransportPair pair = m_transports.value(transport);
    return transferTransport ? pair.transferTransport : pair.jsonRpcTransport;
}

void TransportRouter::unregisterTransportInterface(TransportInterface *transport)
{
    TransportPair pair = m_transports.take(transport);
    disconnect(transport, &TransportInterface::clientConnected, this, &TransportRouter::onClientConnected);
    disconnect(transport, &TransportInterface::clientDisconnected, this, &TransportRouter::onClientDisconnected);
    disconnect(transport, &TransportInterface::dataAvailable, this, &TransportRouter::onDataAvailable);

    for (auto it = m_clients.begin(); it != m_clients.end();) {
        if (it->transport == transport) {
            cleanupClient(it.key());
            it = m_clients.erase(it);
        } else {
            ++it;
        }
    }

    delete pair.jsonRpcTransport;
    delete pair.transferTransport;
}

void TransportRouter::onClientConnected(const QUuid &clientId)
{
    TransportInterface *transport = qobject_cast<TransportInterface *>(sender());
    if (!transport)
        return;

    ClientState state;
    state.transport = transport;
    state.timeoutTimer = new QTimer(this);
    state.timeoutTimer->setSingleShot(true);
    connect(state.timeoutTimer, &QTimer::timeout, this, [this, clientId]() {
        qCWarning(dcTransportRouter()) << "Client" << clientId << "did not select a protocol in time.";
        terminateUnroutedClient(clientId);
    });
    state.timeoutTimer->start(10000);
    m_clients.insert(clientId, state);
}

void TransportRouter::onClientDisconnected(const QUuid &clientId)
{
    if (!m_clients.contains(clientId))
        return;

    const ClientState state = m_clients.value(clientId);
    if (state.protocol != Protocol::Unknown) {
        RoutedTransportInterface *proxy = proxyTransport(state.transport, state.protocol == Protocol::Transfer);
        if (proxy) {
            proxy->forwardClientDisconnected(clientId);
        }
    }

    cleanupClient(clientId);
    m_clients.remove(clientId);
}

void TransportRouter::onDataAvailable(const QUuid &clientId, const QByteArray &data)
{
    if (!m_clients.contains(clientId))
        return;

    ClientState &state = m_clients[clientId];
    if (state.protocol != Protocol::Unknown) {
        RoutedTransportInterface *proxy = proxyTransport(state.transport, state.protocol == Protocol::Transfer);
        if (proxy) {
            proxy->forwardDataAvailable(clientId, data);
        }
        return;
    }

    state.buffer.append(data);

    QByteArray firstPacket;
    QByteArray trailingData;
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(state.buffer, &error);
    if (error.error == QJsonParseError::NoError) {
        firstPacket = state.buffer;
    } else if (error.error == QJsonParseError::GarbageAtEnd) {
        firstPacket = state.buffer.left(error.offset);
        trailingData = state.buffer.mid(error.offset);

        jsonDoc = QJsonDocument::fromJson(firstPacket, &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcTransportRouter()) << "Failed to parse protocol selector from client" << clientId << error.errorString();
            terminateUnroutedClient(clientId);
            return;
        }
    } else {
        if (state.buffer.size() > 1024 * 1024) {
            qCWarning(dcTransportRouter()) << "Dropping unrouted client with oversized buffer";
            terminateUnroutedClient(clientId);
            return;
        }

        if (!state.buffer.trimmed().endsWith('}'))
            return;

        qCWarning(dcTransportRouter()) << "Failed to parse protocol selector from client" << clientId << error.errorString();
        terminateUnroutedClient(clientId);
        return;
    }

    const QString method = jsonDoc.object().value("method").toString();
    if (method == "JSONRPC.Hello") {
        routeClient(clientId, Protocol::JsonRpc, firstPacket);
    } else if (method == "Transfer.Connect") {
        routeClient(clientId, Protocol::Transfer, firstPacket);
    } else {
        qCWarning(dcTransportRouter()) << "Unsupported initial method from client" << clientId << method;
        terminateUnroutedClient(clientId);
        return;
    }

    if (!trailingData.isEmpty() && m_clients.contains(clientId)) {
        onDataAvailable(clientId, trailingData);
    }
}

RoutedTransportInterface *TransportRouter::proxyTransport(TransportInterface *transport, bool transferTransport) const
{
    if (!m_transports.contains(transport))
        return nullptr;

    const TransportPair pair = m_transports.value(transport);
    return transferTransport ? pair.transferTransport : pair.jsonRpcTransport;
}

void TransportRouter::routeClient(const QUuid &clientId, Protocol protocol, const QByteArray &firstPacket)
{
    if (!m_clients.contains(clientId))
        return;

    TransportInterface *transport = nullptr;
    QTimer *timeoutTimer = nullptr;

    ClientState &state = m_clients[clientId];
    state.protocol = protocol;
    timeoutTimer = state.timeoutTimer;
    state.timeoutTimer = nullptr;
    state.buffer.clear();
    transport = state.transport;

    if (timeoutTimer)
        timeoutTimer->deleteLater();

    RoutedTransportInterface *proxy = proxyTransport(transport, protocol == Protocol::Transfer);
    if (!proxy) {
        qCWarning(dcTransportRouter()) << "No proxy transport registered for routed client";
        terminateUnroutedClient(clientId);
        return;
    }

    proxy->forwardClientConnected(clientId);
    proxy->forwardDataAvailable(clientId, firstPacket);
}

void TransportRouter::terminateUnroutedClient(const QUuid &clientId)
{
    if (!m_clients.contains(clientId))
        return;

    const ClientState state = m_clients.value(clientId);
    if (state.transport)
        state.transport->terminateClientConnection(clientId);
}

void TransportRouter::cleanupClient(const QUuid &clientId)
{
    ClientState &state = m_clients[clientId];
    if (state.timeoutTimer) {
        state.timeoutTimer->deleteLater();
        state.timeoutTimer = nullptr;
    }
}

} // namespace nymeaserver
