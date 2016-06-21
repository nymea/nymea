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
    m_reconnectionTimer = new QTimer(this);
    m_reconnectionTimer->setSingleShot(false);
    connect(m_reconnectionTimer, &QTimer::timeout, this, &CloudConnection::reconnectionTimeout);

    m_connection = new QWebSocket("guhd", QWebSocketProtocol::Version13, this);
    connect(m_connection, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_connection, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(m_connection, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
    connect(m_connection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    m_authenticator = new CloudAuthenticator("0631d42ba0464e4ebd4b78b15c53f532", "b7919ebf3bcf48239f348e764744079b", this);
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
        qCWarning(dcCloud()) << "Could not authenticate";
        m_error = m_authenticator->error();
    }
}

void CloudConnection::onConnected()
{
    qCDebug(dcCloud()) << "Connected to cloud proxy server" << m_proxyServerUrl.toString();
    m_error = Cloud::CloudErrorNoError;
    setConnected(true);
    m_reconnectionTimer->stop();
}

void CloudConnection::onDisconnected()
{
    if (!m_reconnectionTimer->isActive())
        qCDebug(dcCloud()) << "Disconnected from cloud:" << m_connection->closeReason();

    m_error = Cloud::CloudErrorProxyServerNotReachable;
    setConnected(false);
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
}

void CloudConnection::reconnectionTimeout()
{
    if (authenticator()->authenticated()) {
        m_connection->open(m_proxyServerUrl);
    } else {
        m_reconnectionTimer->stop();
    }

}

}
