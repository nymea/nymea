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

#ifndef TESTING_ENABLED
#include "tcpserver.h"
#else
#include "mocktcpserver.h"
#endif

#include "openssl/ssl.h"

class QSslConfiguration;
class QSslCertificate;
class QSslKey;

namespace guhserver {

class ServerManager : public QObject
{
    Q_OBJECT
public:
    explicit ServerManager(GuhConfiguration *configuration, QObject *parent = 0);

    // Interfaces
    JsonRPCServer *jsonServer() const;
    RestServer *restServer() const;

    // Transports
    WebServer* webServer() const;
    WebSocketServer* webSocketServer() const;
    BluetoothServer* bluetoothServer() const;

#ifdef TESTING_ENABLED
    MockTcpServer *tcpServer() const;
#else
    TcpServer *tcpServer() const;
#endif

private:
    // Interfaces
    JsonRPCServer *m_jsonServer;
    RestServer *m_restServer;

    // Transports
#ifdef TESTING_ENABLED
    MockTcpServer *m_tcpServer;
#else
    TcpServer *m_tcpServer;
#endif
    WebSocketServer *m_webSocketServer;
    WebServer *m_webServer;
    BluetoothServer *m_bluetoothServer;

    // Encrytption and stuff
    QSslConfiguration m_sslConfiguration;
    QSslKey m_certificateKey;
    QSslCertificate m_certificate;

    bool loadCertificate(const QString &certificateKeyFileName, const QString &certificateFileName);
};


class CertificateGenerator
{
public:
    static void generate(const QString &certificateFilename, const QString &keyFilename);
};
}

#endif // SERVERMANAGER_H
