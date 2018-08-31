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

#include "awsconnector.h"
#include "loggingcategories.h"
#include "nymeasettings.h"

#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QtConcurrent/QtConcurrentRun>
#include <QUuid>
#include <QSettings>
#include <QSslCertificate>
#include <QFile>
#include <QSslKey>

AWSConnector::AWSConnector(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<AWSConnector::PushNotificationsEndpoint>();
    m_clientName = readSyncedNameCache();
}

AWSConnector::~AWSConnector()
{
    if (m_client) {
        m_client->disconnectFromHost();
        qCDebug(dcAWS()) << "Disconnected from AWS.";
        emit disconnected();
    }
}

void AWSConnector::connect2AWS(const QString &endpoint, const QString &clientId, const QString &clientName, const QString &caFile, const QString &clientCertFile, const QString &clientPrivKeyFile)
{
    m_shouldReconnect = true;
    m_currentEndpoint = endpoint;
    m_caFile = caFile;
    m_clientCertFile = clientCertFile;
    m_clientPrivKeyFile = clientPrivKeyFile;
    m_clientId = clientId;
    m_clientName = clientName;

    if (m_client) {
        m_shouldReconnect = true;
        m_client->disconnectFromHost();
        qCDebug(dcAWS()) << "Disconnecting from AWS";
        return;
    }

    doConnect();
}

void AWSConnector::doConnect()
{
    m_setupInProgress = true;
    m_subscriptionCache.clear();

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    QFile certFile(m_clientCertFile);
    certFile.open(QFile::ReadOnly);
    QSslCertificate certificate(certFile.readAll());

    QFile keyFile(m_clientPrivKeyFile);
    keyFile.open(QFile::ReadOnly);
    QSslKey key(keyFile.readAll(), QSsl::Rsa);

    sslConfig.setLocalCertificate(certificate);
    sslConfig.setPrivateKey(key);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);

    QFile caCertFile(m_caFile);
    caCertFile.open(QFile::ReadOnly);
    QSslCertificate caCertificate(caCertFile.readAll());

    sslConfig.setCaCertificates({caCertificate});

    m_client = new QMQTT::Client(m_currentEndpoint, 8883, sslConfig, true, this);
    m_client->setClientId(m_clientId);
    m_client->setVersion(QMQTT::V3_1_1);
    m_client->setKeepAlive(30);
    m_client->setCleanSession(true);
    m_client->setAutoReconnect(true);
    m_client->connectToHost();

    connect(m_client, &QMQTT::Client::connected, this, &AWSConnector::onConnected);
    connect(m_client, &QMQTT::Client::disconnected, this, &AWSConnector::onDisconnected);
    connect(m_client, &QMQTT::Client::error, this, [](const QMQTT::ClientError error){
        qCWarning(dcAWS()) << "An error happened in the MQTT transport" << error;
    });

    connect(m_client, &QMQTT::Client::subscribed, this, &AWSConnector::onSubscribed);
    connect(m_client, &QMQTT::Client::received, this, &AWSConnector::onSubscriptionReceived);
    connect(m_client, &QMQTT::Client::published, this, &AWSConnector::onPublished);
}

void AWSConnector::onConnected()
{
    if (!readRegisteredFlag()) {
        qCDebug(dcAWS()) << "AWS connected. Device not registered yet. Registering...";
        registerDevice();
        return;
    }
    qCDebug(dcAWS()) << "AWS connected. Device already registered in cloud.";

    // OK, we're registerd already, go straight to subscription setup
    setupSubscriptions();
}

void AWSConnector::registerDevice()
{
    // We create a temporary UUID for which will be used by the server to post the reply to our create/device call.
    // Before the first create/device call the cloud doesn't know about us. In order to receive the reply for the
    // call we need to subscribe to a topic every device can subscribe to. If we'd use our deviceId, a potential
    // black hat could snoop in all the devices we register on the system. So in case someone actually does that
    // let's give him meaningless IDs instead of real device ids.
    m_createDeviceId = QUuid::createUuid().toString().remove(QRegExp("[{}]*"));

    // first subscribe to this tmp id topic
    subscribe({QString("create/device/%1").arg(m_createDeviceId)});
}

void AWSConnector::onDeviceRegistered(bool needsReconnect)
{
    storeRegisteredFlag(true);

    if (needsReconnect) {
        qCDebug(dcAWS()) << "Disconnecting from AWS and reconnecting to use new policies";
        m_client->disconnectFromHost();
        return;
    }

    setupSubscriptions();
}

