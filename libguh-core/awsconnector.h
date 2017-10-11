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

#ifndef AWSCONNECTOR_H
#define AWSCONNECTOR_H

#include <QObject>
#include <QFuture>
#include <QDateTime>

#include "MbedTLS/MbedTLSConnection.hpp"
#include <mqtt/Client.hpp>
#include <mqtt/Common.hpp>
#include "util/logging/Logging.hpp"
#include "util/logging/LogMacros.hpp"
#include "util/logging/ConsoleLogSystem.hpp"

class AWSConnector : public QObject, public awsiotsdk::mqtt::SubscriptionHandlerContextData, public awsiotsdk::DisconnectCallbackContextData
{
    Q_OBJECT
public:
    explicit AWSConnector(QObject *parent = 0);
    ~AWSConnector();

    void connect2AWS(const QString &endpoint, const QString &clientId, const QString &clientName, const QString &caFile, const QString &clientCertFile, const QString &clientPrivKeyFile);
    void disconnectAWS();
    bool isConnected() const;

    void pairDevice(const QString &idToken, const QString &authToken, const QString &cognitoUserId);

    void sendWebRtcHandshakeMessage(const QString &sessionId, const QVariantMap &map);

signals:
    void connected();
    void disconnected();
    void devicePaired(const QString &cognritoUserId, int errorCode);
    void webRtcHandshakeMessageReceived(const QString &transactionId, const QVariantMap &data);

private slots:
    void doConnect();
    void onConnected();
    void registerDevice();
    void onDeviceRegistered(bool needsReconnect);
    void setupPairing();
    void onPairingsRetrieved(const QVariantList &pairings);
    void setName();
    void onDisconnected();

private:
    class SubscriptionContext: public awsiotsdk::mqtt::SubscriptionHandlerContextData
    {
    public:
        SubscriptionContext(AWSConnector *connector): c(connector) {}
        AWSConnector *c;
    };
    class DisconnectContext: public awsiotsdk::DisconnectCallbackContextData
    {
    public:
        DisconnectContext(AWSConnector *connector): c(connector) {}
        AWSConnector *c;
    };
    quint16 publish(const QString &topic, const QVariantMap &message);
    quint16 subscribe(const QStringList &topics);
    static void publishCallback(uint16_t actionId, awsiotsdk::ResponseCode rc);
    static void subscribeCallback(uint16_t actionId, awsiotsdk::ResponseCode rc);
    static awsiotsdk::ResponseCode onSubscriptionReceivedCallback(awsiotsdk::util::String topic_name, awsiotsdk::util::String payload,
                                             std::shared_ptr<awsiotsdk::mqtt::SubscriptionHandlerContextData> p_app_handler_data);
    static awsiotsdk::ResponseCode onDisconnectedCallback(awsiotsdk::util::String mqtt_client_id,
                        std::shared_ptr<awsiotsdk::DisconnectCallbackContextData> p_app_handler_data);

    void storeRegisteredFlag(bool registered);
    bool readRegisteredFlag() const;

private:
    std::shared_ptr<awsiotsdk::network::MbedTLSConnection> m_networkConnection;
    std::shared_ptr<awsiotsdk::MqttClient> m_client;
    QString m_currentEndpoint;
    QString m_caFile;
    QString m_clientCertFile;
    QString m_clientPrivKeyFile;

    QString m_clientId;
    QString m_clientName;
    QFuture<void> m_connectingFuture;
    bool m_isCleanSession = true;

    int m_transactionId = 0;
    QString m_createDeviceId;
    int m_createDeviceSubscriptionId = 0;
    QHash<quint16, QString> m_pairingRequests;
    bool m_setupInProgress = false;
    int m_reconnectCounter = 0;
    QDateTime m_lastConnectionDrop;

    std::shared_ptr<awsiotsdk::mqtt::SubscriptionHandlerContextData> m_subscriptionContextData;
    std::shared_ptr<awsiotsdk::DisconnectCallbackContextData> m_disconnectContextData;

    static AWSConnector* s_instance;
    static QHash<quint16, AWSConnector*> s_requestMap;
};

#endif // AWSCONNECTOR_H
