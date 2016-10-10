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

CloudConnection::CloudConnection(const QUrl &authenticationServer, const QUrl &proxyServer, QObject *parent) :
    QObject(parent),
    m_authenticationServerUrl(authenticationServer),
    m_proxyServerUrl(proxyServer),
    m_connected(false),
    m_error(Cloud::CloudErrorNoError)
{
    // If not connected, try to reconnect
    m_reconnectionTimer = new QTimer(this);
    m_reconnectionTimer->setSingleShot(false);
    m_reconnectionTimer->setInterval(10000);
    connect(m_reconnectionTimer, &QTimer::timeout, this, &CloudConnection::reconnectionTimeout);

    // Ping the server to make sure the connection is still alive
    m_pingTimer = new QTimer(this);
    m_pingTimer->setSingleShot(false);
    m_pingTimer->setInterval(30000);
    connect(m_pingTimer, &QTimer::timeout, this, &CloudConnection::onPingTimeout);

    // Timer to check if ping response was received (if not, reconnect the socket)
    m_pingResponseTimer = new QTimer(this);
    m_pingResponseTimer->setSingleShot(true);
    m_pingResponseTimer->setInterval(5000);
    connect(m_pingResponseTimer, &QTimer::timeout, this, &CloudConnection::onPongTimeout);


    m_connection = new QWebSocket("guhd", QWebSocketProtocol::Version13, this);
    connect(m_connection, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_connection, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(m_connection, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
    connect(m_connection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(m_connection, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));
    connect(m_connection, SIGNAL(pong(quint64,QByteArray)), this, SLOT(onPong(quint64,QByteArray)));

    m_authenticator = new CloudAuthenticator("6ac82de6a2ba454394f9022b6a733885", "d63eece1b725419f80961a9b1c49f8d4", this);
    m_authenticator->setUrl(m_authenticationServerUrl);

    connect(m_authenticator, &CloudAuthenticator::authenticationChanged, this, &CloudConnection::onAuthenticationChanged);
}

bool CloudConnection::connectToCloud()
{
    if (m_connected)
        disconnectFromCloud();

    return m_authenticator->startAuthentication();
}

void CloudConnection::disconnectFromCloud()
{
    m_authenticator->stopAuthentication();
    m_connection->close(QWebSocketProtocol::CloseCodeNormal, "Cloud connection disabled.");
}

void CloudConnection::sendData(const QByteArray &data)
{
    m_connection->sendTextMessage(data);
}

CloudAuthenticator *CloudConnection::authenticator() const
{
    return m_authenticator;
}

bool CloudConnection::connected() const
{
    return m_connected;
}

Cloud::CloudError CloudConnection::error() const
{
    return m_error;
}

void CloudConnection::setConnected(const bool &connected)
{
    if (m_connected != connected) {
        m_connected = connected;
        emit connectedChanged();
    }
}

void CloudConnection::onAuthenticationChanged()
{
    if (authenticator()->authenticated()) {
        qCDebug(dcCloud()) << "Connecting to" << m_proxyServerUrl.toString();
        m_error = Cloud::CloudErrorNoError;
        m_connection->open(m_proxyServerUrl);
    } else {
        m_error = m_authenticator->error();
    }
    emit authenticatedChanged();
}

void CloudConnection::onConnected()
{
    qCDebug(dcCloud()) << "Connected to cloud proxy server" << m_proxyServerUrl.toString();
    m_error = Cloud::CloudErrorNoError;
    setConnected(true);
    m_pingTimer->start();
    m_reconnectionTimer->stop();
}

void CloudConnection::onDisconnected()
{
    if (!m_reconnectionTimer->isActive())
        qCDebug(dcCloud()) << "Disconnected from cloud:" << m_connection->closeReason();

    m_error = Cloud::CloudErrorProxyServerNotReachable;
    setConnected(false);
    m_pingTimer->stop();
    m_pingResponseTimer->stop();
    m_reconnectionTimer->start();
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

    m_connection->close();
    setConnected(false);
    m_error = Cloud::CloudErrorProxyServerNotReachable;
    m_pingTimer->stop();
    m_reconnectionTimer->start();
}

void CloudConnection::onPingTimeout()
{
    if (!connected())
        return;

    m_connection->ping("Ping");
    m_pingResponseTimer->start();
}

void CloudConnection::onPong(const quint64 elapsedTime, const QByteArray &payload)
{
    Q_UNUSED(elapsedTime);
    Q_UNUSED(payload);
    m_pingResponseTimer->stop();
}

void CloudConnection::onPongTimeout()
{
    qCWarning(dcCloud()) << "Pong timeout: did not get a ping response from the server (after 5s): reconnecting to the server...";
    disconnectFromCloud();
    connectToCloud();
}

void CloudConnection::onStateChanged(const QAbstractSocket::SocketState &state)
{
    qCDebug(dcCloud()) << "Socket:" << state;
}

void CloudConnection::reconnectionTimeout()
{
    if (authenticator()->authenticated()) {
        m_connection->open(m_proxyServerUrl);
    } else {
        m_authenticator->startAuthentication();
        m_reconnectionTimer->stop();
    }
}

}
