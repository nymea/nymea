/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef CLOUDMANAGER_H
#define CLOUDMANAGER_H

#include <QObject>
#include <QTimer>
#include <QNetworkSession>
#include <QUuid>

class NetworkManager;

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

    explicit CloudManager(NymeaConfiguration *configuration, QObject *parent = nullptr);
#ifdef WITH_DBUS
    explicit CloudManager(NymeaConfiguration *configuration, NetworkManager *networkManager, QObject *parent = nullptr);
#endif // WITH_DBUS
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
