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

#include "cloudconnection.h"

#include "loggingcategories.h"
#include "guhsettings.h"

namespace guhserver {

CloudConnection::CloudConnection(QObject *parent) :
    QObject(parent),
    m_error(CloudConnectionErrorNoError),
    m_enabled(false),
    m_connected(false),
    m_active(false),
    m_authenticated(false)
{
    m_proxyUrl = QUrl("ws://127.0.0.1:1212");
    m_keystoneUrl = QUrl("http://localhost:8000/oauth2/token");

//    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
//    settings.beginGroup("CloudConnection");

    m_connection = new QWebSocket("guhd", QWebSocketProtocol::Version13, this);
    connect(m_connection, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_connection, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(m_connection, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
    connect(m_connection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    m_authenticator = new CloudAuthenticator("0631d42ba0464e4ebd4b78b15c53f532", "b7919ebf3bcf48239f348e764744079b", this);
    m_authenticator->setUrl(m_keystoneUrl);

    connect(m_authenticator, &CloudAuthenticator::authenticationChanged, this, &CloudConnection::onAuthenticationChanged);
}

void CloudConnection::connectToCloud(const QString &username, const QString &password)
{
    m_authenticator->setUsername(username);
    m_authenticator->setPassword(password);
    m_authenticator->startAuthentication();
}

CloudConnection::CloudConnectionError CloudConnection::error() const
{
    return m_error;
}

void CloudConnection::enable()
{
    m_enabled = true;
}

void CloudConnection::disable()
{
    m_enabled = false;
}

bool CloudConnection::enabled() const
{
    return m_enabled;
}

bool CloudConnection::connected() const
{
    return m_connected;
}

bool CloudConnection::active() const
{
    return m_active;
}

bool CloudConnection::authenticated() const
{
    return m_authenticated;
}

void CloudConnection::setEnabled(const bool &enabled)
{
    m_enabled = enabled;
    emit enabledChanged();
}

void CloudConnection::setConnected(const bool &connected)
{
    m_connected = connected;
    emit connectedChanged();
}

void CloudConnection::setActive(const bool &active)
{
    m_active = active;
    emit activeChanged();
}

void CloudConnection::setAuthenticated(const bool &authenticated)
{
    m_authenticated = authenticated;
    emit authenticatedChanged();
}

void CloudConnection::onAuthenticationChanged()
{
    qCDebug(dcCloud()) << "Authentication changed" << m_authenticator->authenticated();
    setAuthenticated(m_authenticator->authenticated());

    if (m_authenticated) {
        qCDebug(dcCloud()) << "Connecting to" << m_proxyUrl.toString();
        m_connection->open(m_proxyUrl);
    } else {
        m_error = CloudConnectionErrorAuthenticationFailed;
    }
}

void CloudConnection::onConnected()
{
    qCDebug(dcCloud()) << "Connected to cloud proxy server" << m_proxyUrl.toString();
    setConnected(true);

    // TODO: authenticate cloud connection
}

void CloudConnection::onDisconnected()
{
    qCDebug(dcCloud()) << "Disconnected from cloud connection:" << m_connection->closeReason();
    setConnected(false);
}

void CloudConnection::onError(const QAbstractSocket::SocketError &error)
{
    qCWarning(dcCloud()) << "Websocket error:" << error << m_connection->errorString();
}

void CloudConnection::onTextMessageReceived(const QString &message)
{
    qCDebug(dcCloud()) << "Cloud message received" << message;
}

}
