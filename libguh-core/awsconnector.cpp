#include "awsconnector.h"
#include "loggingcategories.h"

#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QtConcurrent/QtConcurrentRun>

using namespace awsiotsdk;
using namespace awsiotsdk::network;
using namespace awsiotsdk::mqtt;

QHash<quint16, AWSConnector*> AWSConnector::s_requestMap;

AWSConnector::AWSConnector(QObject *parent) : QObject(parent)
{
    connect(this, &AWSConnector::connected, this, &AWSConnector::onConnected);
}

AWSConnector::~AWSConnector()
{
}

void AWSConnector::connect2AWS(const QString &endpoint, const QString &clientId, const QString &caFile, const QString &clientCertFile, const QString &clientPrivKeyFile)
{
    m_networkConnection = std::shared_ptr<MbedTLSConnection>(new MbedTLSConnection(
                                                                 endpoint.toStdString(),
                                                                 8883,
                                                                 caFile.toStdString(),
                                                                 clientCertFile.toStdString(),
                                                                 clientPrivKeyFile.toStdString(),
                                                                 std::chrono::milliseconds(30000),
                                                                 std::chrono::milliseconds(30000),
                                                                 std::chrono::milliseconds(30000),
                                                                 true
                                                                 ));
    m_client = MqttClient::Create(m_networkConnection, std::chrono::milliseconds(30000));
    m_client->SetDisconnectCallbackPtr(&onDisconnected, std::shared_ptr<DisconnectCallbackContextData>(this));
    m_client->SetAutoReconnectEnabled(true);
    m_clientId = clientId;

    m_connectingFuture = QtConcurrent::run([&]() {
        ResponseCode rc = m_client->Connect(std::chrono::milliseconds(30000), true, mqtt::Version::MQTT_3_1_1, std::chrono::seconds(60), Utf8String::Create(m_clientId.toStdString()), nullptr, nullptr, nullptr);
        if (rc == ResponseCode::MQTT_CONNACK_CONNECTION_ACCEPTED) {
            qCDebug(dcCloud) << "Connected to AWS.";
            emit connected();
        } else {
            qCWarning(dcCloud) << "Error connecting to AWS. Response code:" << QString::fromStdString(ResponseHelper::ToString(rc));
            m_client.reset();
            m_networkConnection.reset();
        }
    });
}

DisconnectCallbackContextData::~DisconnectCallbackContextData() {}

void AWSConnector::disconnectAWS()
{
    if (isConnected()) {
        m_client->Disconnect(std::chrono::seconds(2));
    }
}

bool AWSConnector::isConnected() const
{
    return m_connectingFuture.isFinished() && m_networkConnection && m_client && m_client->IsConnected();
}


quint16 AWSConnector::publish(const QString &topic, const QVariantMap &message)
{
    if (!isConnected()) {
        qCWarning(dcCloud()) << "Can't publish to AWS: Not connected.";
        return -1;
    }
    QString fullTopic = QString("%1/%2").arg(m_clientId, topic);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(message);

    uint16_t packetId = 0;
    ResponseCode res = m_client->PublishAsync(Utf8String::Create(fullTopic.toStdString()), false, false, mqtt::QoS::QOS1, jsonDoc.toJson().toStdString(), &publishCallback, packetId);
    qCDebug(dcCloud()) << "publish call queued with status:" << QString::fromStdString(ResponseHelper::ToString(res)) << packetId;
    s_requestMap.insert(packetId, this);
    return packetId;
}

void AWSConnector::subscribe(const QStringList &topics)
{
    m_subscribedTopics.append(topics);

    if (!isConnected()) {
        qCDebug(dcCloud()) << "Can't subscribe to AWS: Not connected. Subscription will happen upon next connection.";
        return;
    }
    subscribeInternally(topics);
}

void AWSConnector::onConnected()
{
    if (!m_subscribedTopics.isEmpty()) {
        subscribeInternally(m_subscribedTopics);
    }
}

void AWSConnector::subscribeInternally(const QStringList &topics)
{
    util::Vector<std::shared_ptr<mqtt::Subscription>> subscription_list;
    foreach (const QString &topic, topics) {
        QString finalTopic = QString("%1/%2").arg(m_clientId, topic);
        qCDebug(dcCloud()) << "topic to subscribe is" << finalTopic << "is valid topic:" << Subscription::IsValidTopicName(finalTopic.toStdString());
        auto subscription = mqtt::Subscription::Create(Utf8String::Create(finalTopic.toStdString()), mqtt::QoS::QOS1, &onSubscriptionReceivedCallback, std::shared_ptr<SubscriptionHandlerContextData>(this));
        subscription_list.push_back(subscription);
    }


    uint16_t packetId;
    ResponseCode res = m_client->SubscribeAsync(subscription_list, subscribeCallback, packetId);
    qCDebug(dcCloud()) << "subscribe call queued with status:" << QString::fromStdString(ResponseHelper::ToString(res)) << packetId;
    s_requestMap.insert(packetId, this);
}

void AWSConnector::publishCallback(uint16_t actionId, ResponseCode rc)
{
    AWSConnector* obj = s_requestMap.take(actionId);
    if (!obj) {
        qCWarning(dcCloud())<< "Received a response callback but don't have an object waiting for it.";
        return;
    }

    switch (rc) {
    case ResponseCode::SUCCESS:
        emit obj->responseReceived(actionId, true);
        qCDebug(dcCloud()) << "Successfully published" << actionId;
        break;
    default:
        qCDebug(dcCloud())<< "Error publishing data to AWS:" << QString::fromStdString(ResponseHelper::ToString(rc));
        emit obj->responseReceived(actionId, false);
    }
}

void AWSConnector::subscribeCallback(uint16_t actionId, ResponseCode rc)
{
    qCDebug(dcCloud()) << "subscribed to topic" << actionId << QString::fromStdString(ResponseHelper::ToString(rc));
}

ResponseCode AWSConnector::onSubscriptionReceivedCallback(util::String topic_name, util::String payload, std::shared_ptr<SubscriptionHandlerContextData> p_app_handler_data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(payload), &error);
    if (error.error != QJsonParseError::NoError) {
        qCDebug(dcCloud()) << "Failed to parse JSON from AWS subscription on topic" << QString::fromStdString(topic_name) << ":" << error.errorString() << "\n" << QString::fromStdString(payload);
        return ResponseCode::JSON_PARSING_ERROR;
    }

    AWSConnector *connector = dynamic_cast<AWSConnector*>(p_app_handler_data.get());
    QString topic = QString::fromStdString(topic_name);
    topic.remove(QRegExp("^" + connector->m_clientId + "/"));
    emit connector->subscriptionReceived(topic, jsonDoc.toVariant().toMap());
    return ResponseCode::SUCCESS;
}

ResponseCode AWSConnector::onDisconnected(util::String mqtt_client_id, std::shared_ptr<DisconnectCallbackContextData> p_app_handler_data)
{
    Q_UNUSED(p_app_handler_data)
    qCDebug(dcCloud()) << "disconnected" << QString::fromStdString(mqtt_client_id);
    return ResponseCode::SUCCESS;
}
