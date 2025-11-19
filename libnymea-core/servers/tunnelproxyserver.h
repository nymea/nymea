// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
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

#ifndef TUNNELPROXYSERVER_H
#define TUNNELPROXYSERVER_H

#include <QObject>
#include <QTimer>

#include "transportinterface.h"

#include <tunnelproxy/tunnelproxysocket.h>
#include <tunnelproxy/tunnelproxysocketserver.h>

namespace nymeaserver {

using namespace remoteproxyclient;

class TunnelProxyServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit TunnelProxyServer(const QString &serverName, const QUuid &serverUuid, const TunnelProxyServerConfiguration &configuration, QObject *parent = nullptr);
    ~TunnelProxyServer() override;

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clients, const QByteArray &data) override;

    void terminateClientConnection(const QUuid &clientId) override;

public slots:
    void setServerName(const QString &serverName) override;
    void setTunnelProxyConfig(const TunnelProxyServerConfiguration &tunnelProxyConfig);

    bool startServer() override;
    bool stopServer() override;

signals:
    void runningChanged(bool running);

private slots:
    void onStateChanged(TunnelProxySocketServer::State state);
    void onServerRunningChanged(bool running);
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void onServerErrorOccurred(TunnelProxySocketServer::Error error);
    void onSslErrors(const QList<QSslError> &errors);

    void onClientConnected(TunnelProxySocket *tunnelProxySocket);
    void onClientDisconnected(TunnelProxySocket *tunnelProxySocket);

private:
    TunnelProxyServerConfiguration m_tunnelProxyConfig;
    TunnelProxySocketServer *m_tunnelProxySocketServer = nullptr;
    QString m_serverName;
    QUuid m_serverUuid;
    QUrl m_serverUrl;

    QHash<QUuid, TunnelProxySocket *> m_clients;

    void initConfiguration();

};

}

#endif // TUNNELPROXYSERVER_H
