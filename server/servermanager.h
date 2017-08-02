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
    static void generate(const QString &certificateFilename, const QString &keyFilename) {
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
        ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
        X509_gmtime_adj(X509_get_notBefore(x509), 0); // not before current time
        X509_gmtime_adj(X509_get_notAfter(x509), 31536000L); // not after a year from this point
        X509_set_pubkey(x509, pkey);
        name = X509_get_subject_name(x509);
        q_check_ptr(name);
        X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *)"AT", -1, -1, 0);
        X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char *)"Guh GmbH", -1, -1, 0);
        X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"guh core autocreated", -1, -1, 0);
        X509_set_issuer_name(x509, name);
        X509_sign(x509, pkey, EVP_sha1());
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
            qFatal("PEM_write_bio_PrivateKey");
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
};
}

#endif // SERVERMANAGER_H
