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

#include "awsconnector.h"
#include "loggingcategories.h"
#include "guhsettings.h"

#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QtConcurrent/QtConcurrentRun>
#include <QUuid>
#include <QSettings>

using namespace awsiotsdk;
using namespace awsiotsdk::network;
using namespace awsiotsdk::mqtt;

QHash<quint16, AWSConnector*> AWSConnector::s_requestMap;

// Somehow the linker fails to find this... missing in awsiotsdk?
DisconnectCallbackContextData::~DisconnectCallbackContextData() {}

AWSConnector::AWSConnector(QObject *parent) : QObject(parent)
{
    // Enable some AWS logging (does not regard our logging categories)
//    std::shared_ptr<awsiotsdk::util::Logging::ConsoleLogSystem> p_log_system =
//            std::make_shared<awsiotsdk::util::Logging::ConsoleLogSystem>(awsiotsdk::util::Logging::LogLevel::Info);
//    awsiotsdk::util::Logging::InitializeAWSLogging(p_log_system);
    m_disconnectContextData = std::shared_ptr<awsiotsdk::DisconnectCallbackContextData>(new DisconnectContext(this));
    m_subscriptionContextData = std::shared_ptr<awsiotsdk::mqtt::SubscriptionHandlerContextData>(new SubscriptionContext(this));

    m_clientName = readSyncedNameCache();
}

AWSConnector::~AWSConnector()
{
    qCDebug(dcAWS()) << "Stopping AWS connection. This might take a while...";
    m_client.reset();
    m_networkConnection.reset();
    qCDebug(dcAWS()) << "AWS connection stopped.";
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

    m_client.reset();
    m_networkConnection.reset();

    doConnect();
}

void AWSConnector::doConnect()
{
    m_setupInProgress = true;
    m_networkConnection = std::shared_ptr<MbedTLSConnection>(new MbedTLSConnection(
                                                                 m_currentEndpoint.toStdString(),
                                                                 8883,
                                                                 m_caFile.toStdString(),
                                                                 m_clientCertFile.toStdString(),
                                                                 m_clientPrivKeyFile.toStdString(),
                                                                 std::chrono::milliseconds(3000),
                                                                 std::chrono::milliseconds(3000),
                                                                 std::chrono::milliseconds(3000),
                                                                 true
                                                                 ));
    m_client = MqttClient::Create(m_networkConnection, std::chrono::milliseconds(2800), &onDisconnectedCallback, m_disconnectContextData);

    m_client->SetAutoReconnectEnabled(true);
    m_client->SetMaxReconnectBackoffTimeout(std::chrono::seconds(10));

    qCDebug(dcAWS()) << "Connecting to AWS with ID:" << m_clientId << "endpoint:" << m_currentEndpoint << "Min reconnect timeout:" << m_client->GetMinReconnectBackoffTimeout().count() << "Max reconnect timeout:" << (quint32)m_client->GetMaxReconnectBackoffTimeout().count();
    m_connectingFuture = QtConcurrent::run([&]() {
        ResponseCode rc = m_client->Connect(std::chrono::milliseconds(3000), true, mqtt::Version::MQTT_3_1_1, std::chrono::seconds(1200), Utf8String::Create(m_clientId.toStdString()), nullptr, nullptr, nullptr);
        if (rc == ResponseCode::MQTT_CONNACK_CONNECTION_ACCEPTED) {
            staticMetaObject.invokeMethod(this, "onConnected", Qt::QueuedConnection);
        } else {
            qCWarning(dcAWS) << "Error connecting to AWS. Response code:" << QString::fromStdString(ResponseHelper::ToString(rc));
            m_client.reset();
            m_networkConnection.reset();
            QTimer::singleShot(10000, this, &AWSConnector::doConnect);
        }
    });
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
    m_createDeviceSubscriptionId = subscribe({QString("create/device/%1").arg(m_createDeviceId)});
}

