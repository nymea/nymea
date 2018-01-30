/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef CLOUDMANAGER_H
#define CLOUDMANAGER_H

#include <QObject>
#include <QTimer>
#include <QNetworkSession>
#include <QUuid>

#include "networkmanager/networkmanager.h"

class JanusConnector;
class AWSConnector;
class CloudNotifications;

class CloudManager : public QObject
{
    Q_OBJECT
public:
    explicit CloudManager(NetworkManager *networkManager, QObject *parent = nullptr);
    ~CloudManager();

    void setServerUrl(const QString &serverUrl);
    void setDeviceId(const QUuid &deviceId);
    void setDeviceName(const QString &name);
    void setClientCertificates(const QString &caCertificate, const QString &clientCertificate, const QString &clientCertificateKey);

    bool enabled() const;
    void setEnabled(bool enabled);
    bool connected() const;

    void pairDevice(const QString &idToken, const QString &userId);

    bool keepAlive(const QString &sessionId);

    CloudNotifications* createNotificationsPlugin() const;

signals:
    void connectedChanged(bool connected);

    void pairingReply(QString cognitoUserId, int status, const QString &message);

private:
    void connect2aws();

private slots:
    void onlineStateChanged();
    void onPairingFinished(const QString &cognitoUserId, int errorCode, const QString &message);
    void onAWSWebRtcHandshakeMessageReceived(const QString &transactionId, const QVariantMap &data);
    void onJanusWebRtcHandshakeMessageReceived(const QString &transactionId, const QVariantMap &data);
    void awsConnected();
    void awsDisconnected();

private:
    QTimer m_reconnectTimer;
    bool m_enabled = false;
    AWSConnector *m_awsConnector = nullptr;
    JanusConnector *m_janusConnector = nullptr;
    NetworkManager *m_networkManager = nullptr;

    QString m_serverUrl;
    QUuid m_deviceId;
    QString m_deviceName;
    QString m_caCertificate;
    QString m_clientCertificate;
    QString m_clientCertificateKey;

    CloudNotifications *m_notifications;
};

#endif // CLOUDMANAGER_H
