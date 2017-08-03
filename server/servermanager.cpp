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
            m_sslConfiguration.setProtocol(QSsl::TlsV1_2);
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
    m_tcpServer = new TcpServer(configuration->tcpServerAddress(), configuration->tcpServerPort(), this);
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

void CertificateGenerator::generate(const QString &certificateFilename, const QString &keyFilename)
{
    EVP_PKEY * pkey = nullptr;
    RSA * rsa = nullptr;
    X509 * x509 = nullptr;
    X509_NAME * name = nullptr;
    BIO * bp_public = nullptr, * bp_private = nullptr;
    const char * buffer = nullptr;
    long size;

    pkey = EVP_PKEY_new();
    q_check_ptr(pkey);
    rsa = RSA_generate_key(2048, RSA_F4, nullptr, nullptr);
    q_check_ptr(rsa);
    EVP_PKEY_assign_RSA(pkey, rsa);
    x509 = X509_new();
    q_check_ptr(x509);
    // Randomize serial number in case a previous one is stuck in a browser (Chromium
    // completely rejects reused serial numbers and doesn't even allow to bypass it by an exception)
    qsrand(QUuid::createUuid().toString().remove(QRegExp("[a-zA-Z{}-]")).left(5).toInt());
    ASN1_INTEGER_set(X509_get_serialNumber(x509), qrand());
    X509_gmtime_adj(X509_get_notBefore(x509), 0); // not before current time
    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L*10); // not after 10 years from this point
    X509_set_pubkey(x509, pkey);
    name = X509_get_subject_name(x509);
    q_check_ptr(name);
    X509_NAME_add_entry_by_txt(name, "E", MBSTRING_ASC, (unsigned char *)"guh.io", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"guh.io", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "OU", MBSTRING_ASC, (unsigned char *)"home", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char *)"guh.io", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "L", MBSTRING_ASC, (unsigned char *)"Vienna", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *)"AT", -1, -1, 0);
    X509_set_issuer_name(x509, name);
    X509_sign(x509, pkey, EVP_sha256());
    bp_private = BIO_new(BIO_s_mem());
    q_check_ptr(bp_private);
    if(PEM_write_bio_PrivateKey(bp_private, pkey, nullptr, nullptr, 0, nullptr, nullptr) != 1)
    {
        EVP_PKEY_free(pkey);
        X509_free(x509);
        BIO_free_all(bp_private);
        qFatal("PEM_write_bio_PrivateKey");
    }
    bp_public = BIO_new(BIO_s_mem());
    q_check_ptr(bp_public);
    if(PEM_write_bio_X509(bp_public, x509) != 1)
    {
        EVP_PKEY_free(pkey);
        X509_free(x509);
        BIO_free_all(bp_public);
        BIO_free_all(bp_private);
        qFatal("PEM_write_bio_X509");
    }
    size = BIO_get_mem_data(bp_public, &buffer);
    q_check_ptr(buffer);
    QFileInfo certFi(certificateFilename);
    QDir dir;
    QFile certfile(certificateFilename);
    if (!dir.mkpath(certFi.absolutePath()) || !certfile.open(QFile::WriteOnly | QFile::Truncate) || certfile.write(buffer, size) != size) {
        qWarning() << "Error writing certificate file" << certificateFilename;
    }
    certfile.close();

    size = BIO_get_mem_data(bp_private, &buffer);
    q_check_ptr(buffer);
    QFileInfo keyFi(keyFilename);
    QFile keyFile(keyFilename);
    if (!dir.mkpath(keyFi.absolutePath()) || !keyFile.open(QFile::WriteOnly | QFile::Truncate) || keyFile.write(buffer, size) != size) {
        qWarning() << "Error writing key file" << keyFilename;
    }
    keyFile.close();

    EVP_PKEY_free(pkey); // this will also free the rsa key
    X509_free(x509);
    BIO_free_all(bp_public);
    BIO_free_all(bp_private);

}

}
