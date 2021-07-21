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

#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QObject>

#include "loggingcategories.h"
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
class MqttBroker;
#ifdef WITH_BLUETOOTH
class BluetoothServer;
#endif // WITH_BLUETOOTH

class MockTcpServer;

class ServerManager : public QObject
{
    Q_OBJECT
public:
    explicit ServerManager(Platform *platform, NymeaConfiguration *configuration, const QStringList &additionalInterfaces = QStringList(), QObject *parent = nullptr);

    // Interfaces
    JsonRPCServerImplementation *jsonServer() const;

#ifdef WITH_BLUETOOTH
    BluetoothServer* bluetoothServer() const;
#endif // WITH_BLUETOOTH

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
    bool registerZeroConfService(const ServerConfiguration &configuration, const QString &serverType, const QString &serviceType);
    void unregisterZeroConfService(const QString &configId, const QString &serverType);

private:
    Platform *m_platform = nullptr;

    // Interfaces
    JsonRPCServerImplementation *m_jsonServer;

#ifdef WITH_BLUETOOTH
    BluetoothServer *m_bluetoothServer;
#endif // WITH_BLUETOOTH
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
