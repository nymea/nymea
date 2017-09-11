#ifndef AWSCONNECTOR_H
#define AWSCONNECTOR_H

#include <QObject>
#include <QFuture>

#include "MbedTLS/MbedTLSConnection.hpp"
#include <mqtt/Client.hpp>
#include <mqtt/Common.hpp>

class AWSConnector : public QObject, public awsiotsdk::mqtt::SubscriptionHandlerContextData, public awsiotsdk::DisconnectCallbackContextData
{
    Q_OBJECT
public:
    explicit AWSConnector(QObject *parent = 0);
    ~AWSConnector();

    void connect2AWS(const QString &endpoint, const QString &clientId, const QString &caFile, const QString &clientCertFile, const QString &clientPrivKeyFile);
    void disconnectAWS();
    bool isConnected() const;

    quint16 publish(const QString &topic, const QVariantMap &message);

    void subscribe(const QStringList &topics);

signals:
    void connected();
    void responseReceived(quint16 id, bool success);
    void subscriptionReceived(const QString &topic, const QVariantMap &data);

private slots:
    void onConnected();

private:
    void subscribeInternally(const QStringList &topcis);
    static void publishCallback(uint16_t actionId, awsiotsdk::ResponseCode rc);
    static void subscribeCallback(uint16_t actionId, awsiotsdk::ResponseCode rc);
    static awsiotsdk::ResponseCode onSubscriptionReceivedCallback(awsiotsdk::util::String topic_name, awsiotsdk::util::String payload,
                                             std::shared_ptr<SubscriptionHandlerContextData> p_app_handler_data);
    static awsiotsdk::ResponseCode onDisconnected(awsiotsdk::util::String mqtt_client_id,
                        std::shared_ptr<DisconnectCallbackContextData> p_app_handler_data);

private:
    std::shared_ptr<awsiotsdk::network::MbedTLSConnection> m_networkConnection;
    std::shared_ptr<awsiotsdk::MqttClient> m_client;

    QString m_clientId;
    QFuture<void> m_connectingFuture;
    QStringList m_subscribedTopics;

    static QHash<quint16, AWSConnector*> s_requestMap;
};

#endif // AWSCONNECTOR_H
