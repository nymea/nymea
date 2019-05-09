/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QObject>

#include "loggingcategories.h"
#include "nymeaconfiguration.h"

#include <QSslConfiguration>
#include <QSslKey>


namespace nymeaserver {

class NymeaConfiguration;
class JsonRPCServer;
class TcpServer;
class WebSocketServer;
class WebServer;
class BluetoothServer;
class RestServer;
class MqttBroker;

class MockTcpServer;

class ServerManager : public QObject
{
    Q_OBJECT
public:
    explicit ServerManager(NymeaConfiguration *configuration, QObject *parent = nullptr);

    // Interfaces
    JsonRPCServer *jsonServer() const;
    RestServer *restServer() const;

    BluetoothServer* bluetoothServer() const;

    MockTcpServer *mockTcpServer() const;

    MqttBroker *mqttBroker() const;

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

private:
    // Interfaces
    JsonRPCServer *m_jsonServer;
    RestServer *m_restServer;

    BluetoothServer *m_bluetoothServer;
    QHash<QString, TcpServer*> m_tcpServers;
    QHash<QString, WebSocketServer*> m_webSocketServers;
    QHash<QString, WebServer*> m_webServers;
    MockTcpServer *m_mockTcpServer;

    MqttBroker *m_mqttBroker;

    // Encrytption and stuff
    QSslConfiguration m_sslConfiguration;
    QSslKey m_certificateKey;
    QSslCertificate m_certificate;

    bool loadCertificate(const QString &certificateKeyFileName, const QString &certificateFileName);

public slots:
    void setServerName(const QString &serverName);

};

}

#endif // SERVERMANAGER_H
