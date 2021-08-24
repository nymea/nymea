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

/*!
    \class nymeaserver::WebServer
    \brief This class represents the web server for nymead.

    \ingroup server
    \inmodule core

    The \l{WebServer} class provides a HTTP/1.1 web server. The web server
    provides access to the nymea-webinterface and the path can be specified
    in the \tt /etc/nymea/nymead.conf file and to the nymea \l{https://github.com/guh/nymea/wiki/REST-Api-documentation}{REST API}.
    The default port for the web server is 3333, which is according to this
    \l{https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers}{list}
    officially free.

    The URL for the insecure nymea-webinterface access:
    \code http://localhost:3333\endcode

    The URL for the secure HTTPS (TLS 1.2) nymea-webinterface access:
    \code https://localhost:3333\endcode

    The URL for the insecure REST API access to a \l{RestResource}:
    \code http://localhost:3333/api/v1/{RestResource}\endcode

    The URL for the secure HTTPS (TLS 1.2) REST API access to a \l{RestResource}:
    \code https://localhost:3333/api/v1/{RestResource}\endcode

    You can turn on the HTTPS server in the \tt WebServer section of the \tt /etc/nymea/nymead.conf file.

    \note For \tt HTTPS you need to have a certificate and configure it in the \tt SSL-configuration
    section of the \tt /etc/nymea/nymead.conf file.

    \sa WebServerClient, WebSocketServer, TcpServer
*/

/*! \fn void nymeaserver::WebServer::httpRequestReady(const QUuid &clientId, const HttpRequest &httpRequest);
    This signal is emitted when a \a httpRequest from a client with the given \a clientId is ready.

    \sa RestServer, HttpRequest
*/

/*! \fn void nymeaserver::WebServer::clientConnected(const QUuid &clientId);
    This signal is emitted when a new client with the given \a clientId has been connected.
*/

/*! \fn void nymeaserver::WebServer::clientDisconnected(const QUuid &clientId);
    This signal is emitted when a client with the given \a clientId has been disconnected.
*/

/*! \fn void nymeaserver::WebServer::incomingConnection(qintptr socketDescriptor);
    Overwritten virtual method from \l{http://doc.qt.io/qt-5/qtcpserver.html#incomingConnection}{QTcpServer::incomingConnection( \a socketDescriptor)}.
*/

#include "webserver.h"
#include "loggingcategories.h"
#include "nymeasettings.h"
#include "nymeacore.h"
#include "httpreply.h"
#include "httprequest.h"
#include "debugserverhandler.h"
#include "version.h"

#include <QJsonDocument>
#include <QNetworkInterface>
#include <QXmlStreamWriter>
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSslSocket>
#include <QUrlQuery>
#include <QUuid>
#include <QUrl>
#include <QFile>