void AWSConnector::setupSubscriptions()
{
    // Subscribe to pairing info topics
    QStringList subscriptions;
    subscriptions.append(QString("%1/device/name/response").arg(m_clientId));
    subscriptions.append(QString("%1/device/users/response").arg(m_clientId));
    subscriptions.append(QString("%1/pair/response").arg(m_clientId));
    subscriptions.append(QString("%1/notify/response").arg(m_clientId));
    subscriptions.append(QString("%1/notify/info/endpoint").arg(m_clientId));
    subscriptions.append(QString("%1/services/turn/response").arg(m_clientId));
    subscribe(subscriptions);

    // fetch previous pairings
    fetchPairings();
}

void AWSConnector::fetchPairings()
{
    QVariantMap params;
    params.insert("timestamp", QDateTime::currentMSecsSinceEpoch());
    params.insert("id", ++m_transactionId);
    params.insert("command", "getUsers");
    publish(QString("%1/device/users").arg(m_clientId), params);
}

void AWSConnector::onPairingsRetrieved(const QVariantMap &pairings)
{
    m_setupInProgress = false;
    emit connected();

    qCDebug(dcAWS) << pairings.value("users").toList().count() << "devices paired in cloud.";
    if (pairings.value("users").toList().count() > 0) {
        QStringList topics;
        foreach (const QVariant &pairing, pairings.value("users").toList()) {
            topics << QString("%1/%2/#").arg(m_clientId).arg(pairing.toString());
        }
        subscribe(topics);
    }

    qCDebug(dcAWS) << pairings.value("pushNotificationsEndpoints").toList().count() << "push notification enabled users paired in cloud.";
    QList<PushNotificationsEndpoint> pushNotificationEndpoints;
    if (pairings.value("pushNotificationsEndpoints").toList().count() > 0) {
        foreach (const QVariant &pairing, pairings.value("pushNotificationsEndpoints").toList()) {
            foreach (const QString &cognitoUserId, pairing.toMap().keys()) {
                qCDebug(dcAWS()) << "User:" << cognitoUserId << "has" << pairing.toMap().value(cognitoUserId).toList().count() << "push notifications enabled devices.";
                foreach (const QVariant &endpoint, pairing.toMap().value(cognitoUserId).toList()) {
                    PushNotificationsEndpoint ep;
                    ep.userId = cognitoUserId;
                    ep.endpointId = endpoint.toMap().value("endpointId").toString();
                    ep.displayName = endpoint.toMap().value("displayName").toString();
                    pushNotificationEndpoints.append(ep);
                    qCDebug(dcAWS) << "Device:" << ep.displayName << "endpoint:" << ep.endpointId << "user:" << ep.userId;
                }
            }
        }
    }

    if (readSyncedNameCache() != m_clientName) {
        setName();
    }

    emit pushNotificationEndpointsUpdated(pushNotificationEndpoints);

    requestTURNCredentials();
}

void AWSConnector::disconnectAWS()
{
    m_shouldReconnect = false;
    if (isConnected()) {
        m_client->disconnectFromHost();
        qCDebug(dcAWS()) << "Disconnecting from AWS.";
    }
}

bool AWSConnector::isConnected() const
{
    return m_client && m_client->isConnectedToHost() && !m_setupInProgress;
}

void AWSConnector::setDeviceName(const QString &deviceName)
{
    if (m_clientName != deviceName) {
        m_clientName = deviceName;
        storeSyncedNameCache(QString());
        if (isConnected()) {
            setName();
        }
    }
}

void AWSConnector::pairDevice(const QString &idToken, const QString &userId)
{
    QVariantMap map;
    map.insert("idToken", idToken);
    map.insert("userId", userId);
    map.insert("id", ++m_transactionId);
    map.insert("timestamp", QDateTime::currentMSecsSinceEpoch());
    publish(QString("%1/pair").arg(m_clientId), map);
    m_pairingRequests.insert(m_transactionId, userId);
}

void AWSConnector::sendWebRtcHandshakeMessage(const QString &sessionId, const QVariantMap &map)
{
    publish(sessionId + "/reply", map);
}

