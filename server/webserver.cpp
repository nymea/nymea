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

/*!
    \class guhserver::WebServer
    \brief This class represents the web server for guhd.

    \ingroup core
    \inmodule server

    The \l{WebServer} class provides a HTTP/1.1 web server. The web server
    provides access to the guh-webinterface and the path can be specified
    in the \tt /etc/guh/guhd.conf file and to the guh \l{https://github.com/guh/guh/wiki/REST-Api-documentation}{REST API}.
    The default port for the web server is 3333, which is according to this
    \l{https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers}{list}
    officially free.

    The URL for the insecure guh-webinterface access:
    \code http://localhost:3333\endcode

    The URL for the secure HTTPS (TLS 1.2) guh-webinterface access:
    \code https://localhost:3333\endcode

    The URL for the insecure REST API access to a \l{RestResource}:
    \code http://localhost:3333/api/v1/{resource}\endcode

    The URL for the secure HTTPS (TLS 1.2) REST API access to a \l{RestResource}:
    \code https://localhost:3333/api/v1/{RestResource}\endcode

    You can turn on the HTTPS server in the \tt WebServer section of the \tt /etc/guh/guhd.conf file.

    \note For \tt HTTPS you need to have a certificate and configure it in the \tt SSL-configuration
    section of the \tt /etc/guh/guhd.conf file.

    \sa WebSocketServer, TcpServer
*/

/*! \fn void guhserver::WebServer::httpRequestReady(const QUuid &clientId, const HttpRequest &httpRequest);
    This signal is emitted when a \a httpRequest from a client with the given \a clientId is ready.

    \sa RestServer, HttpRequest
*/

/*! \fn void guhserver::WebServer::clientConnected(const QUuid &clientId);
    This signal is emitted when a new client with the given \a clientId has been connected.
*/

/*! \fn void guhserver::WebServer::clientDisconnected(const QUuid &clientId);
    This signal is emitted when a client with the given \a clientId has been disconnected.
*/

/*! \fn void guhserver::WebServer::incomingConnection(qintptr socketDescriptor);
    Overwritten virtual method from \l{http://doc.qt.io/qt-5/qtcpserver.html#incomingConnection}{QTcpServer::incomingConnection( \a socketDescriptor)}.
*/

#include "webserver.h"
#include "loggingcategories.h"
#include "guhsettings.h"
#include "httpreply.h"
#include "httprequest.h"

#include <QJsonDocument>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSslSocket>
#include <QUrlQuery>
#include <QUuid>
#include <QUrl>
#include <QFile>

