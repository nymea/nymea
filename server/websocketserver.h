/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

namespace guhserver {

class WebSocketServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit WebSocketServer(QObject *parent = 0);
    ~WebSocketServer();

    void sendData(const QUuid &clientId, const QVariantMap &data) override;
    void sendData(const QList<QUuid> &clients, const QVariantMap &data) override;

private:
    QWebSocketServer *m_server;
    QHash<QUuid, QWebSocket *> m_clientList;

    bool m_enabled;
    bool m_useSsl;
    qint16 m_port;

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void onBinaryMessageReceived(const QByteArray &data);
    void onTextMessageReceived(const QString &message);
    void onClientError(QAbstractSocket::SocketError error);
    void onServerError(QAbstractSocket::SocketError error);
    void onPing(quint64 elapsedTime, const QByteArray & payload);

public slots:
    bool startServer() override;
    bool stopServer() override;
};

}

#endif // WEBSOCKETSERVER_H
