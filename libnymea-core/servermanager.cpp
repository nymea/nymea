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
    \class nymeaserver::ServerManager
    \brief This class represents the manager of all server interfaces of the nymea server.

    \ingroup server
    \inmodule core

    The \l{ServerManager} starts the \l{JsonRPCServer} and the \l{RestServer}. He also loads
    and provides the SSL configurations for the secure \l{WebServer} and \l{WebSocketServer}
    connection.

    \sa JsonRPCServer, RestServer
*/

#include "servermanager.h"
#include "nymeacore.h"
#include "certificategenerator.h"
#include "nymeasettings.h"
#include "platform/platform.h"
#include "platform/platformzeroconfcontroller.h"
#include "version.h"

#include "jsonrpc/jsonrpcserverimplementation.h"
#include "servers/mocktcpserver.h"
#include "servers/tcpserver.h"
#include "servers/websocketserver.h"
#include "servers/webserver.h"
#include "servers/bluetoothserver.h"
#include "servers/mqttbroker.h"
#include "servers/tunnelproxyserver.h"

#include "network/zeroconf/zeroconfservicepublisher.h"

#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>

namespace nymeaserver {

ServerManager::ServerManager(Platform *platform, NymeaConfiguration *configuration, const QStringList &additionalInterfaces, QObject *parent) :
    QObject(parent),
    m_platform(platform),
    m_nymeaConfiguration(configuration),
    m_sslConfiguration(QSslConfiguration())
{
    if (!QSslSocket::supportsSsl()) {
        qCWarning(dcServerManager()) << "SSL is not supported/installed on this platform.";
    } else {
        qCDebug(dcServerManager()) << "SSL library version:" << QSslSocket::sslLibraryVersionString();

        QString configCertificateFileName = configuration->sslCertificate();
        QString configKeyFileName = configuration->sslCertificateKey();

        QString fallbackCertificateFileName = NymeaSettings::storagePath() + "/certs/nymead-certificate.crt";
        QString fallbackKeyFileName = NymeaSettings::storagePath() + "/certs/nymead-certificate.key";

        bool certsLoaded = false;
        if (!configKeyFileName.isEmpty() && !configCertificateFileName.isEmpty() && loadCertificate(configKeyFileName, configCertificateFileName)) {
            qCDebug(dcServerManager()) << "Using SSL certificate:" << configCertificateFileName;
            certsLoaded = true;
        } else if (!fallbackKeyFileName.isEmpty() && !fallbackCertificateFileName.isEmpty() && loadCertificate(fallbackKeyFileName, fallbackCertificateFileName)) {
            certsLoaded = true;
            qCDebug(dcServerManager()) << "Using fallback self-signed SSL certificate:" << fallbackCertificateFileName;
        } else {
            qCDebug(dcServerManager()) << "Generating self signed certificates...";
            CertificateGenerator::generate(fallbackCertificateFileName, fallbackKeyFileName);
            if (loadCertificate(fallbackKeyFileName, fallbackCertificateFileName)) {
                qCWarning(dcServerManager()) << "Using newly created self-signed SSL certificate:" << fallbackCertificateFileName;
                certsLoaded = true;
            } else {
                qCWarning(dcServerManager()) << "Failed to load SSL certificates. SSL encryption disabled.";
            }
        }
        if (certsLoaded) {
            // Update this to 1.3 when minimum required Qt is 5.12 (and known client apps can deal with it)
            m_sslConfiguration.setProtocol(QSsl::TlsV1_2OrLater);
            m_sslConfiguration.setPrivateKey(m_certificateKey);
            m_sslConfiguration.setLocalCertificate(m_certificate);
        }
    }

    // Interfaces
    m_jsonServer = new JsonRPCServerImplementation(m_sslConfiguration, this);

    // Transports
    MockTcpServer *tcpServer = new MockTcpServer(this);
    m_jsonServer->registerTransportInterface(tcpServer, true);
    tcpServer->startServer();

    foreach (const QString &interfaceString, additionalInterfaces) {
        QUrl additionalInterface(interfaceString);
        ServerConfiguration config;
        config.id = "tmp-" + additionalInterface.host();
        config.address = additionalInterface.host();
        config.port = additionalInterface.port();
        TransportInterface *server = nullptr;
        QString serverType, serviceType;
        if (additionalInterface.scheme().startsWith("nymea")) {
            config.sslEnabled = additionalInterface.scheme().startsWith("nymeas");
            server = new TcpServer(config, m_sslConfiguration, this);
            m_tcpServers.insert(config.id, qobject_cast<TcpServer*>(server));
            serverType = "tcp";
            serviceType = "_jsonrpc._tcp";
        } else if (additionalInterface.scheme().startsWith("ws")) {
            config.sslEnabled = additionalInterface.scheme().startsWith("wss");
            server = new WebSocketServer(config, m_sslConfiguration, this);
            m_webSocketServers.insert(config.id, qobject_cast<WebSocketServer*>(server));
            serverType = "ws";
            serviceType = "_ws._tcp";
        }
        if (server && server->startServer()) {
            registerZeroConfService(config, serverType, serviceType);
        }
    }

    foreach (const ServerConfiguration &config, configuration->tcpServerConfigurations()) {
        TcpServer *tcpServer = new TcpServer(config, m_sslConfiguration, this);
        m_jsonServer->registerTransportInterface(tcpServer, config.authenticationEnabled);
        m_tcpServers.insert(config.id, tcpServer);
        if (tcpServer->startServer()) {
            registerZeroConfService(config, "tcp", "_jsonrpc._tcp");
        }
    }

    foreach (const ServerConfiguration &config, configuration->webSocketServerConfigurations()) {
        WebSocketServer *webSocketServer = new WebSocketServer(config, m_sslConfiguration, this);
        m_jsonServer->registerTransportInterface(webSocketServer, config.authenticationEnabled);
        m_webSocketServers.insert(config.id, webSocketServer);
        if (webSocketServer->startServer()) {
            registerZeroConfService(config, "ws", "_ws._tcp");
        }
    }

    m_bluetoothServer = new BluetoothServer(this);
    m_jsonServer->registerTransportInterface(m_bluetoothServer, true);
    if (configuration->bluetoothServerEnabled()) {
        m_bluetoothServer->startServer();
    }

    foreach (const TunnelProxyServerConfiguration &config, configuration->tunnelProxyServerConfigurations()) {
        TunnelProxyServer *tunnelProxyServer = new TunnelProxyServer(configuration->serverName(), configuration->serverUuid(), config, this);
        qCDebug(dcServerManager()) << "Creating tunnel proxy server using" << config;
        m_tunnelProxyServers.insert(config.id, tunnelProxyServer);
        connect(tunnelProxyServer, &TunnelProxyServer::runningChanged, this, [this, tunnelProxyServer](bool running){
            if (running) {
                // Note: enable authentication in any case, we don't want to expose unprotected access trough the internet
                m_jsonServer->registerTransportInterface(tunnelProxyServer, true);
            } else {
                m_jsonServer->unregisterTransportInterface(tunnelProxyServer);
            }
        });

        // FIXME: make use of SSL over tunnel proxy connections
        // FIXME: make sure the authentication is enabled for the tunnel proxy

        tunnelProxyServer->startServer();
    }

    foreach (const WebServerConfiguration &config, configuration->webServerConfigurations()) {
        WebServer *webServer = new WebServer(config, m_sslConfiguration, this);
        m_webServers.insert(config.id, webServer);
        if (webServer->startServer()) {
            registerZeroConfService(config, "http", "_http._tcp");
        }
    }

    m_mqttBroker = new MqttBroker(this);
    foreach (const ServerConfiguration &config, configuration->mqttServerConfigurations()) {
        if (m_mqttBroker->startServer(config)) {
            registerZeroConfService(config, "mqtt", "_mqtt._tcp");
        }
    }
    m_mqttBroker->updatePolicies(configuration->mqttPolicies().values());

    connect(configuration, &NymeaConfiguration::tcpServerConfigurationChanged, this, &ServerManager::tcpServerConfigurationChanged);
    connect(configuration, &NymeaConfiguration::tcpServerConfigurationRemoved, this, &ServerManager::tcpServerConfigurationRemoved);
    connect(configuration, &NymeaConfiguration::webSocketServerConfigurationChanged, this, &ServerManager::webSocketServerConfigurationChanged);
    connect(configuration, &NymeaConfiguration::webSocketServerConfigurationRemoved, this, &ServerManager::webSocketServerConfigurationRemoved);
    connect(configuration, &NymeaConfiguration::webServerConfigurationChanged, this, &ServerManager::webServerConfigurationChanged);
    connect(configuration, &NymeaConfiguration::webServerConfigurationRemoved, this, &ServerManager::webServerConfigurationRemoved);
    connect(configuration, &NymeaConfiguration::mqttServerConfigurationChanged, this, &ServerManager::mqttServerConfigurationChanged);
    connect(configuration, &NymeaConfiguration::mqttServerConfigurationRemoved, this, &ServerManager::mqttServerConfigurationRemoved);
    connect(configuration, &NymeaConfiguration::mqttPolicyChanged, this, &ServerManager::mqttPolicyChanged);
    connect(configuration, &NymeaConfiguration::mqttPolicyRemoved, this, &ServerManager::mqttPolicyRemoved);
    connect(configuration, &NymeaConfiguration::tunnelProxyServerConfigurationChanged, this, &ServerManager::tunnelProxyServerConfigurationChanged);
    connect(configuration, &NymeaConfiguration::tunnelProxyServerConfigurationRemoved, this, &ServerManager::tunnelProxyServerConfigurationRemoved);
    connect(configuration, &NymeaConfiguration::cloudEnabledChanged, this, &ServerManager::cloudEnabledChanged);
}

/*! Returns the pointer to the created \l{JsonRPCServer} in this \l{ServerManager}. */
JsonRPCServerImplementation *ServerManager::jsonServer() const
{
    return m_jsonServer;
}

/*! Returns the pointer to the created \l{BluetoothServer} in this \l{ServerManager}. */
BluetoothServer *ServerManager::bluetoothServer() const
{
    return m_bluetoothServer;
}

/*! Returns the pointer to the created MockTcpServer in this \l{ServerManager}. */
MockTcpServer *ServerManager::mockTcpServer() const
{
    return m_mockTcpServer;
}

MqttBroker *ServerManager::mqttBroker() const
{
    return m_mqttBroker;
}

void ServerManager::tcpServerConfigurationChanged(const QString &id)
{
    ServerConfiguration config = NymeaCore::instance()->configuration()->tcpServerConfigurations().value(id);
    TcpServer *server = m_tcpServers.value(id);
    if (server) {
        qCDebug(dcServerManager()) << "Restarting TCP server for" << config.address << config.port << "SSL" << (config.sslEnabled ? "enabled" : "disabled") << "Authentication" << (config.authenticationEnabled ? "enabled" : "disabled");
        unregisterZeroConfService(config.id, "tcp");
        server->stopServer();
        server->setConfiguration(config);
    } else {
        qCDebug(dcServerManager()) << "Received a TCP Server config change event but don't have a TCP Server instance for it. Creating new Server instance.";
        server = new TcpServer(config, m_sslConfiguration, this);
        m_tcpServers.insert(config.id, server);
    }
    m_jsonServer->registerTransportInterface(server, config.authenticationEnabled);
    if (server->startServer()) {
        registerZeroConfService(config, "tcp", "_jsonrpc._tcp");
    }
}

void ServerManager::tcpServerConfigurationRemoved(const QString &id)
{
    if (!m_tcpServers.contains(id)) {
        qCWarning(dcServerManager()) << "Received a TCP Server config removed event but don't have a TCP Server instance for it.";
        return;
    }
    TcpServer *server = m_tcpServers.take(id);
    m_jsonServer->unregisterTransportInterface(server);
    unregisterZeroConfService(id, "tcp");
    server->stopServer();
    server->deleteLater();
}

void ServerManager::webSocketServerConfigurationChanged(const QString &id)
{
    WebSocketServer *server = m_webSocketServers.value(id);
    ServerConfiguration config = NymeaCore::instance()->configuration()->webSocketServerConfigurations().value(id);
    if (server) {
        qCDebug(dcServerManager()) << "Restarting WebSocket server for" << config.address << config.port << "SSL" << (config.sslEnabled ? "enabled" : "disabled") << "Authentication" << (config.authenticationEnabled ? "enabled" : "disabled");
        unregisterZeroConfService(id, "ws");
        server->stopServer();
        server->setConfiguration(config);
    } else {
        qCDebug(dcServerManager()) << "Received a WebSocket Server config change event but don't have a WebSocket Server instance for it. Creating new instance.";
        server = new WebSocketServer(config, m_sslConfiguration, this);
        m_webSocketServers.insert(server->configuration().id, server);
    }
    m_jsonServer->registerTransportInterface(server, config.authenticationEnabled);
    if (server->startServer()) {
        registerZeroConfService(config, "ws", "_ws._tcp");
    }
}

void ServerManager::webSocketServerConfigurationRemoved(const QString &id)
{
    if (!m_webSocketServers.contains(id)) {
        qCWarning(dcServerManager()) << "Received a WebSocket Server config removed event but don't have a WebSocket Server instance for it.";
        return;
    }
    WebSocketServer *server = m_webSocketServers.take(id);
    m_jsonServer->unregisterTransportInterface(server);
    unregisterZeroConfService(id, "ws");
    server->stopServer();
    server->deleteLater();
}

void ServerManager::webServerConfigurationChanged(const QString &id)
{
    WebServerConfiguration config = NymeaCore::instance()->configuration()->webServerConfigurations().value(id);
    WebServer *server = m_webServers.value(id);
    if (server) {
        qCDebug(dcServerManager()) << "Restarting Web server for" << config.address << config.port << "SSL" << (config.sslEnabled ? "enabled" : "disabled") << "Authentication" << (config.authenticationEnabled ? "enabled" : "disabled");
        unregisterZeroConfService(id, "http");
        server->stopServer();
        server->setConfiguration(config);
    } else {
        qCDebug(dcServerManager()) << "Received a Web Server config change event but don't have a Web Server instance for it. Creating new WebServer instance on" << config.address << config.port << "(SSL:" << config.sslEnabled << ")";
        server = new WebServer(config, m_sslConfiguration, this);
        m_webServers.insert(config.id, server);
    }
    if (server->startServer()) {
        registerZeroConfService(config, "http", "_http._tcp");
    }
}

void ServerManager::webServerConfigurationRemoved(const QString &id)
{
    if (!m_webServers.contains(id)) {
        qCWarning(dcServerManager()) << "Received a Web Server config removed event but don't have a Web Server instance for it.";
        return;
    }
    WebServer *server = m_webServers.take(id);
    unregisterZeroConfService(id, "http");
    server->stopServer();
    server->deleteLater();
}

void ServerManager::mqttServerConfigurationChanged(const QString &id)
{
    ServerConfiguration config = NymeaCore::instance()->configuration()->mqttServerConfigurations().value(id);
    if (m_mqttBroker->isRunning(id)) {
        unregisterZeroConfService(id, "mqtt");
        m_mqttBroker->stopServer(id);
    }
    if (m_mqttBroker->startServer(config, m_sslConfiguration)) {
        registerZeroConfService(config, "mqtt", "_mqtt._tcp");
    }
}

void ServerManager::mqttServerConfigurationRemoved(const QString &id)
{
    unregisterZeroConfService(id, "mqtt");
    m_mqttBroker->stopServer(id);
}

void ServerManager::mqttPolicyChanged(const QString &clientId)
{
    m_mqttBroker->updatePolicy(NymeaCore::instance()->configuration()->mqttPolicies().value(clientId));
}

void ServerManager::mqttPolicyRemoved(const QString &clientId)
{
    m_mqttBroker->removePolicy(clientId);
}

void ServerManager::tunnelProxyServerConfigurationChanged(const QString &id)
{
    TunnelProxyServer *server = m_tunnelProxyServers.value(id);
    TunnelProxyServerConfiguration config = NymeaCore::instance()->configuration()->tunnelProxyServerConfigurations().value(id);
    if (server) {
        qCDebug(dcServerManager()) << "Restarting tunnel proxy connection using" << config;
        server->stopServer();
        server->setTunnelProxyConfig(config);
    } else {
        qCDebug(dcServerManager()) << "Received a tunnel proxy config change event but don't have a tunnel proxy server instance for it.";
        qCDebug(dcServerManager()) << "Creating tunnel proxy server using" << config;
        server = new TunnelProxyServer(m_nymeaConfiguration->serverName(), m_nymeaConfiguration->serverUuid(), config, this);
        m_tunnelProxyServers.insert(server->configuration().id, server);
        connect(server, &TunnelProxyServer::runningChanged, this, [this, server](bool running){
            if (running) {
                // Note: enable authentication in any case, we don't want to expose unprotected access trough the internet
                m_jsonServer->registerTransportInterface(server, true);
            } else {
                m_jsonServer->unregisterTransportInterface(server);
            }
        });
    }

    server->startServer();
}

void ServerManager::tunnelProxyServerConfigurationRemoved(const QString &id)
{
    if (!m_tunnelProxyServers.contains(id)) {
        qCWarning(dcServerManager()) << "Received a tunnel proxy config removed event but don't have a tunnel proxy connection for it.";
        return;
    }
    TunnelProxyServer *server = m_tunnelProxyServers.take(id);
    m_jsonServer->unregisterTransportInterface(server);
    server->stopServer();
    server->deleteLater();
}

void ServerManager::cloudEnabledChanged(bool enabled)
{
    qCDebug(dcServerManager()) << "Cloud connection" << (enabled ? "enabled. Starting tunnel proxy servers" : "disabled. Stopping tunnel proxy servers.");
    foreach (TunnelProxyServer *server, m_tunnelProxyServers) {
        if (enabled) {
            server->startServer();
        } else {
            server->stopServer();
        }
    }
}

bool ServerManager::registerZeroConfService(const ServerConfiguration &configuration, const QString &serverType, const QString &serviceType)
{
    // Note: reversed order
    QHash<QString, QString> txt;
    txt.insert("jsonrpcVersion", JSON_PROTOCOL_VERSION);
    txt.insert("serverVersion", NYMEA_VERSION_STRING);
    txt.insert("manufacturer", "nymea GmbH");
    txt.insert("uuid", NymeaCore::instance()->configuration()->serverUuid().toString());
    txt.insert("name", NymeaCore::instance()->configuration()->serverName());
    txt.insert("sslEnabled", configuration.sslEnabled ? "true" : "false");
    QString name = "nymea-" + serverType + "-" + configuration.id;
    if (!m_platform->zeroConfController()->servicePublisher()->registerService(name, QHostAddress(configuration.address), static_cast<quint16>(configuration.port), serviceType, txt)) {
        qCWarning(dcServerManager()) << "Could not register ZeroConf service for" << configuration;
        return false;
    }
    return true;
}

void ServerManager::unregisterZeroConfService(const QString &configId, const QString &serverType)
{
    m_platform->zeroConfController()->servicePublisher()->unregisterService("nymea-" + serverType + "-" + configId);
}

bool ServerManager::loadCertificate(const QString &certificateKeyFileName, const QString &certificateFileName)
{
    QFile certificateKeyFile(certificateKeyFileName);
    if (!certificateKeyFile.exists()) {
        qCWarning(dcServerManager()) << "Could not load certificate key file" << certificateKeyFile.fileName() << "because the file does not exist.";
        return false;
    }

    if (!certificateKeyFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcServerManager()) << "Could not open" << certificateKeyFile.fileName() << ":" << certificateKeyFile.errorString();
        return false;
    }

