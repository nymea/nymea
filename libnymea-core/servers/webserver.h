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

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QDir>
#include <QTimer>
#include <QBuffer>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>
#include <QImage>

#include "nymeaconfiguration.h"

// Note: Hypertext Transfer Protocol (HTTP/1.1) from the Internet Engineering Task Force (IETF):
//       https://tools.ietf.org/html/rfc7231

namespace nymeaserver {

class HttpReply;
class HttpRequest;

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

    void sendHttpReply(HttpReply *reply);

private:
    QHash<QUuid, QSslSocket *> m_clientList;
    QList<WebServerClient *> m_webServerClients;
    QHash<QSslSocket *, HttpRequest> m_incompleteRequests;

    QString m_serverName;
    WebServerConfiguration m_configuration;
    QSslConfiguration m_sslConfiguration;

    bool m_enabled = false;

    bool verifyFile(QSslSocket *socket, const QString &fileName);
    QString fileName(const QString &query);

    QByteArray createServerXmlDocument(QHostAddress address);
    HttpReply *processIconRequest(const QString &fileName);
    HttpReply *processDebugRequest(const QString &requestPath);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void httpRequestReady(const QUuid &clientId, const HttpRequest &httpRequest);
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);

private slots:
    void readClient();
    void onDisconnected();
    void onEncrypted();
    void onError(QAbstractSocket::SocketError error);
    void onAsyncReplyFinished();

public slots:
    void setConfiguration(const WebServerConfiguration &config);
    void setServerName(const QString &serverName);
    bool startServer();
    bool stopServer();
    WebServerConfiguration configuration() const;

};

}

#endif // WEBSERVER_H
