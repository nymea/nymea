/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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
#include "network/avahi/qtavahiservice.h"

#include "loggingcategories.h"

namespace guhserver {

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
    QByteArray m_receiveBuffer;
};

class TcpServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit TcpServer(const ServerConfiguration &configuration, const QSslConfiguration &sslConfiguration, QObject *parent = 0);
    ~TcpServer();

    QUrl serverUrl() const;

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void sendData(const QList<QUuid> &clients, const QByteArray &data) override;

private:
    QTimer *m_timer;

    QtAvahiService *m_avahiService;

    SslServer * m_server;
    QHash<QUuid, QTcpSocket *> m_clientList;

    QSslConfiguration m_sslConfig;

private slots:
    void onClientConnected(QSslSocket *socket);
    void onClientDisconnected(QSslSocket *socket);
    void onDataAvailable(QSslSocket *socket, const QByteArray &data);
    void onError(QAbstractSocket::SocketError error);
    void onEncrypted();

    void onAvahiServiceStateChanged(const QtAvahiService::QtAvahiServiceState &state);


public slots:
    void resetAvahiService();
    void reconfigureServer(const ServerConfiguration &configuration);
    bool startServer() override;
    bool stopServer() override;
};

}

#endif // TCPSERVER_H