int AWSConnector::sendPushNotification(const QString &userId, const QString &endpointId, const QString &title, const QString &text)
{
    QVariantMap params;
    params.insert("id", ++m_transactionId);
    params.insert("command", "sendPushNotification");
    params.insert("title", title);
    params.insert("body", text);
    params.insert("timestamp", QDateTime::currentMSecsSinceEpoch());
//    publish(QString("%1/notify/user/%2/%3").arg(m_clientId, userId, endpointId), params);
    Q_UNUSED(userId)
    publish(QString("%1/notify/user/%2").arg(m_clientId, endpointId), params);
    return m_transactionId;
}

void AWSConnector::requestTURNCredentials()
{
    if (!isConnected()) {
        qCWarning(dcAWS()) << "Not connected. Cannot request TURN credentials.";
        emit turnCredentialsReceived(QVariantMap());
        return;
    }
    if (!m_cachedTURNCredentials.first.isEmpty() && QDateTime::currentDateTime().secsTo(m_cachedTURNCredentials.second) > 0) {
        emit turnCredentialsReceived(m_cachedTURNCredentials.first);
        return;
    }
    qCDebug(dcAWS()) << "Requesting TURN credentials";
    QVariantMap params;
    params.insert("id", QUuid::createUuid());
    params.insert("command", "getTurnCredentials");
    params.insert("timestamp", QDateTime::currentMSecsSinceEpoch());
    publish(QString("%1/services/turn").arg(m_clientId), params);
}

quint16 AWSConnector::publish(const QString &topic, const QVariantMap &message)
{
    if (!m_setupInProgress && !isConnected()) {
        qCWarning(dcAWS()) << "Can't publish to AWS: Not connected.";
        return -1;
    }
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(message);

    QMQTT::Message msg(0, topic, jsonDoc.toJson(QJsonDocument::Compact), 1);
    qCDebug(dcAWSTraffic()) << "Publishing:" << topic << jsonDoc.toJson(QJsonDocument::Compact);
    quint16 packetId = m_client->publish(msg);
    return packetId;
}

void AWSConnector::onDisconnected()
{
    qCDebug(dcAWS) << "AWS disconnected.";
    m_client->deleteLater();
    m_client = nullptr;
    emit disconnected();

    bool needReRegistering = false;
    if (m_setupInProgress) {
        qCWarning(dcAWS()) << "Setup process interrupted by disconnect.";
        needReRegistering = true;
    } else {
        if (m_lastConnectionDrop.addSecs(60) > QDateTime::currentDateTime()) {
            m_reconnectCounter++;
        } else {
            m_reconnectCounter = 0;
        }
        m_lastConnectionDrop = QDateTime::currentDateTime();
        if (m_reconnectCounter > 5) {
            qCWarning(dcAWS()) << "Connection dropped 5 times in a row within a minute.";
            needReRegistering = true;
        }
    }

    if (needReRegistering) {
        qCDebug(dcAWS) << "Trying to reregister the device in the cloud";
        storeRegisteredFlag(false);
        storeSyncedNameCache(QString());
    }

    if (m_shouldReconnect) {
        qCDebug(dcAWS()) << "Reconnecting to AWS...";
        doConnect();
    }
}

void AWSConnector::onTurnCredentialsReceived(const QVariantMap &turnCredentials)
{
    qCDebug(dcAWS()) << "Dynamic TURN credentials received";

    m_cachedTURNCredentials.first = turnCredentials;
    m_cachedTURNCredentials.second = QDateTime::currentDateTime().addSecs(turnCredentials.value("ttl").toInt());
    emit turnCredentialsReceived(turnCredentials);

    // refresh the cache
    QTimer::singleShot((turnCredentials.value("ttl").toInt() - 10) * 1000, this, &AWSConnector::requestTURNCredentials);
    qCDebug(dcAWS()) << "Refreshing TURN credentials in" << (turnCredentials.value("ttl").toInt() - 10) << "seconds.";
}

void AWSConnector::setName()
{
    QVariantMap params;
    params.insert("id", ++m_transactionId);
    params.insert("timestamp", QDateTime::currentMSecsSinceEpoch() / 1000);
    params.insert("command", "postName");
    params.insert("name", m_clientName);
    publish(QString("%1/device/name").arg(m_clientId), params);
}

void AWSConnector::subscribe(const QStringList &topics)
{
    foreach (const QString &topic, topics) {
        if (m_subscriptionCache.contains(topic)) {
            qCDebug(dcAWS()) << "Already subscribed to topic:" << topic << ". Not resubscribing";
            continue;
        }
        qCDebug(dcAWSTraffic()) << "Topic to subscribe is" << topic;
        m_client->subscribe(topic, 1);
        m_subscriptionCache.append(topic);
    }
}