    m_certificateKey = QSslKey(&certificateKeyFile, QSsl::Rsa);
    if (m_certificateKey.isNull()) {
        qCWarning(dcServerManager()) << "SSL certificate key" << certificateFileName << "is not valid.";
        return false;
    }

    qCDebug(dcServerManager()) << "Loaded private certificate key" << certificateKeyFileName;
    certificateKeyFile.close();

    QFile certificateFile(certificateFileName);
    if (!certificateFile.exists()) {
        qCWarning(dcServerManager()) << "Could not load certificate file" << certificateFile.fileName() << "because the file does not exist.";
        return false;
    }

    if (!certificateFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcServerManager()) << "Could not open" << certificateFile.fileName() << ":" << certificateFile.errorString();
        return false;
    }

    m_certificate = QSslCertificate(&certificateFile);
    if (m_certificate.isNull()) {
        qCWarning(dcServerManager()) << "SSL certificate" << certificateFileName << "is not valid.";;
        return false;
    }

    qCDebug(dcServerManager()) << "Loaded certificate file" << certificateFileName;
    certificateFile.close();

    return true;
}

/*! Set the server name for all servers to the given \a serverName. */
void ServerManager::setServerName(const QString &serverName)
{
    qCDebug(dcServerManager()) << "Server name changed" << serverName;

    foreach (WebSocketServer *websocketServer, m_webSocketServers.values()) {
        unregisterZeroConfService(websocketServer->configuration().id, "ws");
        websocketServer->setServerName(serverName);
        registerZeroConfService(websocketServer->configuration(), "ws", "_ws._tcp");
    }

    foreach (WebServer *webServer, m_webServers.values()) {
        unregisterZeroConfService(webServer->configuration().id, "http");
        webServer->setServerName(serverName);
        registerZeroConfService(webServer->configuration(), "http", "_http._tcp");
    }

    foreach (TcpServer *tcpServer, m_tcpServers.values()) {
        unregisterZeroConfService(tcpServer->configuration().id, "tcp");
        tcpServer->setServerName(serverName);
        registerZeroConfService(tcpServer->configuration(), "tcp", "_jsonrpc._tcp");
    }

    foreach (const ServerConfiguration &config, m_mqttBroker->configurations()) {
        unregisterZeroConfService(config.id, "mqtt");
        registerZeroConfService(config, "mqtt", "_mqtt._tcp");
    }
}

}