namespace guhserver {

/*! Constructs a \l{WebServer} with the given \a sslConfiguration and \a parent.
 *
 *  \sa ServerManager
 */
WebServer::WebServer(const QSslConfiguration &sslConfiguration, QObject *parent) :
    QTcpServer(parent),
    m_sslConfiguration(sslConfiguration),
    m_useSsl(false),
    m_enabled(false)
{
    // load webserver settings
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    qCDebug(dcWebSocketServer) << "Loading web socket server settings from" << settings.fileName();

    settings.beginGroup("WebServer");
    // 3333 Official free according to https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers
    m_port = settings.value("port", 3333).toInt();
    m_useSsl = settings.value("https", false).toBool();
    m_webinterfaceDir = QDir(settings.value("publicFolder", "/usr/share/guh-webinterface/public/").toString());
    settings.endGroup();

    // check public directory
    qCDebug(dcWebServer) << "Publish webinterface folder" << m_webinterfaceDir.path();
    if (!m_webinterfaceDir.exists())
        qCWarning(dcWebServer) << "Web interface public folder" << m_webinterfaceDir.path() << "does not exist.";


    // check SSL
    if (m_useSsl && m_sslConfiguration.isNull())
        m_useSsl = false;
}

/*! Destructor of this \l{WebServer}. */
WebServer::~WebServer()
{
    this->close();
}

/*! Send the given \a reply map to the corresponding client.
 *
 * \sa HttpReply
 */
void WebServer::sendHttpReply(HttpReply *reply)
{
    QSslSocket *socket = 0;
    socket = m_clientList.value(reply->clientId());
    if (!socket) {
        qCDebug(dcWebServer) << "Invalid socket pointer! This should never happen!!!";
        return;
    }
    writeData(socket, reply->data());
}

bool WebServer::verifyFile(QSslSocket *socket, const QString &fileName)
{
    QFileInfo file(fileName);

    // make shore the file exists
    if (!file.exists()) {
        qCWarning(dcWebServer) << "requested file" << file.filePath() << "does not exist.";
        HttpReply reply(HttpReply::NotFound);
        reply.setPayload("404 Not found.");
        reply.packReply();
        writeData(socket, reply.data());
        return false;
    }

    // make shore the file is in the public directory
    if (!file.canonicalFilePath().startsWith(m_webinterfaceDir.path())) {
        qCWarning(dcWebServer) << "requested file" << file.fileName() << "is outside the public folder.";
        HttpReply reply(HttpReply::Forbidden);
        reply.setPayload("403 Forbidden.");
        reply.packReply();
        writeData(socket, reply.data());
        socket->close();
        return false;
    }

    // make shore we can read the file
    if (!file.isReadable()) {
        qCWarning(dcWebServer) << "requested file" << file.fileName() << "is not readable.";
        HttpReply reply(HttpReply::Forbidden);
        reply.setPayload("403 Forbidden. Page not readable.");
        reply.packReply();
        writeData(socket, reply.data());
        socket->close();
        return false;
    }
    return true;
}

QString WebServer::fileName(const QString &query)
{
    QString fileName;
    if (query.isEmpty() || query == "/") {
        fileName = "/index.html";
    } else {
        fileName = query;
    }

    return m_webinterfaceDir.path() + fileName;
}


void WebServer::writeData(QSslSocket *socket, const QByteArray &data)
{
    socket->write(data);
    socket->close();
}

void WebServer::incomingConnection(qintptr socketDescriptor)
{
    if (!m_enabled)
        return;

    QSslSocket *socket = new QSslSocket();
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qCWarning(dcConnection) << "Could not set socket descriptor. Rejecting connection.";
        socket->close();
        delete socket;
        return;
    }

    // append the new client to the client list
    QUuid clientId = QUuid::createUuid();
    m_clientList.insert(clientId, socket);

    qCDebug(dcConnection) << QString("Webserver client %1:%2 connected").arg(socket->peerAddress().toString()).arg(socket->peerPort());