void AWSConnector::onPublished(const QMQTT::Message& message, quint16 msgid)
{
    qCDebug(dcAWS()) << "Published message:" << message.topic() << msgid;
}

void AWSConnector::onSubscribed(const QString& topic, const quint8 qos)
{
    qCDebug(dcAWSTraffic()) << "Subscribed to topic:" << topic << qos;

    if (topic.startsWith("create/device/")) {
        qCDebug(dcAWS()) << "Subscribed to create/device/";
        // We might get this callback even if we didn't explicitly ask for it as the
        // library automatically resubscribes to all the topics upon reconnect.
        if (!readRegisteredFlag()) {
            QVariantMap params;
            params.insert("id", m_createDeviceId);
            params.insert("UUID", m_clientId);
            publish("create/device", params);
        }
        return;
    }
}

void AWSConnector::onSubscriptionReceived(const QMQTT::Message &message)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.payload(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCDebug(dcAWS()) << "Failed to parse JSON from AWS subscription on topic" << message.topic() << ":" << error.errorString() << "\n" << message.payload();
        return;
    }

    qCDebug(dcAWSTraffic()) << "Subscription received: Topic:" << message.topic() << "payload:" << message.payload();

    QString topic = message.topic();
    if (topic.startsWith("create/device/")) {
        int statusCode = jsonDoc.toVariant().toMap().value("result").toMap().value("code").toInt();
        switch (statusCode) {
        case 201:
            qCDebug(dcAWS()) << "Device successfully registered to the cloud server:" << statusCode << jsonDoc.toVariant().toMap().value("result").toMap().value("message").toString();
            onDeviceRegistered(true);
            return;
        case 200:
            qCDebug(dcAWS()) << "Device already known to the cloud server:" << statusCode << jsonDoc.toVariant().toMap().value("result").toMap().value("message").toString();
            // Ok, we have confirmation that everything went fine and we can proceed, let's remember that to minimize traffic.
            onDeviceRegistered(false);
            break;
        default:
            qCWarning(dcAWS()) << "Error registering device in the cloud. AWS connetion will not work:" << statusCode << jsonDoc.toVariant().toMap().value("result").toMap().value("message").toString();
            return;
        }
    } else if (topic == QString("%1/pair/response").arg(m_clientId)) {
        int statusCode = jsonDoc.toVariant().toMap().value("status").toInt();
        quint16 id = jsonDoc.toVariant().toMap().value("id").toUInt();
        QString message = jsonDoc.toVariant().toMap().value("result").toMap().value("message").toString();
        QString userId = m_pairingRequests.take(id);
        if (statusCode != 200) {
            qCWarning(dcAWS()) << "Pairing failed:" << statusCode << message;
            emit devicePaired(userId, statusCode, message);
        } else if (!userId.isEmpty()) {
            qCDebug(dcAWS()) << "Pairing response for id:" << userId << statusCode;
            emit devicePaired(userId, statusCode, message);
            fetchPairings();
        } else {
            qCWarning(dcAWS()) << "Received a pairing response for a transaction we didn't start";
        }
    } else if (topic == QString("%1/device/users/response").arg(m_clientId)) {
        onPairingsRetrieved(jsonDoc.toVariant().toMap());
    } else if (topic == QString("%1/device/name/response").arg(m_clientId)) {
        qCDebug(dcAWS) << "Set device name in cloud with status:" << jsonDoc.toVariant().toMap().value("status").toInt();
        if (jsonDoc.toVariant().toMap().value("status").toInt() == 200) {
            storeSyncedNameCache(m_clientName);
        }
    } else if (topic.startsWith(QString("%1/eu-west-1:").arg(m_clientId)) && !topic.contains("reply") && !topic.contains("proxy")) {
        static QHash<QString, QDateTime> dupes;
        QString id = jsonDoc.toVariant().toMap().value("id").toString();
        QString type = jsonDoc.toVariant().toMap().value("type").toString();
        if (dupes.contains(id+type)) {
            qCDebug(dcAWS()) << "Dropping duplicate packet";
            return;
        }
        dupes.insert(id+type, QDateTime::currentDateTime());
        foreach (const QString &dupe, dupes.keys()) {
            if (dupes.value(dupe).addSecs(60) < QDateTime::currentDateTime()) {
                dupes.remove(dupe);
            }
        }
        qCDebug(dcAWS) << "received webrtc handshake message.";
        webRtcHandshakeMessageReceived(topic, jsonDoc.toVariant().toMap());
    } else if (topic.startsWith(QString("%1/eu-west-1:").arg(m_clientId)) && topic.contains("reply")) {
        // silently drop our own things (should not be subscribed to that in the first place)
    } else if (topic.startsWith(QString("%1/eu-west-1:").arg(m_clientId)) && topic.contains("proxy")) {
        QString token = jsonDoc.toVariant().toMap().value("token").toString();
        QString timestamp = jsonDoc.toVariant().toMap().value("timestamp").toString();
        static QHash<QString, QDateTime> dupes;
        QString packetId = topic + token + timestamp;
        if (dupes.contains(packetId)) {
            qCDebug(dcAWS()) << "Dropping duplicate packet";
            return;
        }
        dupes.insert(packetId, QDateTime::currentDateTime());
        foreach (const QString &dupe, dupes.keys()) {
            if (dupes.value(dupe).addSecs(60) < QDateTime::currentDateTime()) {
                dupes.remove(dupe);
            }
        }
        qCDebug(dcAWS) << "Proxy remote connection request received";
        proxyConnectionRequestReceived(token, timestamp);
    } else if (topic == QString("%1/notify/response").arg(m_clientId)) {
        int transactionId = jsonDoc.toVariant().toMap().value("id").toInt();
        int status = jsonDoc.toVariant().toMap().value("status").toInt();
        qCDebug(dcAWS()) << "Push notification reply for transaction" << transactionId << " Status:" << status << jsonDoc.toVariant().toMap().value("message").toString();
        emit pushNotificationSent(transactionId, status);
    } else if (topic == QString("%1/notify/info/endpoint").arg(m_clientId)) {
        QVariantMap endpoint = jsonDoc.toVariant().toMap().value("newPushNotificationsEndpoint").toMap();
        Q_ASSERT(endpoint.keys().count() == 1);
        QString cognitoId = endpoint.keys().first();
        PushNotificationsEndpoint ep;
        ep.userId = cognitoId;
        ep.endpointId = endpoint.value(cognitoId).toMap().value("endpointId").toString();
        ep.displayName = endpoint.value(cognitoId).toMap().value("displayName").toString();
        emit pushNotificationEndpointAdded(ep);
    } else if (topic == QString("%1/services/turn/response").arg(m_clientId)) {
        QVariantMap turnCreds = jsonDoc.toVariant().toMap();
        if (turnCreds.value("result").toMap().value("code").toInt() != 201) {
            qCWarning(dcAWS()) << "Error retrieving TURN credentials:" << turnCreds.value("result").toMap().value("code").toInt() << turnCreds.value("result").toMap().value("message").toString();
            return;
        }
        onTurnCredentialsReceived(turnCreds.value("turnCredentials").toMap());
    } else {
        qCWarning(dcAWS()) << "Unhandled subscription received!" << topic << message.payload();
    }
}

