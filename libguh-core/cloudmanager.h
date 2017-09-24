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

#ifndef CLOUDMANAGER_H
#define CLOUDMANAGER_H

#include "awsconnector.h"
#include "janusconnector.h"

#include <QObject>
#include <QTimer>
#include <QNetworkSession>
#include <QUuid>

class CloudManager : public QObject
{
    Q_OBJECT
public:
    explicit CloudManager(QObject *parent = nullptr);

    void setServerUrl(const QString &serverUrl);
    void setDeviceId(const QString &deviceId);
    void setClientCertificates(const QString &caCertificate, const QString &clientCertificate, const QString &clientCertificateKey);

    bool enabled() const;
    void setEnabled(bool enabled);

    void pairDevice(const QString &idToken, const QString &authToken, const QString &cognitoId);

signals:
    void pairingReply(QString cognitoUserId, int status);

private:
    void connect2aws();

private slots:
    void onlineStateChanged();
    void onPairingFinished(const QString &cognitoUserId, int errorCode);
    void onAWSWebRtcHandshakeMessageReceived(const QString &transactionId, const QVariantMap &data);
    void onJanusWebRtcHandshakeMessageReceived(const QString &transactionId, const QVariantMap &data);

private:
    QNetworkSession *m_networkSession;
    QTimer m_reconnectTimer;
    bool m_enabled = false;
    AWSConnector *m_awsConnector = nullptr;
    JanusConnector *m_janusConnector = nullptr;

    QString m_serverUrl;
    QUuid m_deviceId;
    QString m_caCertificate;
    QString m_clientCertificate;
    QString m_clientCertificateKey;
};

#endif // CLOUDMANAGER_H
