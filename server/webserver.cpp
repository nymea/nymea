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
    m_server = new QTcpServer(this);

    // load webserver settings
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    qCDebug(dcTcpServer) << "Loading Webserver settings from:" << settings.fileName();

    settings.beginGroup("Webserver");
    m_port = settings.value("port", 3000).toUInt();
    // load the path to the webinterface public folder (qdir to make shore there is no "/" at the end)
    m_webinterfaceDir = QDir(settings.value("publicFolder", "/usr/share/guh-webinterface/public/").toString());
    settings.endGroup();

    qCDebug(dcTcpServer) << "Using port" << m_port;
    qCDebug(dcTcpServer) << "Publish webinterface from" << m_webinterfaceDir.path();

    connect(m_server, &QTcpServer::newConnection, this, &WebServer::onNewConnection);
}

WebServer::~WebServer()
{
    m_server->close();
}

void WebServer::sendData(const QUuid &clientId, const QVariantMap &data)
{
    Q_UNUSED(clientId)
    Q_UNUSED(data)

    // TODO: reply.setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");

}

void WebServer::sendData(const QList<QUuid> &clients, const QVariantMap &data)
{
    Q_UNUSED(clients)
    Q_UNUSED(data)

    // TODO: reply.setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
}

bool WebServer::verifyFile(QTcpSocket *socket, const QString &fileName)
{
    QFileInfo checkFile(fileName);

    // make shore the file exists
    if (!checkFile.exists()) {
        qCWarning(dcWebServer) << "requested file" << checkFile.fileName() << "does not exist.";
        HttpReply reply(HttpReply::NotFound);
        reply.setPayload("404 Not found.");
        writeData(socket, reply.packReply());
        return false;
    }

    // make shore the file is in the public directory
    if (!checkFile.canonicalFilePath().startsWith(m_webinterfaceDir.path())) {
        qCWarning(dcWebServer) << "requested file" << checkFile.fileName() << "is outside the public folder.";
        HttpReply reply(HttpReply::Forbidden);
        reply.setPayload("403 Forbidden.");
        writeData(socket, reply.packReply());
        socket->close();
        return false;
    }

    // make shore we can read the file
    if (!checkFile.isReadable()) {
        qCWarning(dcWebServer) << "requested file" << checkFile.fileName() << "is not readable.";
        HttpReply reply(HttpReply::Forbidden);
        reply.setPayload("403 Forbidden. Page not readable.");
        writeData(socket, reply.packReply());
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


WebServer::RequestMethod WebServer::getRequestMethodType(const QString &methodString)
{
    if (methodString == "GET") {
        return RequestMethod::Get;
    } else if (methodString == "POST") {
        return RequestMethod::Post;
    } else if (methodString == "PUT") {
        return RequestMethod::Put;
    } else if (methodString == "DELETE") {
        return RequestMethod::Delete;
    }
    return RequestMethod::Unhandled;
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

    // TODO: maby check already at this point if this is a ws connection or not

    connect(socket, &QTcpSocket::readyRead, this, &WebServer::readClient);
    connect(socket, &QTcpSocket::disconnected, this, &WebServer::discardClient);
    qCDebug(dcConnection) << "Webserver client connected" << socket->peerName() << socket->peerAddress().toString() << socket->peerPort();

    emit clientConnected(clientId);
}

void WebServer::readClient()
{
    if (!m_enabled)
        return;

    QTcpSocket* socket = static_cast<QTcpSocket *>(sender());

    // read data
    QByteArray data = socket->readAll();

    QStringList lines = QString(data).split("\r\n");
    QStringList tokens = QString(data).split(QRegExp("[ \r\n][ \r\n]*"));

    // verify HTTP version
    if (!lines.first().contains("HTTP/1.1")) {
        qCWarning(dcWebServer) << "HTTP version is not supported." ;
        HttpReply reply(HttpReply::HttpVersionNotSupported);
        reply.setPayload("505 HTTP version is not supported.");
        writeData(socket, reply.packReply());
        return;
    }

    if (tokens.isEmpty() || tokens.count() < 2)
        return;

    QString methodString = tokens.at(0);
    QString queryString = tokens.at(1);

    qCDebug(dcWebServer) << QString("Got request from %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    qCDebug(dcWebServer) << "Request method:" << methodString;
    qCDebug(dcWebServer) << "Request query :" << queryString;

    // verify method
    RequestMethod requestMethod = getRequestMethodType(methodString);
    if (requestMethod == RequestMethod::Unhandled) {
        qCWarning(dcWebServer) << "method" << methodString << "not allowed";
        HttpReply reply(HttpReply::MethodNotAllowed);
        reply.setHeader(HttpReply::AllowHeader, "GET, PUT, POST, DELETE");
        reply.setPayload("405 Method not allowed.");
        writeData(socket, reply.packReply());
        return;
    }

    // TODO: authentification check

    // TODO: parse payload and header

    // TODO: verify header to make shore this is a valid HTTP request

    if (queryString.startsWith("/api/v1")) {
        // TODO: check if this is an API call
        qCDebug(dcWebServer) << "got api call";
        HttpReply reply(HttpReply::Ok);
        reply.setPayload("Got api call. This is not implemented yet...");
        writeData(socket, reply.packReply());
        return;
    }

    if (queryString.startsWith("/ws")) {
        qCDebug(dcWebServer) << "got websocket request";
        HttpReply reply(HttpReply::Ok);
        reply.setPayload("Got api call. This is not implemented yet...");
        writeData(socket, reply.packReply());

        //  TODO: move the ws client to a separat websocket client list and redirect
        //  the notification stream to thouse clients
    }

    // request for a file...
    if (requestMethod == RequestMethod::Get) {
        if (!verifyFile(socket, fileName(queryString)))
            return;

        QFile file(fileName(queryString));
        if (file.open(QFile::ReadOnly | QFile::Truncate)) {
            qCDebug(dcWebServer) << "load file" << file.fileName();
            HttpReply reply(HttpReply::Ok);
            if (file.fileName().endsWith(".html")) {
                reply.setHeader(HttpReply::ContentTypeHeader, "text/html; charset=\"utf-8\";");
            }
            reply.setPayload(file.readAll());
            writeData(socket, reply.packReply());
            return;
        }
    }

    qCWarning(dcWebServer) << "Not recognized request.";
    HttpReply reply(HttpReply::NotImplemented);
    reply.setPayload("501 Not Implemented.");
    writeData(socket, reply.packReply());
    return;
}

void WebServer::discardClient()
{    
    QTcpSocket* socket = static_cast<QTcpSocket *>(sender());
    qCDebug(dcConnection) << "Webserver client disonnected.";

    // clean up
    QUuid clientId = m_clientList.key(socket);
    m_clientList.take(clientId)->deleteLater();

    emit clientDisconnected(clientId);
}

bool WebServer::startServer()
{
    if (!m_server->listen(QHostAddress::Any, m_port)) {
        qCWarning(dcConnection) << "Webserver could not listen on" << m_server->serverAddress().toString() << m_port;
        m_enabled = false;
        return false;
    }
    m_enabled = true;
    qCDebug(dcConnection) << "Started webserver on" << QString("http://%1:%2").arg(m_server->serverAddress().toString()).arg(m_port);
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