namespace nymeaserver {

/*! Constructs a \l{WebServer} with the given \a configuration, \a sslConfiguration and \a parent.
 *
 *  \sa ServerManager, WebServerConfiguration
 */
WebServer::WebServer(const WebServerConfiguration &configuration, const QSslConfiguration &sslConfiguration, QObject *parent) :
    QTcpServer(parent),
    m_configuration(configuration),
    m_sslConfiguration(sslConfiguration)
{
    if (QCoreApplication::instance()->organizationName() == "nymea-test") {
        m_configuration.publicFolder = QCoreApplication::applicationDirPath();
    }
    qCDebug(dcWebServer()) << "Starting WebServer. Interface:" << m_configuration.address << "Port:" << m_configuration.port << "SSL:" << m_configuration.sslEnabled << "AUTH:" << m_configuration.authenticationEnabled << "Public folder:" << QDir(m_configuration.publicFolder).canonicalPath();
}

/*! Destructor of this \l{WebServer}. */
WebServer::~WebServer()
{
    qCDebug(dcWebServer()) << "Shutting down \"Webserver\"" << serverUrl().toString();

    this->close();
}

/*! Returns the server URL of this WebServer. */
QUrl WebServer::serverUrl() const
{
    return QUrl(QString("%1://%2:%3").arg((m_configuration.sslEnabled ? "https" : "http")).arg(m_configuration.address.toString()).arg(m_configuration.port));
}

/*! Send the given \a reply map to the corresponding client.
 *
 * \sa HttpReply
 */
void WebServer::sendHttpReply(HttpReply *reply)
{
    // get the right socket
    QSslSocket *socket = nullptr;
    socket = m_clientList.value(reply->clientId());
    if (!socket) {
        qCWarning(dcWebServer()) << "Invalid socket pointer! This should never happen!!! Missing clientId in reply?";
        return;
    }

    // send raw data
    reply->packReply();
    qCDebug(dcWebServerTraffic()) << "Send reply to" << socket->peerAddress().toString() << reply;
    qCDebug(dcWebServer()) << "Respond" << socket->peerAddress().toString() << reply->httpStatusCode() << reply->httpReasonPhrase();
    socket->write(reply->data());
}

bool WebServer::verifyFile(QSslSocket *socket, const QString &fileName)
{
    QFileInfo file(fileName);

    // make sure the file exists
    if (!file.exists()) {
        qCDebug(dcWebServer()) << "requested file" << file.filePath() << "does not exist.";
        HttpReply *reply = HttpReply::createErrorReply(HttpReply::NotFound);
        reply->setClientId(m_clientList.key(socket));
        sendHttpReply(reply);
        reply->deleteLater();
        return false;
    }

    // make sure the file is in the public directory
    if (!file.canonicalFilePath().startsWith(QDir(m_configuration.publicFolder).canonicalPath())) {
        qCDebug(dcWebServer()) << "Requested file" << file.fileName() << "is outside the public folder.";
        HttpReply *reply = HttpReply::createErrorReply(HttpReply::Forbidden);
        reply->setClientId(m_clientList.key(socket));
        sendHttpReply(reply);
        reply->deleteLater();
        return false;
    }

    // make sure we can read the file
    if (!file.isReadable()) {
        qCDebug(dcWebServer()) << "Requested file" << file.fileName() << "is not readable.";
        HttpReply *reply = HttpReply::createErrorReply(HttpReply::Forbidden);
        reply->setClientId(m_clientList.key(socket));
        reply->setPayload("403 Forbidden. File not readable");
        sendHttpReply(reply);
        reply->deleteLater();
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

    return m_configuration.publicFolder + "/" + fileName;
}

HttpReply *WebServer::processIconRequest(const QString &fileName)
{
    if (!fileName.endsWith(".png"))
        return HttpReply::createErrorReply(HttpReply::NotFound);

    QByteArray imageData;
    QImage image(":" + fileName);
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "png");

    if (!imageData.isEmpty()) {
        HttpReply *reply = HttpReply::createSuccessReply();
        reply->setHeader(HttpReply::ContentTypeHeader, "image/png");
        reply->setPayload(imageData);
        return reply;
    }

    return HttpReply::createErrorReply(HttpReply::NotFound);
}

void WebServer::incomingConnection(qintptr socketDescriptor)
{
    if (!m_enabled)
        return;

    QSslSocket *socket = new QSslSocket();
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qCWarning(dcWebServer()) << "Could not set socket descriptor. Rejecting connection.";
        socket->close();
        delete socket;
        return;
    }

