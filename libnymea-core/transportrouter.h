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

#ifndef TRANSPORTROUTER_H
#define TRANSPORTROUTER_H

#include <QHash>
#include <QObject>
#include <QTimer>

#include "transportinterface.h"

namespace nymeaserver {

class RoutedTransportInterface : public TransportInterface
{
    Q_OBJECT
public:
    explicit RoutedTransportInterface(TransportInterface *backend, QObject *parent = nullptr);

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clients, const QByteArray &data) override;
    void terminateClientConnection(const QUuid &clientId) override;

    bool startServer() override;
    bool stopServer() override;

    void forwardClientConnected(const QUuid &clientId);
    void forwardClientDisconnected(const QUuid &clientId);
    void forwardDataAvailable(const QUuid &clientId, const QByteArray &data);

private:
    TransportInterface *m_backend = nullptr;
};

class TransportRouter : public QObject
{
    Q_OBJECT
public:
    explicit TransportRouter(QObject *parent = nullptr);

    RoutedTransportInterface *registerTransportInterface(TransportInterface *transport, bool transferTransport);
    RoutedTransportInterface *transportProxy(TransportInterface *transport, bool transferTransport) const;
    void unregisterTransportInterface(TransportInterface *transport);

private:
    enum class Protocol {
        Unknown,
        JsonRpc,
        Transfer
    };

    struct TransportPair
    {
        RoutedTransportInterface *jsonRpcTransport = nullptr;
        RoutedTransportInterface *transferTransport = nullptr;
    };

    struct ClientState
    {
        TransportInterface *transport = nullptr;
        Protocol protocol = Protocol::Unknown;
        QByteArray buffer;
        QTimer *timeoutTimer = nullptr;
    };

private slots:
    void onClientConnected(const QUuid &clientId);
    void onClientDisconnected(const QUuid &clientId);
    void onDataAvailable(const QUuid &clientId, const QByteArray &data);

private:
    RoutedTransportInterface *proxyTransport(TransportInterface *transport, bool transferTransport) const;
    void routeClient(const QUuid &clientId, Protocol protocol, const QByteArray &firstPacket);
    void terminateUnroutedClient(const QUuid &clientId);
    void cleanupClient(const QUuid &clientId);

    QHash<TransportInterface *, TransportPair> m_transports;
    QHash<QUuid, ClientState> m_clients;
};

} // namespace nymeaserver

#endif // TRANSPORTROUTER_H
