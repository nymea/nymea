/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
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
    void setServerName(const QString &serverName) override;
    bool startServer() override;
    bool stopServer() override;
};

}

#endif // WEBSOCKETSERVER_H