    if (m_useSsl) {
        // configure client connection
        socket->setSslConfiguration(m_sslConfiguration);
        connect(socket, SIGNAL(encrypted()), this, SLOT(onEncrypted()));
        socket->startServerEncryption();
        // wait for encrypted connection before continue with this client
        return;
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    emit clientConnected(clientId);
}


void WebServer::readClient()
{
    if (!m_enabled)
        return;

    QSslSocket *socket = qobject_cast<QSslSocket *>(sender());
    QUuid clientId = m_clientList.key(socket);

    // check client
    if (clientId.isNull()) {
        qCWarning(dcWebServer) << "Client not recognized";
        socket->close();
        socket->deleteLater();
        return;
    }

    // read HTTP request
    QByteArray data = socket->readAll();

    HttpRequest request;
    if (m_incompleteRequests.contains(socket)) {
        qCWarning(dcWebServer) << "Append data to incomlete request";
        request = m_incompleteRequests.take(socket);
        request.appendData(data);
    } else {
        request = HttpRequest(data);
    }

    if (!request.isComplete()) {
        m_incompleteRequests.insert(socket, request);
        return;
    }

    if (!request.isValid()) {
        qCWarning(dcWebServer) << "Got invalid request.";
        HttpReply reply(HttpReply::BadRequest);
        reply.setPayload("400 Bad Request.");
        writeData(socket, reply.data());
        return;
    }

    // verify HTTP version
    if (request.httpVersion() != "HTTP/1.1") {
        qCWarning(dcWebServer) << "HTTP version is not supported." ;
        HttpReply reply(HttpReply::HttpVersionNotSupported);
        reply.setPayload("505 HTTP version is not supported.");
        writeData(socket, reply.data());
        return;
    }

    qCDebug(dcWebServer) << QString("Got valid request from %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    qCDebug(dcWebServer) << request.methodString() << request.url().path();

    // verify method
    if (request.method() == HttpRequest::Unhandled) {
        HttpReply reply(HttpReply::MethodNotAllowed);
        reply.setHeader(HttpReply::AllowHeader, "GET, PUT, POST, DELETE");
        reply.setPayload("405 Method not allowed.");
        writeData(socket, reply.data());
        return;
    }

    // verify API query
    if (request.url().path().startsWith("/api/v1")) {
        emit httpRequestReady(clientId, request);
        return;
    }

    // request for a file...
    if (request.method() == HttpRequest::Get && m_webinterfaceDir.exists()) {
        QString path = fileName(request.url().path());
        if (!verifyFile(socket, path))
            return;

        QFile file(path);
        if (file.open(QFile::ReadOnly | QFile::Truncate)) {
            qCDebug(dcWebServer) << "load file" << file.fileName();
            HttpReply reply(HttpReply::Ok);
            if (file.fileName().endsWith(".html")) {
                reply.setHeader(HttpReply::ContentTypeHeader, "text/html; charset=\"utf-8\";");
            }
            reply.setPayload(file.readAll());
            writeData(socket, reply.data());
            return;
        }
    }

    // reject everything else...
    qCWarning(dcWebServer) << "Unknown message received. Respond client with 400: Not Implemented.";
    HttpReply reply(HttpReply::NotFound);
    reply.setPayload("404 Not found.");
    writeData(socket, reply.data());
}

void WebServer::onDisconnected()
{    
    QSslSocket* socket = static_cast<QSslSocket *>(sender());
    qCDebug(dcConnection) << "Webserver client disonnected.";

    // clean up
    QUuid clientId = m_clientList.key(socket);
    m_clientList.remove(clientId);
    m_incompleteRequests.remove(socket);
    socket->deleteLater();

    emit clientDisconnected(clientId);
}

void WebServer::onEncrypted()
{
    QSslSocket* socket = static_cast<QSslSocket *>(sender());
    qCDebug(dcConnection) << QString("Encrypted connection %1:%2 successfully established.").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    emit clientConnected(m_clientList.key(socket));
}

void WebServer::onError(QAbstractSocket::SocketError error)
{
    QSslSocket* socket = static_cast<QSslSocket *>(sender());
    qCWarning(dcConnection) << "Client socket error" << socket->peerAddress() << error << socket->errorString();
}

/*! Returns true if this \l{WebServer} started successfully. */
bool WebServer::startServer()
{
    if (!listen(QHostAddress::Any, m_port)) {
        qCWarning(dcConnection) << "Webserver could not listen on" << serverAddress().toString() << m_port;
        m_enabled = false;
        return false;
    }
    if (m_useSsl) {
        qCDebug(dcConnection) << "Started webserver on" << QString("https://%1:%2").arg(serverAddress().toString()).arg(m_port);
    } else {
        qCDebug(dcConnection) << "Started webserver on" << QString("http://%1:%2").arg(serverAddress().toString()).arg(m_port);
    }
    m_enabled = true;
    return true;
}

/*! Returns true if this \l{WebServer} stopped successfully. */
bool WebServer::stopServer()
{
    close();
    m_enabled = false;
    qCDebug(dcConnection) << "Webserver closed.";
    return true;
}

}
