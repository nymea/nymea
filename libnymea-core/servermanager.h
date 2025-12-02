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

#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QObject>

#include "nymeaconfiguration.h"

#include <QSslConfiguration>
#include <QSslKey>


namespace nymeaserver {

class Platform;
class NymeaConfiguration;
class JsonRPCServerImplementation;
class TcpServer;
class WebSocketServer;
class WebServer;
class BluetoothServer;
class MqttBroker;
class TunnelProxyServer;

class MockTcpServer;

class ServerManager : public QObject
{
    Q_OBJECT
public:
    explicit ServerManager(Platform *platform, NymeaConfiguration *configuration, const QStringList &additionalInterfaces = QStringList(), QObject *parent = nullptr);

    // Interfaces
    JsonRPCServerImplementation *jsonServer() const;

    BluetoothServer* bluetoothServer() const;

    MockTcpServer *mockTcpServer() const;

    MqttBroker *mqttBroker() const;

public slots:
    void setServerName(const QString &serverName);

private slots:
    void tcpServerConfigurationChanged(const QString &id);
    void tcpServerConfigurationRemoved(const QString &id);
    void webSocketServerConfigurationChanged(const QString &id);
    void webSocketServerConfigurationRemoved(const QString &id);
    void webServerConfigurationChanged(const QString &id);
    void webServerConfigurationRemoved(const QString &id);
    void mqttServerConfigurationChanged(const QString &id);
    void mqttServerConfigurationRemoved(const QString &id);
    void mqttPolicyChanged(const QString &clientId);
    void mqttPolicyRemoved(const QString &clientId);
    void tunnelProxyServerConfigurationChanged(const QString &id);
    void tunnelProxyServerConfigurationRemoved(const QString &id);

private:
    bool registerZeroConfService(const ServerConfiguration &configuration, const QString &serverType, const QString &serviceType);
    void unregisterZeroConfService(const QString &configId, const QString &serverType);

    bool loadCertificate(const QString &certificateKeyFileName, const QString &certificateFileName);

private:
    Platform *m_platform = nullptr;
    bool m_disableInsecureInterfaces = false;
    NymeaConfiguration *m_nymeaConfiguration = nullptr;

    // Interfaces
    JsonRPCServerImplementation *m_jsonServer;

    BluetoothServer *m_bluetoothServer;
    QHash<QString, TcpServer*> m_tcpServers;
    QHash<QString, WebSocketServer*> m_webSocketServers;
    QHash<QString, WebServer*> m_webServers;
    QHash<QString, TunnelProxyServer *> m_tunnelProxyServers;
    MockTcpServer *m_mockTcpServer;

    MqttBroker *m_mqttBroker;

    // Encrytption and stuff
    QSslConfiguration m_sslConfiguration;
    QSslKey m_certificateKey;
    QSslCertificate m_certificate;
};

}

#endif // SERVERMANAGER_H
