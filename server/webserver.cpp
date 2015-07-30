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

#include "webserver.h"
#include "loggingcategories.h"
#include "guhsettings.h"
#include "httpreply.h"
#include "httprequest.h"

#include <QJsonDocument>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrlQuery>
#include <QUuid>
#include <QUrl>
#include <QFile>

namespace guhserver {

WebServer::WebServer(QObject *parent) :
    TransportInterface(parent),
    m_enabled(false)
{
    // load webserver settings
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    qCDebug(dcTcpServer) << "Loading webserver settings from:" << settings.fileName();

    settings.beginGroup("Webserver");
    m_port = settings.value("port", 3000).toInt();
    m_webinterfaceDir = QDir(settings.value("publicFolder", "/usr/share/guh-webinterface/public/").toString());
    settings.endGroup();

    qCDebug(dcTcpServer) << "Publish webinterface from" << m_webinterfaceDir.path();

    if (!m_webinterfaceDir.exists())
        qCWarning(dcWebServer) << "Web interface path" << m_webinterfaceDir.path() << "does not exist.";

    // create webserver
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &WebServer::onNewConnection);
}

WebServer::~WebServer()
{
    m_server->close();
}

void WebServer::sendData(const QUuid &clientId, const QVariantMap &data)
{
    QTcpSocket *socket = m_clientList.value(clientId);
    HttpReply reply(HttpReply::Ok);
    reply.setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    reply.setPayload(QJsonDocument::fromVariant(data).toJson());
    reply.packReply();
    writeData(socket, reply.data());
}

void WebServer::sendData(const QList<QUuid> &clients, const QVariantMap &data)
{
    foreach (const QUuid &client, clients) {
        QTcpSocket *socket = m_clientList.value(client);
        HttpReply reply(HttpReply::Ok);
        reply.setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
        reply.setPayload(QJsonDocument::fromVariant(data).toJson());
        reply.packReply();
        writeData(socket, reply.data());
    }
}

void WebServer::sendHttpReply(HttpReply *reply)
{
    QTcpSocket *socket = 0;
    socket = m_clientList.value(reply->clientId());

    if (!socket) {
        qCDebug(dcWebServer) << "Invalid socket pointer! This should never happen!!!";
        return;
    }
    writeData(socket, reply->data());
}

bool WebServer::verifyFile(QTcpSocket *socket, const QString &fileName)
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

void WebServer::writeData(QTcpSocket *socket, const QByteArray &data)
{
    QTextStream os(socket);
    os.setAutoDetectUnicode(true);
    os << data;
    socket->close();
}

void WebServer::onNewConnection()
{
    if (!m_enabled)
        return;

    QTcpSocket* socket = m_server->nextPendingConnection();

    // append the new client to the client list
    QUuid clientId = QUuid::createUuid();
    m_clientList.insert(clientId, socket);

    connect(socket, &QTcpSocket::readyRead, this, &WebServer::readClient);
    connect(socket, &QTcpSocket::disconnected, this, &WebServer::onDisconnected);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    qCDebug(dcConnection) << "Webserver client connected" << socket->peerName() << socket->peerAddress().toString() << socket->peerPort();

    emit clientConnected(clientId);
}

void WebServer::readClient()
{
    if (!m_enabled)
        return;

    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
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
    if (request.method() == HttpRequest::Get) {
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
    qCWarning(dcWebServer) << "Unknown message received. Respond client with 501: Not Implemented.";
    HttpReply reply(HttpReply::NotImplemented);
    reply.setPayload("501 Not implemented.");
    writeData(socket, reply.data());
}

void WebServer::onDisconnected()
{    
    QTcpSocket* socket = qobject_cast<QTcpSocket *>(sender());
    qCDebug(dcConnection) << "Webserver client disonnected.";

    // clean up
    QUuid clientId = m_clientList.key(socket);
    m_clientList.remove(clientId);
    m_incompleteRequests.remove(socket);
    socket->deleteLater();

    emit clientDisconnected(clientId);
}

void WebServer::onError(QAbstractSocket::SocketError error)
{
    QTcpSocket* socket = qobject_cast<QTcpSocket *>(sender());
    qWarning(dcWebServer) << "Client socket error" << socket->peerAddress() << error << socket->errorString();
}

bool WebServer::startServer()
{
    if (!m_server->listen(QHostAddress::Any, m_port)) {
        qCWarning(dcConnection) << "Webserver could not listen on" << m_server->serverAddress().toString() << m_port;
        m_enabled = false;
        return false;
    }
    qCDebug(dcConnection) << "Started webserver on" << QString("http://%1:%2").arg(m_server->serverAddress().toString()).arg(m_port);
    m_enabled = true;

    return true;
}

bool WebServer::stopServer()
{
    m_server->close();
    m_enabled = false;
    qCDebug(dcConnection) << "Webserver closed.";
    return true;
}

}
