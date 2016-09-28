/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#ifndef CLOUDCONNECTION_H
#define CLOUDCONNECTION_H

#include <QUrl>
#include <QUuid>
#include <QObject>
#include <QTimer>
#include <QWebSocket>

#include "cloud.h"
#include "cloudauthenticator.h"

namespace guhserver {

class CloudConnection : public QObject
{
    Q_OBJECT
public:
    explicit CloudConnection(const QUrl &authenticationServer, const QUrl &proxyServer, QObject *parent = 0);

    bool connectToCloud();
    void disconnectFromCloud();

    void sendData(const QByteArray &data);

    CloudAuthenticator *authenticator() const;

    bool connected() const;
    Cloud::CloudError error() const;

private:
    QWebSocket *m_connection;
    CloudAuthenticator *m_authenticator;

    QTimer *m_reconnectionTimer;
    QTimer *m_pingTimer;

    QUrl m_authenticationServerUrl;
    QUrl m_proxyServerUrl;

    bool m_connected;
    Cloud::CloudError m_error;

    void setConnected(const bool &connected);

signals:
    void dataReceived(const QVariantMap &data);
    void enabledChanged();
    void connectedChanged();
    void activeChanged();
    void authenticatedChanged();

private slots:
    void onAuthenticationChanged();
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(const QAbstractSocket::SocketError &error);
    void onStateChanged(const QAbstractSocket::SocketState &state);
    void onPingTimeout();
    void onPong(const quint64 elapsedTime, const QByteArray &payload);

    void reconnectionTimeout();

};

}

#endif // CLOUDCONNECTION_H
