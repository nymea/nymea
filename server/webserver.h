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

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QDir>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>

class HttpRequest;
class HttpReply;

// Note: Hypertext Transfer Protocol (HTTP/1.1) from the Internet Engineering Task Force (IETF):
//       https://tools.ietf.org/html/rfc7231

namespace guhserver {

class WebServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = 0);
    ~WebServer();

    void sendData(const QUuid &clientId, const QVariantMap &data);
    void sendData(const QList<QUuid> &clients, const QVariantMap &data);
    void sendHttpReply(HttpReply *reply);

private:
    QHash<QUuid, QSslSocket *> m_clientList;
    QHash<QSslSocket *, HttpRequest> m_incompleteRequests;

    bool m_enabled;
    bool m_useSsl;
    qint16 m_port;
    QDir m_webinterfaceDir;
    QSslCertificate m_certificate;
    QSslKey m_certificateKey;

    bool verifyFile(QSslSocket *socket, const QString &fileName);
    QString fileName(const QString &query);

    bool loadCertificate(const QString &keyFileName, const QString &certificateFileName);

    void writeData(QSslSocket *socket, const QByteArray &data);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void httpRequestReady(const QUuid &clientId, const HttpRequest &httpRequest);
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);
    void dataAvailable(const QUuid &clientId, const QString &targetNamespace, const QString &method, const QVariantMap &message);

private slots:
    void readClient();
    void onDisconnected();
    void onEncrypted();
    void onError(QAbstractSocket::SocketError error);

public slots:
    bool startServer();
    bool stopServer();

};

}

#endif // WEBSERVER_H