void AWSConnector::storeRegisteredFlag(bool registered)
{
    QSettings settings(NymeaSettings::storagePath() + "/cloudstatus.conf", QSettings::IniFormat);
    settings.setValue("registered", registered);
}

bool AWSConnector::readRegisteredFlag() const
{
    QSettings settings(NymeaSettings::storagePath() + "/cloudstatus.conf", QSettings::IniFormat);
    return settings.value("registered", false).toBool();
}

void AWSConnector::storeSyncedNameCache(const QString &syncedName)
{
    QSettings settings(NymeaSettings::storagePath() + "/cloudstatus.conf", QSettings::IniFormat);
    settings.setValue("syncedName", syncedName);
}

QString AWSConnector::readSyncedNameCache()
{
    QSettings settings(NymeaSettings::storagePath() + "/cloudstatus.conf", QSettings::IniFormat);
    return settings.value("syncedName", QString()).toString();
}

QString AWSConnector::getCertificateFingerprint(const QString &certificateFile) const
{
    QFile certFile(certificateFile);
    if (!certFile.open(QFile::ReadOnly)) {
        qCWarning(dcAWS()) << "Error openi<ng certificate file" << certificateFile;
        return QString();
    }
    QSslCertificate crt = QSslCertificate(certFile.readAll());
    QByteArray output;
    QByteArray digest = crt.digest(QCryptographicHash::Sha256);
    for (int i = 0; i < digest.length(); i++) {
        if (output.length() > 0) {
            output.append(":");
        }
        output.append(digest.mid(i,1).toHex().toUpper());
    }
    return output;
}
