/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QObject>

#include "loggingcategories.h"
#include "jsonrpc/jsonrpcserver.h"
#include "rest/restserver.h"
#include "websocketserver.h"
#include "bluetoothserver.h"
#include "tcpserver.h"
#include "mocktcpserver.h"

class QSslConfiguration;
class QSslCertificate;
class QSslKey;

namespace nymeaserver {

class ServerManager : public QObject
{
    Q_OBJECT
public:
    explicit ServerManager(NymeaConfiguration *configuration, QObject *parent = 0);

    // Interfaces
    JsonRPCServer *jsonServer() const;
    RestServer *restServer() const;

    BluetoothServer* bluetoothServer() const;

    MockTcpServer *mockTcpServer() const;

private slots:
    void tcpServerConfigurationChanged(const QString &id);
    void tcpServerConfigurationRemoved(const QString &id);
    void webSocketServerConfigurationChanged(const QString &id);
    void webSocketServerConfigurationRemoved(const QString &id);
    void webServerConfigurationChanged(const QString &id);
    void webServerConfigurationRemoved(const QString &id);

private:
    // Interfaces
    JsonRPCServer *m_jsonServer;
    RestServer *m_restServer;

    BluetoothServer *m_bluetoothServer;
    QHash<QString, TcpServer*> m_tcpServers;
    QHash<QString, WebSocketServer*> m_webSocketServers;
    QHash<QString, WebServer*> m_webServers;
    MockTcpServer *m_mockTcpServer;

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
