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

class AWSConnector;
class CloudNotifications;
namespace remoteproxyclient {
class RemoteProxyConnection;
}

namespace nymeaserver {

class NymeaConfiguration;
class CloudTransport;
class CloudManager : public QObject
{
    Q_OBJECT
public:
    enum CloudConnectionState {
        CloudConnectionStateDisabled,
        CloudConnectionStateUnconfigured,
        CloudConnectionStateConnecting,
        CloudConnectionStateConnected
    };
    Q_ENUM(CloudConnectionState)

    explicit CloudManager(NymeaConfiguration *configuration, NetworkManager *networkManager, QObject *parent = nullptr);
    ~CloudManager();

    bool enabled() const;
    void setEnabled(bool enabled);

    bool installClientCertificates(const QByteArray &rootCA, const QByteArray &certificatePEM, const QByteArray &publicKey, const QByteArray &privateKey, const QString &endpoint);

    CloudConnectionState connectionState() const;

    void pairDevice(const QString &idToken, const QString &userId);

    CloudNotifications* createNotificationsPlugin() const;
    CloudTransport* createTransportInterface() const;

signals:
    void connectionStateChanged();

    void pairingReply(QString cognitoUserId, int status, const QString &message);

private:
    void connect2aws();

private slots:
    void onlineStateChanged();
    void onPairingFinished(const QString &cognitoUserId, int errorCode, const QString &message);
    void awsConnected();
    void awsDisconnected();
    void setDeviceName(const QString &name);

private:
    QTimer m_reconnectTimer;
    bool m_enabled = false;
    AWSConnector *m_awsConnector = nullptr;
    NymeaConfiguration *m_configuration = nullptr;
    NetworkManager *m_networkManager = nullptr;
    CloudTransport *m_transport = nullptr;

    QString m_serverUrl;
    QUuid m_deviceId;
    QString m_deviceName;
    QString m_caCertificate;
    QString m_clientCertificate;
    QString m_clientCertificateKey;
};

}
#endif // CLOUDMANAGER_H
