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
#include "network/httpreply.h"
#include "network/httprequest.h"

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

void WebServer::sendHttpReply(const QUuid &clientId, const HttpReply &reply)
{
    QTcpSocket *socket = m_clientList.value(clientId);
    writeData(socket, reply.data());
}

bool WebServer::verifyFile(QTcpSocket *socket, const QString &fileName)
{
    QFileInfo checkFile(fileName);

    // make shore the file exists
    if (!checkFile.exists()) {
        qCWarning(dcWebServer) << "requested file" << checkFile.fileName() << "does not exist.";
        HttpReply reply(HttpReply::NotFound);
        reply.setPayload("404 Not found.");
        reply.packReply();
        writeData(socket, reply.data());
        return false;
    }

    // make shore the file is in the public directory
    if (!checkFile.canonicalFilePath().startsWith(m_webinterfaceDir.path())) {
        qCWarning(dcWebServer) << "requested file" << checkFile.fileName() << "is outside the public folder.";
        HttpReply reply(HttpReply::Forbidden);
        reply.setPayload("403 Forbidden.");
        reply.packReply();
        writeData(socket, reply.data());
        socket->close();
        return false;
    }

    // make shore we can read the file
    if (!checkFile.isReadable()) {
        qCWarning(dcWebServer) << "requested file" << checkFile.fileName() << "is not readable.";
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

    return QFileInfo(m_webinterfaceDir.path() + fileName).canonicalFilePath();
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
        return;
    }

    // read HTTP request
    HttpRequest request = HttpRequest(socket->readAll());
    if (!request.isValid()) {
        qCWarning(dcWebServer) << "Got invalid request.";
        HttpReply reply(HttpReply::BadRequest);
        reply.setPayload("400 Bad Request.");
        reply.packReply();
        writeData(socket, reply.data());
        return;
    }

    // verify HTTP version
    if (request.httpVersion() != "HTTP/1.1") {
        qCWarning(dcWebServer) << "HTTP version is not supported." ;
        HttpReply reply(HttpReply::HttpVersionNotSupported);
        reply.setPayload("505 HTTP version is not supported.");
        reply.packReply();
        writeData(socket, reply.data());
        return;
    }

    qCDebug(dcWebServer) << QString("Got valid request from %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    qCDebug(dcWebServer) << request;

    // verify method
    if (request.method() == HttpRequest::Unhandled) {
        HttpReply reply(HttpReply::MethodNotAllowed);
        reply.setHeader(HttpReply::AllowHeader, "GET, PUT, POST, DELETE");
        reply.setPayload("405 Method not allowed.");
        reply.packReply();
        writeData(socket, reply.data());
        return;
    }

    // verify query
    if (request.urlQuery().query().startsWith("/api/v1")) {
        emit httpRequestReady(clientId, request);
        return;
    }

    // request for a file...
    if (request.method() == HttpRequest::Get) {
        if (!verifyFile(socket, fileName(request.urlQuery().query())))
            return;

        QFile file(fileName(request.urlQuery().query()));
        if (file.open(QFile::ReadOnly | QFile::Truncate)) {
            qCDebug(dcWebServer) << "load file" << file.fileName();
            HttpReply reply(HttpReply::Ok);
            if (file.fileName().endsWith(".html")) {
                reply.setHeader(HttpReply::ContentTypeHeader, "text/html; charset=\"utf-8\";");
            }
            reply.setPayload(file.readAll());
            reply.packReply();
            writeData(socket, reply.data());
            return;
        }
    }

    // reject everything else...
    qCWarning(dcWebServer) << "Unknown message received. Respond client with 501: Not Implemented.";
    HttpReply reply(HttpReply::NotImplemented);
    reply.setPayload("501 Not implemented.");
    reply.packReply();
    writeData(socket, reply.data());
}

void WebServer::onDisconnected()
{    
    QTcpSocket* socket = qobject_cast<QTcpSocket *>(sender());
    qCDebug(dcConnection) << "Webserver client disonnected.";

    // clean up
    QUuid clientId = m_clientList.key(socket);
    m_clientList.remove(clientId);
    socket->deleteLater();

    emit clientDisconnected(clientId);
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
