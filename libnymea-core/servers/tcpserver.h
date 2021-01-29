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

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QUuid>
#include <QTimer>
#include <QSslConfiguration>
#include <QDebug>

#include "transportinterface.h"

#include "loggingcategories.h"

namespace nymeaserver {

class SslServer: public QTcpServer
{
    Q_OBJECT
public:
    SslServer(bool sslEnabled, const QSslConfiguration &config, QObject *parent = nullptr):
        QTcpServer(parent),
        m_sslEnabled(sslEnabled),
        m_config(config)
    {

    }

signals:
    void clientConnected(QSslSocket *socket);
    void clientDisconnected(QSslSocket *socket);
    void dataAvailable(QSslSocket *socket, const QByteArray &data);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientDisconnected();
    void onSocketReadyRead();

private:
    bool m_sslEnabled = false;
    QSslConfiguration m_config;
};

class TcpServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit TcpServer(const ServerConfiguration &configuration, const QSslConfiguration &sslConfiguration, QObject *parent = nullptr);
    ~TcpServer() override;

    QUrl serverUrl() const;

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clients, const QByteArray &data) override;

    void terminateClientConnection(const QUuid &clientId) override;

private:
    QTimer *m_timer = nullptr;

    SslServer *m_server = nullptr;
    QHash<QUuid, QTcpSocket *> m_clientList;

    QSslConfiguration m_sslConfig;

private slots:
    void onClientConnected(QSslSocket *socket);
    void onClientDisconnected(QSslSocket *socket);
    void onDataAvailable(QSslSocket *socket, const QByteArray &data);
    void onError(QAbstractSocket::SocketError error);

public slots:
    void setServerName(const QString &serverName) override;
    bool startServer() override;
    bool stopServer() override;
};

}

#endif // TCPSERVER_H
