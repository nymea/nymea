/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
