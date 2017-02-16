/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "cloudauthenticator.h"

namespace guhserver {

class CloudConnection : public QObject
{
    Q_OBJECT
    Q_ENUMS(CloudConnectionError)

public:
    enum CloudConnectionError {
        CloudConnectionErrorNoError,
        CloudConnectionErrorAuthenticationFailed,
        CloudConnectionErrorCloudServerNotReachable
    };

    explicit CloudConnection(QObject *parent = 0);

    void connectToCloud();
    void disconnectFromCloud();

    CloudAuthenticator *authenticator() const;

    CloudConnectionError error() const;
    void enable();
    void disable();

    bool connected() const;
    bool authenticated() const;

    void sendData(const QByteArray &data);
    void sendRequest(const QVariantMap &request);

private:
    QWebSocket *m_connection;
    CloudAuthenticator *m_authenticator;
    CloudConnectionError m_error;

    QTimer *m_reconnectionTimer;
    QTimer *m_pingTimer;
    QTimer *m_pingResponseTimer;

    QUrl m_proxyUrl;
    QUrl m_keystoneUrl;

    bool m_connected;
    bool m_authenticated;

    void setConnected(const bool &connected);
    void setAuthenticated(const bool &authenticated);

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
    void onError(const QAbstractSocket::SocketError &error);
    void onStateChanged(const QAbstractSocket::SocketState &state);
    void onPingTimeout();
    void onPong(const quint64 elapsedTime, const QByteArray &payload);
    void onPongTimeout();

    void reconnectionTimeout();

};

}

#endif // CLOUDCONNECTION_H