void AWSConnector::onDeviceRegistered(bool needsReconnect)
{
    storeRegisteredFlag(true);

    if (needsReconnect) {
        qCDebug(dcAWS()) << "Disconnecting from AWS and reconnecting to use new policies";
        QtConcurrent::run([&]() {
            m_client->Disconnect(std::chrono::milliseconds(500));
            m_client.reset();
            m_networkConnection.reset();
            QTimer::singleShot(1000, this, &AWSConnector::doConnect);
        });
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

void AWSConnector::onPairingsRetrieved(const QVariantList &pairings)
{
    QStringList topics;
    foreach (const QVariant &pairing, pairings) {
        topics << QString("%1/%2/#").arg(m_clientId).arg(pairing.toString());
    }
    subscribe(topics);

    if (readSyncedNameCache() != m_clientName) {
        setName();
    }

    m_setupInProgress = false;
    emit connected();
}

void AWSConnector::disconnectAWS()
{
    m_shouldReconnect = false;
    if (isConnected()) {
        m_client->Disconnect(std::chrono::seconds(2));
        m_client.reset();
        m_networkConnection.reset();
        qCDebug(dcAWS()) << "Disconnected from AWS.";
        emit disconnected();
    }
}

bool AWSConnector::isConnected() const
{
    return m_connectingFuture.isFinished() && m_networkConnection && m_client && m_client->IsConnected() && !m_setupInProgress;
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

quint16 AWSConnector::publish(const QString &topic, const QVariantMap &message)
{
    if (!m_setupInProgress && !isConnected()) {
        qCWarning(dcAWS()) << "Can't publish to AWS: Not connected.";
        return -1;
    }
    QString fullTopic = topic;
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(message);

    uint16_t packetId = 0;
    ResponseCode res = m_client->PublishAsync(Utf8String::Create(fullTopic.toStdString()), false, false, mqtt::QoS::QOS1, jsonDoc.toJson(QJsonDocument::Compact).toStdString(), &publishCallback, packetId);
    qCDebug(dcAWSTraffic()) << "publish call queued with status:" << QString::fromStdString(ResponseHelper::ToString(res)) << packetId << "for topic" << topic << jsonDoc.toJson();
    s_requestMap.insert(packetId, this);
    return packetId;
}

void AWSConnector::onDisconnected()
{
    qCDebug(dcAWS) << "AWS disconnected.";
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

void AWSConnector::setName()
{
    QVariantMap params;
    params.insert("id", ++m_transactionId);
    params.insert("timestamp", QDateTime::currentMSecsSinceEpoch() / 1000);
    params.insert("command", "postName");
    params.insert("name", m_clientName);
    publish(QString("%1/device/name").arg(m_clientId), params);
}

quint16 AWSConnector::subscribe(const QStringList &topics)
{
    util::Vector<std::shared_ptr<mqtt::Subscription>> subscription_list;
    foreach (const QString &topic, topics) {
        qCDebug(dcAWSTraffic()) << "Topic to subscribe is" << topic;
        if (!Subscription::IsValidTopicName(topic.toStdString())) {
            qCWarning(dcAWS()) << "Trying to subscribe to invalid topic:" << topic;
            continue;
        }
        auto subscription = mqtt::Subscription::Create(Utf8String::Create(topic.toStdString()), mqtt::QoS::QOS1, &onSubscriptionReceivedCallback, m_subscriptionContextData);
        subscription_list.push_back(subscription);
    }

    uint16_t packetId;
    ResponseCode res = m_client->SubscribeAsync(subscription_list, subscribeCallback, packetId);
    qCDebug(dcAWSTraffic()) << "Subscribe call queued with status:" << QString::fromStdString(ResponseHelper::ToString(res)) << "Packet ID:" << packetId;
    s_requestMap.insert(packetId, this);
    return packetId;
}

void AWSConnector::publishCallback(uint16_t actionId, ResponseCode rc)
{
    AWSConnector* obj = s_requestMap.take(actionId);
    if (!obj) {
        qCWarning(dcAWS())<< "Received a response callback but don't have an object waiting for it.";
        return;
    }

    switch (rc) {
    case ResponseCode::SUCCESS:
        qCDebug(dcAWSTraffic()) << "Successfully published" << actionId;
        break;
    default:
        qCDebug(dcAWS())<< "Error publishing data to AWS:" << QString::fromStdString(ResponseHelper::ToString(rc));
    }
}

void AWSConnector::subscribeCallback(uint16_t actionId, ResponseCode rc)
{
    if (rc != ResponseCode::SUCCESS) {
        qCWarning(dcAWS()) << "Error subscribing to" << actionId << QString::fromStdString(ResponseHelper::ToString(rc));
        return;
    }

    AWSConnector *connector = s_requestMap.take(actionId);
    if (!connector) {
        qCWarning(dcAWS()) << "received a subscribe callback but don't have a request id for it.";
        return;
    }

    if (actionId == connector->m_createDeviceSubscriptionId) {
        qCDebug(dcAWS()) << "subscribed to create/device/response";
        // We might get this callback even if we didn't explicitly ask for it as the
        // library automatically resubscribes to all the topics upon reconnect.
        if (!connector->readRegisteredFlag()) {
            QVariantMap params;
            params.insert("id", connector->m_createDeviceId);
            params.insert("UUID", connector->m_clientId);
            connector->publish("create/device", params);
        }
        return;
    }

    qCDebug(dcAWSTraffic()) << "Successfully subscribed (actionId:" << actionId << ")";
}

ResponseCode AWSConnector::onSubscriptionReceivedCallback(util::String topic_name, util::String payload, std::shared_ptr<SubscriptionHandlerContextData> p_app_handler_data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(payload), &error);
    if (error.error != QJsonParseError::NoError) {
        qCDebug(dcAWS()) << "Failed to parse JSON from AWS subscription on topic" << QString::fromStdString(topic_name) << ":" << error.errorString() << "\n" << QString::fromStdString(payload);
        return ResponseCode::JSON_PARSING_ERROR;
    }

    qCDebug(dcAWSTraffic()) << "Subscription received: Topic:" << QString::fromStdString(topic_name) << "payload:" << QString::fromStdString(payload);

    AWSConnector *connector = dynamic_cast<SubscriptionContext*>(p_app_handler_data.get())->c;
    QString topic = QString::fromStdString(topic_name);
    if (topic.startsWith("create/device/")) {
        int statusCode = jsonDoc.toVariant().toMap().value("result").toMap().value("code").toInt();
        switch (statusCode) {
        case 201:
            qCDebug(dcAWS()) << "Device successfully registered to the cloud server:" << statusCode << jsonDoc.toVariant().toMap().value("result").toMap().value("message").toString();
            connector->staticMetaObject.invokeMethod(connector, "onDeviceRegistered", Qt::QueuedConnection, Q_ARG(bool, true));
            return ResponseCode::SUCCESS;
        case 200:
            qCDebug(dcAWS()) << "Device already known to the cloud server:" << statusCode << jsonDoc.toVariant().toMap().value("result").toMap().value("message").toString();
            // Ok, we have confirmation that everything went fine and we can proceed, let's remember that to minimize traffic.
            connector->staticMetaObject.invokeMethod(connector, "onDeviceRegistered", Qt::QueuedConnection, Q_ARG(bool, false));
            break;
        default:
            qCWarning(dcAWS()) << "Error registering device in the cloud. AWS connetion will not work:" << statusCode << jsonDoc.toVariant().toMap().value("result").toMap().value("message").toString();
            return ResponseCode::SUCCESS;
        }
    } else if (topic == QString("%1/pair/response").arg(connector->m_clientId)) {
        int statusCode = jsonDoc.toVariant().toMap().value("status").toInt();
        int id = jsonDoc.toVariant().toMap().value("id").toInt();
        QString message = jsonDoc.toVariant().toMap().value("result").toMap().value("message").toString();
        QString userId = connector->m_pairingRequests.take(id);
        if (statusCode != 200) {
            qCWarning(dcAWS()) << "Pairing failed:" << statusCode << message;
            emit connector->devicePaired(userId, statusCode, message);
        } else if (!userId.isEmpty()) {
            qCDebug(dcAWS()) << "Pairing response for id:" << userId << statusCode;
            emit connector->devicePaired(userId, statusCode, message);
            connector->staticMetaObject.invokeMethod(connector, "fetchPairings", Qt::QueuedConnection);
        } else {
            qCWarning(dcAWS()) << "Received a pairing response for a transaction we didn't start";
        }
    } else if (topic == QString("%1/device/users/response").arg(connector->m_clientId)) {
        if (jsonDoc.toVariant().toMap().value("users").toList().isEmpty()) {
            qCDebug(dcAWS()) << "No devices paired yet...";
            return ResponseCode::SUCCESS;
        }
        qCDebug(dcAWS) << jsonDoc.toVariant().toMap().value("users").toList().count() << "devices paired in cloud.";
        connector->staticMetaObject.invokeMethod(connector, "onPairingsRetrieved", Qt::QueuedConnection, Q_ARG(QVariantList, jsonDoc.toVariant().toMap().value("users").toList()));
    } else if (topic == QString("%1/device/name/response").arg(connector->m_clientId)) {
        qCDebug(dcAWS) << "Set device name in cloud with status:" << jsonDoc.toVariant().toMap().value("status").toInt();
        if (jsonDoc.toVariant().toMap().value("status").toInt() == 200) {
            connector->storeSyncedNameCache(connector->m_clientName);
        }
    } else if (topic.startsWith(QString("%1/eu-west-1:").arg(connector->m_clientId)) && !topic.contains("reply")) {
        static QStringList dupes;
        QString id = jsonDoc.toVariant().toMap().value("id").toString();
        QString type = jsonDoc.toVariant().toMap().value("type").toString();
        if (dupes.contains(id+type)) {
            qCDebug(dcAWS()) << "Dropping duplicate packet";
            return ResponseCode::SUCCESS;
        }
        dupes.append(id+type);

        qCDebug(dcAWS) << "received webrtc handshake message" << topic << jsonDoc.toJson();
        connector->webRtcHandshakeMessageReceived(topic, jsonDoc.toVariant().toMap());
    } else if (topic.startsWith(QString("%1/eu-west-1:").arg(connector->m_clientId)) && topic.contains("reply")) {
        // silently drop our own things (should not be subscribed to that in the first place)
    } else {
        qCWarning(dcAWS) << "Unhandled subscription received!" << topic << QString::fromStdString(payload);
    }
    return ResponseCode::SUCCESS;
}

ResponseCode AWSConnector::onDisconnectedCallback(util::String mqtt_client_id, std::shared_ptr<DisconnectCallbackContextData> p_app_handler_data)
{
    Q_UNUSED(mqtt_client_id)
    AWSConnector* connector = dynamic_cast<DisconnectContext*>(p_app_handler_data.get())->c;
    connector->staticMetaObject.invokeMethod(connector, "onDisconnected", Qt::QueuedConnection);
    return ResponseCode::SUCCESS;
}

void AWSConnector::storeRegisteredFlag(bool registered)
{
    QSettings settings(GuhSettings::storagePath() + "/cloudstatus.conf", QSettings::IniFormat);
    settings.setValue("registered", registered);
}

bool AWSConnector::readRegisteredFlag() const
{
    QSettings settings(GuhSettings::storagePath() + "/cloudstatus.conf", QSettings::IniFormat);
    return settings.value("registered", false).toBool();
}

void AWSConnector::storeSyncedNameCache(const QString &syncedName)
{
    QSettings settings(GuhSettings::storagePath() + "/cloudstatus.conf", QSettings::IniFormat);
    settings.setValue("syncedName", syncedName);
}

QString AWSConnector::readSyncedNameCache()
{
    QSettings settings(GuhSettings::storagePath() + "/cloudstatus.conf", QSettings::IniFormat);
    return settings.value("syncedName", QString()).toString();
}
