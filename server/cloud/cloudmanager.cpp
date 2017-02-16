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

#include "cloudmanager.h"
#include "jsonhandler.h"
#include "guhcore.h"

#include <QJsonDocument>
#include <QJsonParseError>

namespace guhserver {

CloudManager::CloudManager(const bool &enabled, const QUrl &authenticationServerUrl, const QUrl &proxyServerUrl, QObject *parent) :
    TransportInterface(parent),
    m_enabled(enabled),
    m_active(false),
    m_authenticated(false),
    m_runningAuthentication(false)
{
    m_cloudConnection = new CloudConnection(authenticationServerUrl, proxyServerUrl, this);
    connect(m_cloudConnection, &CloudConnection::authenticatedChanged, this, &CloudManager::onAuthenticatedChanged);
    connect(m_cloudConnection, &CloudConnection::connectedChanged, this, &CloudManager::onConnectedChanged);

    m_interface = new CloudInterface(this);
    connect(m_cloudConnection, &CloudConnection::dataReceived, m_interface, &CloudInterface::dataReceived);
}

CloudManager::~CloudManager()
{
    qCDebug(dcApplication) << "Shutting down \"Cloud Manager\"";
    stopServer();
}

void CloudManager::connectToCloud(const QString &username, const QString &password)
{
    m_cloudConnection->authenticator()->setUsername(username);
    m_cloudConnection->authenticator()->setPassword(password);

    if (!m_cloudConnection->connectToCloud()) {
        m_runningAuthentication = false;
        emit authenticationFinished(m_cloudConnection->error());
        return;
    }

    m_runningAuthentication = true;
}

void CloudManager::sendData(const QUuid &clientId, const QVariantMap &data)
{
    if (m_tunnelClients.value(clientId).isNull())
        return;

    // Used from the JsonRpcServer
    m_interface->sendApiData(m_tunnelClients.value(clientId), data);
}

void CloudManager::sendData(const QList<QUuid> &clients, const QVariantMap &data)
{
    // Used from the JsonRpcServer
    foreach (const QUuid &clientId, clients) {
        sendData(clientId, data);
    }
}

bool CloudManager::enabled() const
{
    return m_enabled;
}

bool CloudManager::connected() const
{
    return m_cloudConnection->connected();
}

bool CloudManager::active() const
{
    return m_active;
}

bool CloudManager::authenticated() const
{
    return m_authenticated;
}

bool CloudManager::startServer()
{    
    if (m_enabled && !m_cloudConnection->connected())
        m_cloudConnection->connectToCloud();

    return true;
}

bool CloudManager::stopServer()
{
    m_cloudConnection->disconnectFromCloud();
    return true;
}

void CloudManager::onCloudEnabledChanged()
{
    if (GuhCore::instance()->configuration()->cloudEnabled()) {
        qCDebug(dcCloud()) << "Enabled.";
        setEnabled(true);
        startServer();
    } else {
        qCDebug(dcCloud()) << "Disabled.";
        setEnabled(false);
        stopServer();
    }
}

void CloudManager::onAuthenticationServerUrlChanged()
{
    //TODO: set URL in connection/authenticator
}

void CloudManager::onProxyServerUrlChanged()
{
    //TODO: set URL in connection and reconnect
}

void CloudManager::setEnabled(const bool &enabled)
{
    m_enabled = enabled;
    emit enabledChanged();
}

void CloudManager::setActive(const bool &active)
{
    m_active = active;
    emit activeChanged();
}

void CloudManager::setAuthenticated(const bool &authenticated)
{
    m_authenticated = authenticated;
    emit authenticatedChanged();
}

void CloudManager::sendCloudData(const QVariantMap &data)
{
    m_cloudConnection->sendData(QJsonDocument::fromVariant(data).toJson());
}

void CloudManager::onConnectionAuthentificationFinished(const bool &authenticated, const QUuid &connectionId)
{
    if (authenticated) {
        qCDebug(dcCloud()) << "Connection authenticated";
        setAuthenticated(true);
        m_connectionId = connectionId;
        qCDebug(dcCloud()) << "Connection id:" << connectionId.toString();

        if (m_runningAuthentication) {
            m_runningAuthentication = false;
            emit authenticationFinished(Cloud::CloudErrorNoError);
        }

    } else {
        qCWarning(dcCloud()) << "Connection not authorized.";
        setAuthenticated(false);

        if (m_runningAuthentication) {
            m_runningAuthentication = false;
            emit authenticationFinished(m_cloudConnection->error());
        }
    }
}

void CloudManager::onTunnelAdded(const QUuid &tunnelId, const QUuid &serverId, const QUuid &clientId)
{
    if (serverId == m_connectionId) {
        qCDebug(dcCloud()) << "New tunnel connection from" << clientId.toString();
        m_tunnelClients.insert(clientId, tunnelId);
        emit clientConnected(clientId);
        setActive(true);
    }
}

void CloudManager::onTunnelRemoved(const QUuid &tunnelId)
{
    if (m_tunnelClients.values().contains(tunnelId)) {
        QUuid clientId = m_tunnelClients.key(tunnelId);
        qCDebug(dcCloud()) << "Tunnel connection from" << clientId.toString() << "removed.";
        m_tunnelClients.remove(clientId);
        emit clientDisconnected(clientId);
        if (m_tunnelClients.isEmpty()) {
            qCDebug(dcCloud()) << "Remote connection inactive.";
            setActive(false);
        }
    }
}

void CloudManager::onCloudDataReceived(const QUuid &tunnelId, const QVariantMap &data)
{
    if (m_tunnelClients.values().contains(tunnelId)) {
        QUuid clientId = m_tunnelClients.key(tunnelId);

        bool success;
        int commandId = data.value("id").toInt(&success);
        if (!success) {
            qCWarning(dcCloud()) << "TunnelData: Error parsing command. Missing \"id\":" << data;
            sendErrorResponse(clientId, commandId, "Error parsing command. Missing 'id'");
            return;
        }

        QStringList commandList = data.value("method").toString().split('.');
        if (commandList.count() != 2) {
            qCWarning(dcCloud()) << "TunnelData: Error parsing method.\nGot:" << data.value("method").toString() << "\nExpected: \"Namespace.method\"";
            return;
        }

        QString targetNamespace = commandList.first();
        QString method = commandList.last();

        JsonHandler *handler = GuhCore::instance()->jsonRPCServer()->handlers().value(targetNamespace);
        if (!handler) {
            sendErrorResponse(clientId, commandId, "No such namespace");
            return;
        }

        if (!handler->hasMethod(method)) {
            sendErrorResponse(clientId, commandId, "No such method");
            return;
        }

        emit dataAvailable(clientId, targetNamespace, method, data);
    }
}

void CloudManager::onConnectedChanged()
{
    // Start authentication if connected
    if (m_cloudConnection->connected()) {
        m_interface->authenticateConnection(m_cloudConnection->authenticator()->token());
    } else {
        qCDebug(dcCloud()) << "Disconnected";
        // Reset information
        setAuthenticated(false);
        setActive(false);
        m_connectionId = QUuid();

        if (m_runningAuthentication) {
            m_runningAuthentication = false;
            emit authenticationFinished(m_cloudConnection->error());
        }

        // Clean up all tunnels
        foreach (const QUuid &clientId, m_tunnelClients.keys()) {
            emit clientDisconnected(clientId);
        }
        m_tunnelClients.clear();

        // Delete all replies
        qDeleteAll(m_replies.values());
        m_replies.clear();
    }

    emit connectedChanged();
}

void CloudManager::onAuthenticatedChanged()
{
    if (m_cloudConnection->authenticator()->authenticated()) {
        if (m_runningAuthentication) {
            m_runningAuthentication = false;
            emit authenticationFinished(Cloud::CloudErrorNoError);
        }
        setAuthenticated(true);
    } else {
        if (m_runningAuthentication) {
            m_runningAuthentication = false;
            emit authenticationFinished(m_cloudConnection->error());
        }
        setAuthenticated(false);
    }
}

}
