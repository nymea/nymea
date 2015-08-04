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

/*!
    \class guhserver::ServerManager
    \brief This class represents the manager of all server interfaces of the guh server.

    \ingroup core
    \inmodule server

    The \l{ServerManager} starts the \l{JsonRPCServer} and the \l{RestServer}. He also loads
    and provides the SSL configurations for the secure \l{WebServer} and \l{WebSocketServer}
    connection.

    \sa JsonRPCServer, RestServer
*/

#include "servermanager.h"
#include "guhsettings.h"

#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>

namespace guhserver {

/*! Constructs a \l{ServerManager} with the given \a parent. */
ServerManager::ServerManager(QObject *parent) :
    QObject(parent),
    m_sslConfiguration(QSslConfiguration())
{
    // check SSL
    if (!QSslSocket::supportsSsl()) {
        qCWarning(dcConnection) << "SSL is not supported/installed on this platform.";
    } else {
        qCDebug(dcConnection) << "SSL library version:" << QSslSocket::sslLibraryVersionString();

        // load SSL configuration settings
        GuhSettings settings(GuhSettings::SettingsRoleGlobal);
        qCDebug(dcConnection) << "Loading SSL-configuration from" << settings.fileName();

        settings.beginGroup("SSL-Configuration");
        QString certificateFileName = settings.value("certificate", QVariant("/etc/ssl/certs/guhd-certificate.crt")).toString();
        QString keyFileName = settings.value("certificate-key", QVariant("/etc/ssl/private/guhd-certificate.key")).toString();
        settings.endGroup();

        if (!loadCertificate(keyFileName, certificateFileName)) {
            qCWarning(dcConnection) << "SSL encryption disabled";
        } else {
            m_sslConfiguration.setProtocol(QSsl::TlsV1_2);
            m_sslConfiguration.setPrivateKey(m_certificateKey);
            m_sslConfiguration.setLocalCertificate(m_certificate);
        }
    }

    qCDebug(dcApplication) << "Starting JSON RPC Server";
    m_jsonServer = new JsonRPCServer(m_sslConfiguration, this);

    qCDebug(dcApplication) << "Starting REST Server";
    m_restServer = new RestServer(m_sslConfiguration, this);
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

bool ServerManager::loadCertificate(const QString &certificateKeyFileName, const QString &certificateFileName)
{
    QFile certificateKeyFile(certificateKeyFileName);
    if (!certificateKeyFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcWebServer) << "Could not open" << certificateKeyFile.fileName() << ":" << certificateKeyFile.errorString();
        return false;
    }
    m_certificateKey = QSslKey(certificateKeyFile.readAll(), QSsl::Rsa);
    qCDebug(dcWebServer) << "Loaded successfully private certificate key " << certificateKeyFileName;
    certificateKeyFile.close();

    QFile certificateFile(certificateFileName);
    if (!certificateFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcWebServer) << "Could not open" << certificateFile.fileName() << ":" << certificateFile.errorString();
        return false;
    }
    m_certificate = QSslCertificate(certificateFile.readAll());
    qCDebug(dcWebServer) << "Loaded successfully certificate file " << certificateFileName;
    certificateFile.close();

    return true;
}

}
