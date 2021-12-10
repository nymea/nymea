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

#ifndef AWSCONNECTOR_H
#define AWSCONNECTOR_H

#include <QObject>
#include <QFuture>
#include <QDateTime>
#include <QTimer>

#include <mqttclient.h>

class AWSConnector : public QObject
{
    Q_OBJECT
public:
    explicit AWSConnector(QObject *parent = nullptr);
    ~AWSConnector();

    void connect2AWS(const QString &endpoint, const QString &clientId, const QString &clientName, const QString &caFile, const QString &clientCertFile, const QString &clientPrivKeyFile);
    void disconnectAWS();
    bool isConnected() const;

    void setDeviceName(const QString &deviceName);
    void pairDevice(const QString &idToken, const QString &userId);

signals:
    void connected();
    void disconnected();
    void devicePaired(const QString &cognritoUserId, int errorCode, const QString &message);

    void proxyConnectionRequestReceived(const QString &token, const QString &nonce, const QString &serverUrl);

private slots:
    void doConnect();
    void onConnected();
    void onDisconnected();
    void onPublished(quint16 msgid, const QString &topic);
    void onSubscribed(const QString &topic, Mqtt::SubscribeReturnCode returnCode);
    void onPublishReceived(const QString &topic, const QByteArray &payload);

    void registerDevice();
    void onDeviceRegistered(bool needsReconnect);
    void setupSubscriptions();
    void fetchPairings();
    void onPairingsRetrieved(const QVariantMap &pairings);
    void setName();


private:
    quint16 publish(const QString &topic, const QVariantMap &message);
    void subscribe(const QStringList &topics);

    void storeRegisteredFlag(bool registered);
    bool readRegisteredFlag() const;

    void storeSyncedNameCache(const QString &syncedName);
    QString readSyncedNameCache();

    QString getCertificateFingerprint(const QSslCertificate &certificate) const;

private:
    MqttClient *m_client = nullptr;
    QString m_currentEndpoint;
    QString m_caFile;
    QString m_clientCertFile;
    QString m_clientPrivKeyFile;

    QString m_clientId;
    QString m_clientName;
    QFuture<void> m_connectingFuture;
    bool m_isCleanSession = true;
    bool m_shouldReconnect = false;
    QTimer m_reconnectTimer;
    QTimer m_connectTimer;

    quint8 m_transactionId = 0;
    QString m_createDeviceId;
    int m_createDeviceSubscriptionId = 0;
    QHash<quint16, QString> m_pairingRequests;
    bool m_setupInProgress = false;
    int m_reconnectCounter = 0;
    QDateTime m_lastConnectionDrop;
    QStringList m_subscriptionCache;
    QPair<QVariantMap, QDateTime> m_cachedTURNCredentials;

};

#endif // AWSCONNECTOR_H
