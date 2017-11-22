/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
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
    \class guhserver::ServerManager
    \brief This class represents the manager of all server interfaces of the guh server.

    \ingroup server
    \inmodule core

    The \l{ServerManager} starts the \l{JsonRPCServer} and the \l{RestServer}. He also loads
    and provides the SSL configurations for the secure \l{WebServer} and \l{WebSocketServer}
    connection.

    \sa JsonRPCServer, RestServer
*/

#include "servermanager.h"
#include "guhcore.h"
#include "certificategenerator.h"

#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>

namespace guhserver {

/*! Constructs a \l{ServerManager} with the given \a parent. */
ServerManager::ServerManager(GuhConfiguration* configuration, QObject *parent) :
    QObject(parent),
    m_sslConfiguration(QSslConfiguration())
{
    // TODO: check this


    if (!QSslSocket::supportsSsl()) {
        qCWarning(dcConnection) << "SSL is not supported/installed on this platform.";
    } else {
        qCDebug(dcConnection) << "SSL library version:" << QSslSocket::sslLibraryVersionString();

        QString configCertificateFileName = configuration->sslCertificate();
        QString configKeyFileName = configuration->sslCertificateKey();

        QString fallbackCertificateFileName = GuhSettings::storagePath() + "/certs/guhd-certificate.crt";
        QString fallbackKeyFileName = GuhSettings::storagePath() + "/certs/guhd-certificate.key";

        bool certsLoaded = false;
        if (loadCertificate(configKeyFileName, configCertificateFileName)) {
            qCDebug(dcConnection) << "Using SSL certificate:" << configKeyFileName;
            certsLoaded = true;
        } else if (loadCertificate(fallbackKeyFileName, fallbackCertificateFileName)) {
            certsLoaded = true;
            qCWarning(dcConnection) << "Using fallback self-signed SSL certificate:" << fallbackCertificateFileName;
        } else {
            qCDebug(dcConnection) << "Generating self signed certificates...";
            CertificateGenerator::generate(fallbackCertificateFileName, fallbackKeyFileName);
            if (loadCertificate(fallbackKeyFileName, fallbackCertificateFileName)) {
                qCWarning(dcConnection) << "Using newly created self-signed SSL certificate:" << fallbackCertificateFileName;
                certsLoaded = true;
            } else {
                qCWarning(dcConnection) << "Failed to load SSL certificates. SSL encryption disabled.";
            }
        }
        if (certsLoaded) {
            // Enable this when we can
            // Debian jessie doesn't have better options yet and the client apps currently only support up to TLS 1.1
//            m_sslConfiguration.setProtocol(QSsl::TlsV1_1OrLater);
            m_sslConfiguration.setPrivateKey(m_certificateKey);
            m_sslConfiguration.setLocalCertificate(m_certificate);
        }
    }

    // Interfaces
    m_jsonServer = new JsonRPCServer(m_sslConfiguration, this);
    m_restServer = new RestServer(m_sslConfiguration, this);


    // Transports
    MockTcpServer *tcpServer = new MockTcpServer(this);
    m_jsonServer->registerTransportInterface(tcpServer, true);
    tcpServer->startServer();
    foreach (const ServerConfiguration &config, configuration->tcpServerConfigurations()) {
        TcpServer *tcpServer = new TcpServer(config, m_sslConfiguration, this);
        m_jsonServer->registerTransportInterface(tcpServer, config.authenticationEnabled);
        m_tcpServers.insert(config.id, tcpServer);
        tcpServer->startServer();
    }

    foreach (const ServerConfiguration &config, configuration->webSocketServerConfigurations()) {
        WebSocketServer *webSocketServer = new WebSocketServer(config, m_sslConfiguration, this);
        m_jsonServer->registerTransportInterface(webSocketServer, config.authenticationEnabled);
        m_webSocketServers.insert(config.id, webSocketServer);
        webSocketServer->startServer();
    }

    m_bluetoothServer = new BluetoothServer(this);
    m_jsonServer->registerTransportInterface(m_bluetoothServer, true);
    if (configuration->bluetoothServerEnabled()) {
        m_bluetoothServer->startServer();
    }

    foreach (const WebServerConfiguration &config, configuration->webServerConfigurations()) {
        WebServer *webServer = new WebServer(config, m_sslConfiguration, this);
        m_restServer->registerWebserver(webServer);
        m_webServers.insert(config.id, webServer);
    }

    connect(configuration, &GuhConfiguration::serverNameChanged, this, &ServerManager::onServerNameChanged);
    connect(configuration, &GuhConfiguration::tcpServerConfigurationChanged, this, &ServerManager::tcpServerConfigurationChanged);
    connect(configuration, &GuhConfiguration::tcpServerConfigurationRemoved, this, &ServerManager::tcpServerConfigurationRemoved);
    connect(configuration, &GuhConfiguration::webSocketServerConfigurationChanged, this, &ServerManager::webSocketServerConfigurationChanged);
    connect(configuration, &GuhConfiguration::webSocketServerConfigurationRemoved, this, &ServerManager::webSocketServerConfigurationRemoved);
    connect(configuration, &GuhConfiguration::webServerConfigurationChanged, this, &ServerManager::webServerConfigurationChanged);
    connect(configuration, &GuhConfiguration::webServerConfigurationRemoved, this, &ServerManager::webServerConfigurationRemoved);
}

/*! Returns the pointer to the created \l{JsonRPCServer} in this \l{ServerManager}. */
JsonRPCServer *ServerManager::jsonServer() const
{
    return m_jsonServer;
}

/*! Returns the pointer to the created \l{RestServer} in this \l{ServerManager}. */
RestServer *ServerManager::restServer() const
{
    return m_restServer;
}

BluetoothServer *ServerManager::bluetoothServer() const
{
    return m_bluetoothServer;
}

MockTcpServer *ServerManager::mockTcpServer() const
{
    return m_mockTcpServer;
}

void ServerManager::onServerNameChanged()
{
    qCDebug(dcConnection()) << "Server name changed";

    foreach (WebSocketServer *websocketServer, m_webSocketServers.values()) {
        websocketServer->resetAvahiService();
    }

    foreach (WebServer *webServer, m_webServers.values()) {
        webServer->resetAvahiService();
    }
}

void ServerManager::tcpServerConfigurationChanged(const QString &id)
{
    ServerConfiguration config = GuhCore::instance()->configuration()->tcpServerConfigurations().value(id);
    TcpServer *server = m_tcpServers.value(id);
    if (server) {
        qDebug(dcConnection) << "Restarting TCP server for" << config.address << config.port << "SSL" << (config.sslEnabled ? "enabled" : "disabled") << "Authentication" << (config.authenticationEnabled ? "enabled" : "disabled");
        server->stopServer();
        server->setConfiguration(config);
    } else {
        qDebug(dcConnection) << "Received a TCP Server config change event but don't have a TCP Server instance for it. Creating new Server instance.";
        server = new TcpServer(config, m_sslConfiguration, this);
        m_tcpServers.insert(config.id, server);
    }
    m_jsonServer->registerTransportInterface(server, config.authenticationEnabled);
    server->startServer();
}

void ServerManager::tcpServerConfigurationRemoved(const QString &id)
{
    if (!m_tcpServers.contains(id)) {
        qWarning(dcConnection) << "Received a TCP Server config removed event but don't have a TCP Server instance for it.";
        return;
    }
    TcpServer *server = m_tcpServers.take(id);
    m_jsonServer->unregisterTransportInterface(server);
    server->stopServer();
    server->deleteLater();
}

void ServerManager::webSocketServerConfigurationChanged(const QString &id)
{
    WebSocketServer *server = m_webSocketServers.value(id);
    ServerConfiguration config = GuhCore::instance()->configuration()->webSocketServerConfigurations().value(id);
    if (server) {
        qDebug(dcConnection) << "Restarting WebSocket server for" << config.address << config.port << "SSL" << (config.sslEnabled ? "enabled" : "disabled") << "Authentication" << (config.authenticationEnabled ? "enabled" : "disabled");
        server->stopServer();
        server->setConfiguration(config);
    } else {
        qDebug(dcConnection) << "Received a WebSocket Server config change event but don't have a WebSocket Server instance for it. Creating new instance.";
        server = new WebSocketServer(config, m_sslConfiguration, this);
        m_webSocketServers.insert(server->configuration().id, server);
    }
    m_jsonServer->registerTransportInterface(server, config.authenticationEnabled);
    server->startServer();
}

void ServerManager::webSocketServerConfigurationRemoved(const QString &id)
{
    if (!m_webSocketServers.contains(id)) {
        qWarning(dcConnection) << "Received a WebSocket Server config removed event but don't have a WebSocket Server instance for it.";
        return;
    }
    WebSocketServer *server = m_webSocketServers.take(id);
    m_jsonServer->unregisterTransportInterface(server);
    server->stopServer();
    server->deleteLater();
}

void ServerManager::webServerConfigurationChanged(const QString &id)
{
    WebServerConfiguration config = GuhCore::instance()->configuration()->webServerConfigurations().value(id);
    WebServer *server = m_webServers.value(id);
    if (server) {
        qDebug(dcConnection) << "Restarting Web server for" << config.address << config.port << "SSL" << (config.sslEnabled ? "enabled" : "disabled") << "Authentication" << (config.authenticationEnabled ? "enabled" : "disabled");
        server->stopServer();
        server->reconfigureServer(config);
    } else {
        qDebug(dcConnection) << "Received a Web Server config change event but don't have a Web Server instance for it. Creating new WebServer instance on" << config.address.toString() << config.port << "(SSL:" << config.sslEnabled << ")";
        server = new WebServer(config, m_sslConfiguration, this);
        m_restServer->registerWebserver(server);
        m_webServers.insert(config.id, server);
    }
}

void ServerManager::webServerConfigurationRemoved(const QString &id)
{
    if (!m_webServers.contains(id)) {
        qWarning(dcConnection) << "Received a Web Server config removed event but don't have a Web Server instance for it.";
        return;
    }
    WebServer *server = m_webServers.take(id);
    server->stopServer();
    server->deleteLater();
}

bool ServerManager::loadCertificate(const QString &certificateKeyFileName, const QString &certificateFileName)
{
    QFile certificateKeyFile(certificateKeyFileName);
    if (!certificateKeyFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcConnection) << "Could not open" << certificateKeyFile.fileName() << ":" << certificateKeyFile.errorString();
        return false;
    }
    m_certificateKey = QSslKey(certificateKeyFile.readAll(), QSsl::Rsa);
    qCDebug(dcConnection) << "Loaded private certificate key " << certificateKeyFileName;
    certificateKeyFile.close();

    QFile certificateFile(certificateFileName);
    if (!certificateFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcConnection) << "Could not open" << certificateFile.fileName() << ":" << certificateFile.errorString();
        return false;
    }
    m_certificate = QSslCertificate(certificateFile.readAll());
    qCDebug(dcConnection) << "Loaded certificate file " << certificateFileName;
    certificateFile.close();

    return true;
}

}
