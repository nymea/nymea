/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
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
#include "awsconnector.h"
#include "janusconnector.h"
#include "loggingcategories.h"

CloudManager::CloudManager(NetworkManager *networkManager, QObject *parent) : QObject(parent),
    m_networkManager(networkManager)
{
    m_awsConnector = new AWSConnector(this);
    connect(m_awsConnector, &AWSConnector::devicePaired, this, &CloudManager::onPairingFinished);
    connect(m_awsConnector, &AWSConnector::webRtcHandshakeMessageReceived, this, &CloudManager::onAWSWebRtcHandshakeMessageReceived);
    connect(m_awsConnector, &AWSConnector::connected, this, &CloudManager::awsConnected);
    connect(m_awsConnector, &AWSConnector::disconnected, this, &CloudManager::awsDisconnected);

    m_janusConnector = new JanusConnector(this);
    connect(m_janusConnector, &JanusConnector::webRtcHandshakeMessageReceived, this, &CloudManager::onJanusWebRtcHandshakeMessageReceived);

    connect(m_networkManager, &NetworkManager::stateChanged, this, &CloudManager::onlineStateChanged);
}

CloudManager::~CloudManager()
{
    qCDebug(dcApplication) << "Shutting down \"CloudManager\"";
}

void CloudManager::setServerUrl(const QString &serverUrl)
{
    m_serverUrl = serverUrl;
}

void CloudManager::setDeviceId(const QUuid &deviceId)
{
    m_deviceId = deviceId;
}

void CloudManager::setDeviceName(const QString &name)
{
    m_deviceName = name;
}

void CloudManager::setClientCertificates(const QString &caCertificate, const QString &clientCertificate, const QString &clientCertificateKey)
{
    m_caCertificate = caCertificate;
    m_clientCertificate = clientCertificate;
    m_clientCertificateKey = clientCertificateKey;
}

bool CloudManager::enabled() const
{
    return m_enabled;
}

void CloudManager::setEnabled(bool enabled)
{
    if (enabled) {
        bool missingConfig = false;
        if (m_deviceId.isNull()) {
            qCWarning(dcCloud()) << "Don't have a unique device ID.";
            missingConfig = true;
        }
        if (m_deviceName.isEmpty()) {
            qCWarning(dcCloud()) << "Don't have a device name set";
            missingConfig = true;
        }
        if (m_serverUrl.isEmpty()) {
            qCWarning(dcCloud()) << "Cloud server URL not set.";
            missingConfig = true;
        }
        if (m_clientCertificate.isEmpty()) {
            qCWarning(dcCloud()) << "Cloud certificate not set.";
            missingConfig = true;
        }
        if (m_clientCertificateKey.isEmpty()) {
            qCWarning(dcCloud()) << "Cloud certificate key not set.";
            missingConfig = true;
        }
        if (m_caCertificate.isEmpty()) {
            qCWarning(dcCloud()) << "Cloud certificate CA not set.";
            missingConfig = true;
        }
        if (missingConfig) {
            qCWarning(dcCloud()) << "Cloud configuration incomplete. Not enabling cloud connection.";
            return;
        }

        qCDebug(dcCloud()) << "Enabling cloud connection.";
        m_enabled = true;
        if (!m_awsConnector->isConnected() && m_networkManager->state() == NetworkManager::NetworkManagerStateConnectedGlobal) {
            connect2aws();
        }
    } else {
        qCDebug(dcCloud()) << "Disabling cloud connection.";
        m_enabled = false;
        m_awsConnector->disconnectAWS();
    }
}

bool CloudManager::connected() const
{
    return m_awsConnector->isConnected();
}

void CloudManager::pairDevice(const QString &idToken, const QString &userId)
{
    m_awsConnector->pairDevice(idToken, userId);
}

bool CloudManager::keepAlive(const QString &sessionId)
{
    return m_janusConnector->sendKeepAliveMessage(sessionId);
}

void CloudManager::connect2aws()
{
    m_awsConnector->connect2AWS(m_serverUrl,
                                m_deviceId.toString().remove(QRegExp("[{}]*")),
                                m_deviceName,
                                m_caCertificate,
                                m_clientCertificate,
                                m_clientCertificateKey
                                );
}

void CloudManager::onlineStateChanged()
{
    if (m_networkManager->state() == NetworkManager::NetworkManagerStateConnectedGlobal) {
        if (m_enabled && !m_awsConnector->isConnected()) {
            connect2aws();
        }
    }
}

void CloudManager::onPairingFinished(const QString &cognitoUserId, int errorCode, const QString &message)
{
    emit pairingReply(cognitoUserId, errorCode, message);
}

void CloudManager::onAWSWebRtcHandshakeMessageReceived(const QString &transactionId, const QVariantMap &data)
{
    m_janusConnector->sendWebRtcHandshakeMessage(transactionId, data);
}

void CloudManager::onJanusWebRtcHandshakeMessageReceived(const QString &transactionId, const QVariantMap &data)
{
    m_awsConnector->sendWebRtcHandshakeMessage(transactionId, data);
}

void CloudManager::awsConnected()
{
    emit connectedChanged(true);
}

void CloudManager::awsDisconnected()
{
    emit connectedChanged(false);
}
