/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 - 2018 Simon Stürz <simon.stuerz@guh.io>            *
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

#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QObject>
#include <QUuid>
#include <QVariant>
#include <QList>
#include <QWebSocket>
#include <QWebSocketServer>

#include "transportinterface.h"

// Note: WebSocket Protocol from the Internet Engineering Task Force (IETF) -> RFC6455 V13:
//       http://tools.ietf.org/html/rfc6455

class QSslConfiguration;

namespace nymeaserver {

class WebSocketServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit WebSocketServer(const ServerConfiguration &configuration, const QSslConfiguration &sslConfiguration, QObject *parent = nullptr);
    ~WebSocketServer() override;

    QUrl serverUrl() const;

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clients, const QByteArray &data) override;

    void terminateClientConnection(const QUuid &clientId) override;

private:
    QWebSocketServer *m_server = nullptr;
    QHash<QUuid, QWebSocket *> m_clientList;
    QSslConfiguration m_sslConfiguration;
    bool m_enabled;

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void onBinaryMessageReceived(const QByteArray &data);
    void onTextMessageReceived(const QString &message);
    void onClientError(QAbstractSocket::SocketError error);
    void onServerError(QAbstractSocket::SocketError error);
    void onPing(quint64 elapsedTime, const QByteArray & payload);

public slots:
    void reconfigureServer(const ServerConfiguration &config);
    void setServerName(const QString &serverName) override;
    bool startServer() override;
    bool stopServer() override;
};

}

#endif // WEBSOCKETSERVER_H
