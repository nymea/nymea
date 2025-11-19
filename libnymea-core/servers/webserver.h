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

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QDir>
#include <QTimer>
#include <QImage>
#include <QBuffer>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>

#include "nymeaconfiguration.h"

// Note: Hypertext Transfer Protocol (HTTP/1.1) from the Internet Engineering Task Force (IETF):
//       https://tools.ietf.org/html/rfc7231

class HttpReply;
class HttpRequest;
class WebServerResource;

namespace nymeaserver {

class WebServerClient : public QObject
{
    Q_OBJECT
public:
    WebServerClient(const QHostAddress &address, QObject *parent = nullptr);

    QHostAddress address() const;

    QList<QSslSocket *> connections();
    void addConnection(QSslSocket *socket);
    void removeConnection(QSslSocket *socket);

    void resetTimout(QSslSocket *socket);

private:
    QHostAddress m_address;
    QList<QSslSocket *> m_connections;
    QHash<QTimer *, QSslSocket *> m_runningConnections;

private slots:
    void onTimout();
};


class WebServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit WebServer(const WebServerConfiguration &configuration, const QSslConfiguration &sslConfiguration, QObject *parent = nullptr);
    ~WebServer() override;

    QUrl serverUrl() const;
    WebServerConfiguration configuration() const;

    void sendHttpReply(HttpReply *reply);

    QList<WebServerResource *> resources() const;
    bool registerResource(WebServerResource *resource);
    void unregisterResource(WebServerResource *resource);

private:
    QHash<QUuid, QSslSocket *> m_clientList;
    QList<WebServerClient *> m_webServerClients;
    QHash<QSslSocket *, HttpRequest> m_incompleteRequests;

    QString m_serverName;
    WebServerConfiguration m_configuration;
    QSslConfiguration m_sslConfiguration;

    QHash<QString, WebServerResource *> m_resources;

    bool m_enabled = false;

    bool verifyFile(QSslSocket *socket, const QString &fileName);
    QString fileName(const QString &query);

    QByteArray createServerXmlDocument(QHostAddress address);
    HttpReply *processIconRequest(const QString &fileName);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);

    void httpRequestReady(const QUuid &clientId, const HttpRequest &httpRequest);

private slots:
    void readClient();
    void onDisconnected();
    void onEncrypted();
    void onError(QAbstractSocket::SocketError error);
    void onAsyncReplyFinished();

    void setupClient(const QUuid &clientId, QSslSocket *socket);

public slots:
    void setConfiguration(const nymeaserver::WebServerConfiguration &config);
    void setServerName(const QString &serverName);
    bool startServer();
    bool stopServer();


};

}

#endif // WEBSERVER_H
