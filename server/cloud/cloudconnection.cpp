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

#include <QJsonDocument>

namespace guhserver {

CloudConnection::CloudConnection(QObject *parent) :
    QObject(parent),
    m_error(CloudConnectionErrorNoError),
    m_connected(false),
    m_authenticated(false)
{
    m_proxyUrl = QUrl("ws://127.0.0.1:1212");
    m_keystoneUrl = QUrl("http://localhost:8000/oauth2/token");

    m_reconnectionTimer = new QTimer(this);
    m_reconnectionTimer->setSingleShot(false);
    connect(m_reconnectionTimer, &QTimer::timeout, this, &CloudConnection::reconnectionTimeout);

    m_connection = new QWebSocket("guhd", QWebSocketProtocol::Version13, this);
    connect(m_connection, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_connection, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(m_connection, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
    connect(m_connection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    m_authenticator = new CloudAuthenticator("6ac82de6a2ba454394f9022b6a733885", "d63eece1b725419f80961a9b1c49f8d4", this);
    m_authenticator->setUrl(m_authenticationServerUrl);

    connect(m_authenticator, &CloudAuthenticator::authenticationChanged, this, &CloudConnection::onAuthenticationChanged);
}

void CloudConnection::connectToCloud()
{
    if (m_connection->state() == QAbstractSocket::ConnectedState) {
        disconnectFromCloud();
    }

    m_authenticator->startAuthentication();
}

void CloudConnection::disconnectFromCloud()
{
    m_authenticator->stopAuthentication();
    m_connection->close(QWebSocketProtocol::CloseCodeNormal, "Disconnecting");
}

CloudAuthenticator *CloudConnection::authenticator() const
{
    return m_authenticator;
}

CloudConnection::CloudConnectionError CloudConnection::error() const
{
    return m_error;
}

bool CloudConnection::connected() const
{
    return m_connected;
}

bool CloudConnection::authenticated() const
{
    return m_authenticated;
}

void CloudConnection::sendData(const QByteArray &data)
{
    m_connection->sendTextMessage(data);
}

void CloudConnection::setConnected(const bool &connected)
{
    m_connected = connected;
    emit connectedChanged();
}

void CloudConnection::setAuthenticated(const bool &authenticated)
{
    m_authenticated = authenticated;
    emit authenticatedChanged();
}

void CloudConnection::onAuthenticationChanged()
{
    setAuthenticated(m_authenticator->authenticated());

    if (m_authenticated) {
        qCDebug(dcCloud()) << "Connecting to" << m_proxyUrl.toString();
        m_connection->open(m_proxyUrl);
    } else {
        m_error = m_authenticator->error();
    }
    emit authenticatedChanged();
}

void CloudConnection::onConnected()
{
    qCDebug(dcCloud()) << "Connected to cloud proxy server" << m_proxyUrl.toString();
    setConnected(true);
    m_reconnectionTimer->stop();
}

void CloudConnection::onDisconnected()
{
    if (!m_reconnectionTimer->isActive())
        qCDebug(dcCloud()) << "Disconnected from cloud:" << m_connection->closeReason();

    setConnected(false);
    m_reconnectionTimer->start(10000);
}

void CloudConnection::onError(const QAbstractSocket::SocketError &error)
{
    if (!m_reconnectionTimer->isActive())
        qCWarning(dcCloud()) << "Websocket error:" << error << m_connection->errorString();
    m_reconnectionTimer->start(10000);
}

void CloudConnection::onTextMessageReceived(const QString &message)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcCloud()) << "Could not parse json data from guh" << message.toUtf8() << error.errorString();
        return;
    }

    emit dataReceived(jsonDoc.toVariant().toMap());
}

void CloudConnection::onError(const QAbstractSocket::SocketError &error)
{
    if (!m_reconnectionTimer->isActive())
        qCWarning(dcCloud()) << "Websocket error:" << error << m_connection->errorString();

    m_error = Cloud::CloudErrorProxyServerNotReachable;
    m_reconnectionTimer->start(10000);
    setConnected(false);
}

void CloudConnection::reconnectionTimeout()
{
    if (m_authenticated) {
        m_connection->open(m_proxyUrl);
    } else {
        m_reconnectionTimer->stop();
        m_error = CloudConnectionErrorAuthenticationFailed;
    }
}


}