    // check webserver client
    bool existing = false;
    foreach (WebServerClient *client, m_webServerClients) {
        if (client->address() == socket->peerAddress()) {
            if (client->connections().count() >= 50) {
                qCWarning(dcWebServer()).noquote() << QString("Maximum connections for this client reached: rejecting connection from client %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
                socket->close();
                delete socket;
                return;
            }
            client->addConnection(socket);
            existing = true;
            break;
        }
    }

    if (!existing) {
        WebServerClient *webServerClient = new WebServerClient(socket->peerAddress());
        webServerClient->addConnection(socket);
        m_webServerClients.append(webServerClient);
    }

    // append the new client to the client list
    QUuid clientId = QUuid::createUuid();
    m_clientList.insert(clientId, socket);

    qCDebug(dcWebServer()).noquote() << QString("Webserver client %1:%2 connected").arg(socket->peerAddress().toString()).arg(socket->peerPort());

    if (m_configuration.sslEnabled) {
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

    // Check client
    if (clientId.isNull()) {
        qCWarning(dcWebServer()) << "Client not recognized";
        socket->close();
        return;
    }

    // Read HTTP request
    QByteArray data = socket->readAll();

    HttpRequest request;
    if (m_incompleteRequests.contains(socket)) {
        request = m_incompleteRequests.take(socket);
        request.appendData(data);
    } else {
        request = HttpRequest(data);
    }

    // Check if the request is complete
    if (!request.isComplete()) {
        m_incompleteRequests.insert(socket, request);
        return;
    }

    qCDebug(dcWebServerTraffic()) << "Received request from" << clientId.toString() << socket->peerAddress().toString() << request;

    // Check if the request is valid
    if (!request.isValid()) {
        qCDebug(dcWebServer()) << "Got invalid request:" << request.url().path();
        HttpReply *reply = HttpReply::createErrorReply(HttpReply::BadRequest);
        reply->setClientId(clientId);
        sendHttpReply(reply);
        reply->deleteLater();
        return;
    }

    // Check HTTP version
    if (request.httpVersion() != "HTTP/1.1" && request.httpVersion() != "HTTP/1.0") {
        qCDebug(dcWebServer()) << "HTTP version is not supported." << request.httpVersion();
        HttpReply *reply = HttpReply::createErrorReply(HttpReply::HttpVersionNotSupported);
        reply->setClientId(clientId);
        sendHttpReply(reply);
        reply->deleteLater();
        return;
    }

    qCDebug(dcWebServer()).noquote() << QString("Got valid request from %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()) << request.methodString() << request.url().path() << request.urlQuery().toString();

    // Reset timout
    foreach (WebServerClient *webserverClient, m_webServerClients) {
        if (webserverClient->address() == socket->peerAddress()) {
            webserverClient->resetTimout(socket);
            break;
        }
    }

    // Verify method
    if (request.method() == HttpRequest::Unhandled) {
        HttpReply *reply = HttpReply::createErrorReply(HttpReply::MethodNotAllowed);
        reply->setClientId(clientId);
        reply->setHeader(HttpReply::AllowHeader, "GET, PUT, POST, DELETE, OPTIONS");
        sendHttpReply(reply);
        reply->deleteLater();
        return;
    }

    // Check icon call
    if (request.url().path().startsWith("/icons/") && request.method() == HttpRequest::Get) {
        HttpReply *reply = processIconRequest(request.url().path());
        reply->setClientId(clientId);
        sendHttpReply(reply);
        reply->deleteLater();
        return;
    }

    // Check if this is a debug call
    if (request.url().path().startsWith("/debug")) {
        // Check if debug server is enabled
        if (NymeaCore::instance()->configuration()->debugServerEnabled()) {
            // Verify methods
            if (request.method() != HttpRequest::Get && request.method() != HttpRequest::Options) {
                HttpReply *reply = HttpReply::createErrorReply(HttpReply::MethodNotAllowed);
                reply->setClientId(clientId);
                reply->setHeader(HttpReply::AllowHeader, "GET, OPTIONS");
                sendHttpReply(reply);
                reply->deleteLater();
                return;
            }

            qCDebug(dcDebugServer()) << "Request:" << request.url().toString();
            HttpReply *reply = NymeaCore::instance()->debugServerHandler()->processDebugRequest(request.url().path(), request.urlQuery());
            reply->setClientId(clientId);

            // Handle async replies
            if (reply->type() == HttpReply::TypeAsync) {
                connect(reply, &HttpReply::finished, this, &WebServer::onAsyncReplyFinished);
                reply->startWait();
            } else {
                sendHttpReply(reply);
                reply->deleteLater();
            }
            return;
        } else {
            qCWarning(dcWebServer()) << "The debug server handler is disabled. You can enable it by adding \'debugServerEnabled=true\' in the \'nymead\' section of the nymead.conf file.";
            HttpReply *reply = HttpReply::createErrorReply(HttpReply::NotFound);
            reply->setClientId(clientId);
            sendHttpReply(reply);
            reply->deleteLater();
            return;
        }
    }

    // Check server.xml call
    if (request.url().path() == "/server.xml" && request.method() == HttpRequest::Get) {
        qCDebug(dcWebServer()) << "Server XML request call";
        HttpReply *reply = HttpReply::createSuccessReply();
        reply->setHeader(HttpReply::ContentTypeHeader, "text/xml");
        reply->setPayload(createServerXmlDocument(socket->localAddress()));
        reply->setClientId(clientId);
        sendHttpReply(reply);
        reply->deleteLater();
        return;
    }


    // Request for a file...
    if (request.method() == HttpRequest::Get) {
        // Check if the webinterface dir does exist, otherwise a filerequest is not relevant
        // FIXME: return a default webpage containing server information
        if (!QDir(m_configuration.publicFolder).exists()) {
            qDebug(dcWebServer()) << "Webinterface folder" << m_configuration.publicFolder << "does not exist.";
            HttpReply *reply = HttpReply::createErrorReply(HttpReply::NotFound);
            reply->setClientId(clientId);
            sendHttpReply(reply);
            reply->deleteLater();
            return;
        }

        QString path = fileName(request.url().path());
        if (!verifyFile(socket, path))
            return;

        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            qCDebug(dcWebServer()) << "Load file" << file.fileName();
            HttpReply *reply = HttpReply::createSuccessReply();

            // Check content type
            if (file.fileName().endsWith(".html")) {
                reply->setHeader(HttpReply::ContentTypeHeader, "text/html; charset=\"utf-8\";");
            } else if (file.fileName().endsWith(".css")) {
                reply->setHeader(HttpReply::ContentTypeHeader, "text/css; charset=\"utf-8\";");
            } else if (file.fileName().endsWith(".pdf")) {
                reply->setHeader(HttpReply::ContentTypeHeader, "application/pdf");
            } else if (file.fileName().endsWith(".js")) {
                reply->setHeader(HttpReply::ContentTypeHeader, "text/javascript; charset=\"utf-8\";");
            } else if (file.fileName().endsWith(".ttf")) {
                reply->setHeader(HttpReply::ContentTypeHeader, "application/x-font-ttf");
            } else if (file.fileName().endsWith(".eot")) {
                reply->setHeader(HttpReply::ContentTypeHeader, "application/vnd.ms-fontobject");
            } else if (file.fileName().endsWith(".woff")) {
                reply->setHeader(HttpReply::ContentTypeHeader, "application/x-font-woff");
            } else if (file.fileName().endsWith(".jpg") || file.fileName().endsWith(".jpeg")) {
                reply->setHeader(HttpReply::ContentTypeHeader, "image/jpeg");
            } else if (file.fileName().endsWith(".png") || file.fileName().endsWith(".PNG")) {
                reply->setHeader(HttpReply::ContentTypeHeader, "image/png");
            } else if (file.fileName().endsWith(".ico")) {
                reply->setHeader(HttpReply::ContentTypeHeader, "image/x-icon");
            } else if (file.fileName().endsWith(".svg")) {
                reply->setHeader(HttpReply::ContentTypeHeader, "image/svg+xml; charset=\"utf-8\";");
            }

            reply->setPayload(file.readAll());
            reply->setClientId(clientId);
            sendHttpReply(reply);
            reply->deleteLater();
            return;
        }
    }

    // Reject everything else...
    qCDebug(dcWebServer()) << "Unknown message received.";
    HttpReply *reply = HttpReply::createErrorReply(HttpReply::NotImplemented);
    reply->setClientId(clientId);
    sendHttpReply(reply);
    reply->deleteLater();
}

void WebServer::onDisconnected()
{    
    QSslSocket* socket = static_cast<QSslSocket *>(sender());

    // Remove connection from server client
    foreach (WebServerClient *client, m_webServerClients) {
        if (client->address() == socket->peerAddress()) {
            client->removeConnection(socket);
            if (client->connections().isEmpty()) {
                m_webServerClients.removeAll(client);
                client->deleteLater();
            }
            break;
        }
    }

    qCDebug(dcWebServer()).noquote() << QString("Webserver client disonnected %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());

    // clean up
    QUuid clientId = m_clientList.key(socket);
    m_clientList.remove(clientId);
    m_incompleteRequests.remove(socket);
    emit clientDisconnected(clientId);

    socket->deleteLater();
}

void WebServer::onEncrypted()
{
    QSslSocket* socket = static_cast<QSslSocket *>(sender());
    qCDebug(dcWebServer()).noquote() << QString("Encrypted connection %1:%2 successfully established.").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    emit clientConnected(m_clientList.key(socket));
}

void WebServer::onError(QAbstractSocket::SocketError error)
{
    QSslSocket* socket = static_cast<QSslSocket *>(sender());
    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        qCDebug(dcWebServer()).noquote() << QString("Client socket error %1:%2 ->").arg(socket->peerAddress().toString()).arg(socket->peerPort()) << socket->errorString();
        break;
    default:
        qCWarning(dcWebServer()).noquote() << QString("Client socket error %1:%2 ->").arg(socket->peerAddress().toString()).arg(socket->peerPort()) << socket->errorString();
        break;
    }
}

void WebServer::onAsyncReplyFinished()
{
    HttpReply *reply = qobject_cast<HttpReply*>(sender());
    qCDebug(dcWebServer()) << "Async reply finished";

    // check if the reply timeouted
    if (reply->timedOut()) {
        reply->clear();
        reply->setHttpStatusCode(HttpReply::GatewayTimeout);
    }

    sendHttpReply(reply);
    reply->deleteLater();
}

/*! Set the configuration of this \l{WebServer} to the given \a config.
 *
 * \sa WebServerConfiguration
 */
void WebServer::setConfiguration(const WebServerConfiguration &config)
{
    m_configuration = config;
}

/*! Sets the server name to the given \a serverName. */
void WebServer::setServerName(const QString &serverName)
{
    m_serverName = serverName;
}

/*! Returns true if this \l{WebServer} started successfully. */
bool WebServer::startServer()
{
    if (!listen(m_configuration.address, static_cast<quint16>(m_configuration.port))) {
        qCWarning(dcWebServer()) << "Webserver could not listen on" << serverUrl().toString() << errorString();
        m_enabled = false;
        return false;
    }

    qCDebug(dcWebServer()) << "Started web server on" << serverUrl().toString();

    m_enabled = true;
    return true;
}

/*! Returns true if this \l{WebServer} stopped successfully. */
bool WebServer::stopServer()
{
    foreach (QSslSocket *client, m_clientList.values())
        client->close();

    close();
    m_enabled = false;
    qCDebug(dcWebServer()) << "Webserver closed.";
    return true;
}

WebServerConfiguration WebServer::configuration() const
{
    return m_configuration;
}


QByteArray WebServer::createServerXmlDocument(QHostAddress address)
{
    QByteArray uuid = NymeaCore::instance()->configuration()->serverUuid().toString().remove(QRegExp("[{}]")).toUtf8();

    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.setAutoFormatting(true);
    writer.writeStartDocument("1.0");
    writer.writeStartElement("root");
    writer.writeAttribute("xmlns", "urn:schemas-upnp-org:device-1-0");

    writer.writeStartElement("specVersion");
    writer.writeTextElement("major", "1");
    writer.writeTextElement("minor", "1");
    writer.writeEndElement(); // specVersion

    QString presentationUrl = QString("%1://%2:%3")
            .arg(m_configuration.sslEnabled ? "https" : "http")
            .arg(address.toString())
            .arg(m_configuration.port);
    writer.writeStartElement("device");
    writer.writeTextElement("presentationURL", presentationUrl);
    writer.writeTextElement("deviceType", "urn:schemas-upnp-org:device:Basic:1");
    writer.writeTextElement("friendlyName", NymeaCore::instance()->configuration()->serverName());
    writer.writeTextElement("manufacturer", "nymea GmbH");
    writer.writeTextElement("manufacturerURL", "http://nymea.io");
    writer.writeTextElement("modelDescription", "IoT server");
    writer.writeTextElement("modelName", "nymead");
    writer.writeTextElement("modelNumber", NYMEA_VERSION_STRING);
    writer.writeTextElement("modelURL", "http://nymea.io"); // (optional)
    writer.writeTextElement("UDN", "uuid:" + uuid);

    writer.writeStartElement("iconList");

    writer.writeStartElement("icon");
    writer.writeTextElement("mimetype", "image/png");
    writer.writeTextElement("width", "8");
    writer.writeTextElement("height", "8");
    writer.writeTextElement("depth", "8");
    writer.writeTextElement("url", "/icons/nymea-logo-8x8.png");
    writer.writeEndElement(); // icon

    writer.writeStartElement("icon");
    writer.writeTextElement("mimetype", "image/png");
    writer.writeTextElement("width", "16");
    writer.writeTextElement("height", "16");
    writer.writeTextElement("depth", "8");
    writer.writeTextElement("url", "/icons/nymea-logo-16x16.png");
    writer.writeEndElement(); // icon

    writer.writeStartElement("icon");
    writer.writeTextElement("mimetype", "image/png");
    writer.writeTextElement("width", "22");
    writer.writeTextElement("height", "22");
    writer.writeTextElement("depth", "8");
    writer.writeTextElement("url", "/icons/nymea-logo-22x22.png");
    writer.writeEndElement(); // icon

    writer.writeStartElement("icon");
    writer.writeTextElement("mimetype", "image/png");
    writer.writeTextElement("width", "32");
    writer.writeTextElement("height", "32");
    writer.writeTextElement("depth", "8");
    writer.writeTextElement("url", "/icons/nymea-logo-32x32.png");
    writer.writeEndElement(); // icon

    writer.writeStartElement("icon");
    writer.writeTextElement("mimetype", "image/png");
    writer.writeTextElement("width", "48");
    writer.writeTextElement("height", "48");
    writer.writeTextElement("depth", "8");
    writer.writeTextElement("url", "/icons/nymea-logo-48x48.png");
    writer.writeEndElement(); // icon

    writer.writeStartElement("icon");
    writer.writeTextElement("mimetype", "image/png");
    writer.writeTextElement("width", "64");
    writer.writeTextElement("height", "64");
    writer.writeTextElement("depth", "8");
    writer.writeTextElement("url", "/icons/nymea-logo-64x64.png");
    writer.writeEndElement(); // icon

    writer.writeStartElement("icon");
    writer.writeTextElement("mimetype", "image/png");
    writer.writeTextElement("width", "120");
    writer.writeTextElement("height", "120");
    writer.writeTextElement("depth", "8");
    writer.writeTextElement("url", "/icons/nymea-logo-120x120.png");
    writer.writeEndElement(); // icon

    writer.writeStartElement("icon");
    writer.writeTextElement("mimetype", "image/png");
    writer.writeTextElement("width", "128");
    writer.writeTextElement("height", "128");
    writer.writeTextElement("depth", "8");
    writer.writeTextElement("url", "/icons/nymea-logo-128x128.png");
    writer.writeEndElement(); // icon

    writer.writeStartElement("icon");
    writer.writeTextElement("mimetype", "image/png");
    writer.writeTextElement("width", "256");
    writer.writeTextElement("height", "256");
    writer.writeTextElement("depth", "8");
    writer.writeTextElement("url", "/icons/nymea-logo-256x256.png");
    writer.writeEndElement(); // icon

    writer.writeStartElement("icon");
    writer.writeTextElement("mimetype", "image/png");
    writer.writeTextElement("width", "512");
    writer.writeTextElement("height", "512");
    writer.writeTextElement("depth", "8");
    writer.writeTextElement("url", "/icons/nymea-logo-512x512.png");
    writer.writeEndElement(); // icon

    writer.writeEndElement(); // iconList

    writer.writeStartElement("serviceList");

    int counter = 1;
    int sslCounter = 1;
    foreach (const ServerConfiguration &config, NymeaCore::instance()->configuration()->webSocketServerConfigurations()) {
        if (config.address == QHostAddress("0.0.0.0") || config.address == address) {
            writer.writeStartElement("service");
            writer.writeTextElement("serviceType", QString("urn:nymea.io:service:%1:%2").arg(config.sslEnabled ? "wss" : "ws").arg(config.sslEnabled ? sslCounter : counter));
            writer.writeTextElement("serviceId", QString("urn:nymea.io:serviceId:%1:%2").arg(config.sslEnabled ? "wss" : "ws").arg(config.sslEnabled ? sslCounter++ : counter++));
            QString url = QString("%1%2:%3").arg(config.sslEnabled ? "wss://" : "ws://").arg(address.toString()).arg(config.port);
            writer.writeTextElement("SCPDURL", url);
            writer.writeEndElement(); // service
        }
    }

    counter = 1;
    sslCounter = 1;
    foreach (const ServerConfiguration &config, NymeaCore::instance()->configuration()->tcpServerConfigurations()) {
        if (config.address == QHostAddress("0.0.0.0") || config.address == address) {
            writer.writeStartElement("service");
            writer.writeTextElement("serviceType", QString("urn:nymea.io:service:%1:%2").arg(config.sslEnabled ? "nymeas" : "nymea").arg(config.sslEnabled ? sslCounter : counter));
            writer.writeTextElement("serviceId", QString("urn:nymea.io:serviceId:%1:%2").arg(config.sslEnabled ? "nymeas" : "nymea").arg(config.sslEnabled ? sslCounter++ : counter++));
            QString url = QString("%1%2:%3").arg(config.sslEnabled ? "nymeas://" : "nymea://").arg(address.toString()).arg(config.port);
            writer.writeTextElement("SCPDURL", url);
            writer.writeEndElement(); // service
        }
    }

    writer.writeEndElement(); // serviceList

    writer.writeEndElement(); // device
    writer.writeEndElement(); // root
    writer.writeEndDocument();
    return data;
}

/*!
    \class nymeaserver::WebServerClient
    \brief This class represents a client the web server for nymead.

    \ingroup server
    \inmodule core

    The \l{WebServerClient} represents a client for the nymea \l{WebServer}. Each client can
    have up to 50 connections and each connection will timeout after 12 seconds if the
    connection will not be used.

    If all connections of a \l{WebServerClient} are closed, the client will be removed from
    system.

    \sa WebServer
*/

/*! Constructs a \l{WebServerClient} with the given \a address and \a parent. */
WebServerClient::WebServerClient(const QHostAddress &address, QObject *parent):
    QObject(parent),
    m_address(address)
{
}

/*! Returns the address of this \l{WebServerClient}. */
QHostAddress WebServerClient::address() const
{
    return m_address;
}

/*! Returns the list of connections (sockets) of this \l{WebServerClient}. */
QList<QSslSocket *> WebServerClient::connections()
{
    return m_connections;
}

/*! Adds a new connection (\a socket) to this \l{WebServerClient}. A \l{WebServerClient}
 *  can have up to 50 connecections. The connection will timout and closed if the client
 *  does not use the connection for 65 seconds.
 */
void WebServerClient::addConnection(QSslSocket *socket)
{
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(65000);
    connect(timer, &QTimer::timeout, this, &WebServerClient::onTimout);

    m_runningConnections.insert(timer, socket);
    m_connections.append(socket);

    timer->start();
}

/*! Removes a connection the given \a socket from the connection list of this \l{WebServerClient}. */
void WebServerClient::removeConnection(QSslSocket *socket)
{
    QTimer *timer = m_runningConnections.key(socket);
    m_runningConnections.remove(timer);
    m_connections.removeAll(socket);

    delete timer;
}

/*! Resets the connection timeout for the given \a socket. If the socket will not be used for 12 seconds the
 *  connection will be closed.
 */
void WebServerClient::resetTimout(QSslSocket *socket)
{
    QTimer *timer = nullptr;
    timer = m_runningConnections.key(socket);
    if (timer)
        timer->start();
}

void WebServerClient::onTimout()
{
    QTimer *timer =  static_cast<QTimer *>(sender());
    QSslSocket *socket = m_runningConnections.value(timer);
    qCDebug(dcWebServer()).noquote() << QString("Client connection timout %1:%2 -> closing connection").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    removeConnection(socket);
    socket->close();
}

}
