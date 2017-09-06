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
    // check SSL
    if (!configuration->sslEnabled()) {
        qCDebug(dcConnection) << "SSL encryption disabled by config.";
    } else if (!QSslSocket::supportsSsl()) {
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
            m_sslConfiguration.setProtocol(QSsl::TlsV1_1OrLater);
            m_sslConfiguration.setPrivateKey(m_certificateKey);
            m_sslConfiguration.setLocalCertificate(m_certificate);
        }
    }

    // Interfaces
    m_jsonServer = new JsonRPCServer(m_sslConfiguration, this);
    m_restServer = new RestServer(m_sslConfiguration, this);


    // Transports
#ifdef TESTING_ENABLED
    m_tcpServer = new MockTcpServer(this);
#else
    m_tcpServer = new TcpServer(configuration->tcpServerAddress(), configuration->tcpServerPort(),  configuration->sslEnabled(), m_sslConfiguration, this);
#endif

    m_webSocketServer = new WebSocketServer(configuration->webSocketAddress(), configuration->webSocketPort(), configuration->sslEnabled(), m_sslConfiguration, this);

    m_bluetoothServer = new BluetoothServer(this);

    // Register transport interfaces for the JSON RPC server
    m_jsonServer->registerTransportInterface(m_tcpServer);
    m_jsonServer->registerTransportInterface(m_webSocketServer);
    m_jsonServer->registerTransportInterface(m_bluetoothServer, configuration->bluetoothServerEnabled());

    // Register transport itnerfaces for the Webserver
    m_webServer = new WebServer(configuration->webServerAddress(), configuration->webServerPort(), configuration->webServerPublicFolder(), configuration->sslEnabled(), m_sslConfiguration, this);
    m_restServer->registerWebserver(m_webServer);
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

WebServer *ServerManager::webServer() const
{
    return m_webServer;
}

WebSocketServer *ServerManager::webSocketServer() const
{
    return m_webSocketServer;
}

BluetoothServer *ServerManager::bluetoothServer() const
{
    return m_bluetoothServer;
}

#ifdef TESTING_ENABLED
MockTcpServer
#else
TcpServer
#endif
*ServerManager::tcpServer() const
{
    return m_tcpServer;
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
