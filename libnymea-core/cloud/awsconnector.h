/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017-2018 Michael Zanetti <michael.zanetti@guh.io>       *
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

#ifndef AWSCONNECTOR_H
#define AWSCONNECTOR_H

#include <QObject>
#include <QFuture>
#include <QDateTime>

#include <nymea-mqtt/mqttclient.h>

class AWSConnector : public QObject
{
    Q_OBJECT
public:
    explicit AWSConnector(QObject *parent = nullptr);
    ~AWSConnector();

    class PushNotificationsEndpoint {
    public:
        QString userId;
        QString endpointId;
        QString displayName;
    };

    void connect2AWS(const QString &endpoint, const QString &clientId, const QString &clientName, const QString &caFile, const QString &clientCertFile, const QString &clientPrivKeyFile);
    void disconnectAWS();
    bool isConnected() const;

    void setDeviceName(const QString &deviceName);
    void pairDevice(const QString &idToken, const QString &userId);

    void sendWebRtcHandshakeMessage(const QString &sessionId, const QVariantMap &map);

public slots:
    int sendPushNotification(const QString &userId, const QString &endpointId, const QString &title, const QString &text);
    void requestTURNCredentials();

signals:
    void connected();
    void disconnected();
    void devicePaired(const QString &cognritoUserId, int errorCode, const QString &message);
    void webRtcHandshakeMessageReceived(const QString &transactionId, const QVariantMap &data);
    void pushNotificationEndpointsUpdated(const QList<AWSConnector::PushNotificationsEndpoint> pushNotificationEndpoints);
    void pushNotificationEndpointAdded(const AWSConnector::PushNotificationsEndpoint &pushNotificationEndpoint);
    void pushNotificationSent(int id, int status);
    void turnCredentialsReceived(const QVariantMap &turnCredentials);

    void proxyConnectionRequestReceived(const QString &token, const QString &nonce);

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
    void onTurnCredentialsReceived(const QVariantMap &turnCredentials);


private:
    quint16 publish(const QString &topic, const QVariantMap &message);
    void subscribe(const QStringList &topics);

    void storeRegisteredFlag(bool registered);
    bool readRegisteredFlag() const;

    void storeSyncedNameCache(const QString &syncedName);
    QString readSyncedNameCache();

    QString getCertificateFingerprint(const QString &certificateFilePath) const;

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
Q_DECLARE_METATYPE(AWSConnector::PushNotificationsEndpoint)

#endif // AWSCONNECTOR_H
