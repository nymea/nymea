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

        m_enabled = true;
        if (!m_awsConnector->isConnected() && m_networkManager->state() == NetworkManager::NetworkManagerStateConnectedGlobal) {
            connect2aws();
        }
    }
}

void CloudManager::pairDevice(const QString &idToken, const QString &authToken, const QString &cognitoId)
{
    m_awsConnector->pairDevice(idToken, authToken, cognitoId);
}

void CloudManager::connect2aws()
{
    m_awsConnector->connect2AWS(m_serverUrl,
//                                "1e10fb7e-d9d9-4145-88dd-2d3caf623c18",  // micha's test id (needs micha's test certs) - remove that before merging
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

void CloudManager::onPairingFinished(const QString &cognitoUserId, int errorCode)
{
    emit pairingReply(cognitoUserId, errorCode);
}

void CloudManager::onAWSWebRtcHandshakeMessageReceived(const QString &transactionId, const QVariantMap &data)
{
    m_janusConnector->sendWebRtcHandshakeMessage(transactionId, data);
}

void CloudManager::onJanusWebRtcHandshakeMessageReceived(const QString &transactionId, const QVariantMap &data)
{
    m_awsConnector->sendWebRtcHandshakeMessage(transactionId, data);
}
