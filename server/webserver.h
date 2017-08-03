/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "network/avahi/qtavahiservice.h"

// Note: Hypertext Transfer Protocol (HTTP/1.1) from the Internet Engineering Task Force (IETF):
//       https://tools.ietf.org/html/rfc7231

namespace guhserver {

class HttpRequest;
class HttpReply;

class WebServerClient : public QObject
{
    Q_OBJECT
public:
    WebServerClient(const QHostAddress &address, QObject *parent = 0);

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
    explicit WebServer(const QHostAddress &host, const uint &port, const QString &publicFolder, bool sslEnabled, const QSslConfiguration &sslConfiguration, QObject *parent = 0);
    ~WebServer();

    void sendHttpReply(HttpReply *reply);
    int port() const;
    QList<QHostAddress> serverAddressList();


private:
    QHash<QUuid, QSslSocket *> m_clientList;
    QList<WebServerClient *> m_webServerClients;
    QHash<QSslSocket *, HttpRequest> m_incompleteRequests;

    QtAvahiService *m_avahiService;

    QHostAddress m_host;
    qint16 m_port;
    QDir m_webinterfaceDir;

    QSslConfiguration m_sslConfiguration;
    bool m_useSsl;

    bool m_enabled;

    bool verifyFile(QSslSocket *socket, const QString &fileName);
    QString fileName(const QString &query);

    QByteArray createServerXmlDocument(QHostAddress address);
    HttpReply *processIconRequest(const QString &fileName);
    QHostAddress getServerAddress(QHostAddress clientAddress);

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

    void onAvahiServiceStateChanged(const QtAvahiService::QtAvahiServiceState &state);

public slots:
    bool reconfigureServer(const QHostAddress &address, const uint &port);
    bool startServer();
    bool stopServer();

};

}

#endif // WEBSERVER_H
