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
#include <QObject>
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

    void connectToCloud(const QString &username, const QString &password);

    CloudConnectionError error() const;

    void enable();
    void disable();

    bool enabled() const;
    bool connected() const;
    bool active() const;
    bool authenticated() const;

private:
    QWebSocket *m_connection;
    CloudAuthenticator *m_authenticator;
    CloudConnectionError m_error;

    QUrl m_proxyUrl;
    QUrl m_keystoneUrl;

    bool m_enabled;
    bool m_connected;
    bool m_active;
    bool m_authenticated;

    void setEnabled(const bool &enabled);
    void setConnected(const bool &connected);
    void setActive(const bool &active);
    void setAuthenticated(const bool &authenticated);

signals:
    void enabledChanged();
    void connectedChanged();
    void activeChanged();
    void authenticatedChanged();

private slots:
    void onAuthenticationChanged();
    void onConnected();
    void onDisconnected();
    void onError(const QAbstractSocket::SocketError &error);
    void onTextMessageReceived(const QString &message);

};

}
#endif // CLOUDCONNECTION_H
