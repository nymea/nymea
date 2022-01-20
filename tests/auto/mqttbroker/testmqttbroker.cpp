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

#include "nymeatestbase.h"
#include "nymeacore.h"
#include "servers/mqttbroker.h"
#include "servers/mocktcpserver.h"

#include <mqttclient.h>

#include <QXmlReader>

using namespace nymeaserver;

class TestMqttBroker: public NymeaTestBase
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testServerConfigurationAPI();
    void testPolicyConfigurationAPI();

    void testConnectAuthentication_data();
    void testConnectAuthentication();

    void testPublishPolicy_data();
    void testPublishPolicy();

    void testSubscribePolicy_data();
    void testSubscribePolicy();
};

void TestMqttBroker::initTestCase()
{
    NymeaTestBase::initTestCase("*.debug=false\nnymea.mqtt*.debug=true\nMqtt.debug=true\nJsonRpc.debug=true");
}

void TestMqttBroker::testServerConfigurationAPI()
{
    // Set up notifications spy
    enableNotifications({"Configuration"});
    QSignalSpy notificationsSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    // Get server instances
    QVariant response = injectAndWait("Configuration.GetMqttServerConfigurations");
    QVariantMap responseParams = response.toMap().value("params").toMap();
    qCDebug(dcMqtt) << "RESP:" << response;
    QVERIFY2(responseParams.value("mqttServerConfigurations").toList().count() == 1, "There should be one default MQTT server instance");
    QVERIFY2(responseParams.value("mqttServerConfigurations").toList().first().toMap().value("port").toInt() == 1883, "Default MQTT port should be 1883");

    // Add a server
    QVariantMap params;
    QVariantMap configuration;
    configuration.insert("id", "testconfig");
    configuration.insert("address", "127.0.0.1");
    configuration.insert("port", 1885);
    configuration.insert("sslEnabled", false);
    configuration.insert("authenticationEnabled", false);
    params.insert("configuration", configuration);
    response = injectAndWait("Configuration.SetMqttServerConfiguration", params);
    responseParams = response.toMap().value("params").toMap();
    QCOMPARE(responseParams.value("configurationError").toString(), QString("ConfigurationErrorNoError"));

    QTest::qWait(200);
    QVariantList notificationsList = checkNotifications(notificationsSpy, "Configuration.MqttServerConfigurationChanged");
    QCOMPARE(notificationsList.count(), 1);

    // Get config again and verify it's 2 now
    response = injectAndWait("Configuration.GetMqttServerConfigurations");
    responseParams = response.toMap().value("params").toMap();
    QVERIFY2(responseParams.value("mqttServerConfigurations").toList().count() == 2, "There should be 2 MQTT server instances now");
    QVariantMap newConfig = responseParams.value("mqttServerConfigurations").toList().first().toMap();
    if (newConfig.value("id").toString() != "testconfig") {
        newConfig = responseParams.value("mqttServerConfigurations").toList().at(1).toMap();
    }
    QCOMPARE(newConfig.value("port").toInt(), 1885);

    // Check if we can connect
    MqttClient* mqttClient = new MqttClient("testclient", this);
    mqttClient->setAutoReconnect(false);
    QSignalSpy connectedSpy(mqttClient, &MqttClient::connected);
    QSignalSpy disconnectedSpy(mqttClient, &MqttClient::disconnected);
    mqttClient->connectToHost("127.0.0.1", 1885);
    if (connectedSpy.count() == 0) {
        connectedSpy.wait();
    }
    QVERIFY2(connectedSpy.count() == 1, "Mqtt client didn't connect");
    QVERIFY2(connectedSpy.first().at(0).value<Mqtt::ConnectReturnCode>() == Mqtt::ConnectReturnCodeAccepted, "Connection not accepted");

    // Update same configuration to a different port
    notificationsSpy.clear();
    params.clear();
    configuration.insert("port", 1886);
    params.insert("configuration", configuration);
    response = injectAndWait("Configuration.SetMqttServerConfiguration", params);
    responseParams = response.toMap().value("params").toMap();
    QCOMPARE(responseParams.value("configurationError").toString(), QString("ConfigurationErrorNoError"));

    QTest::qWait(200);
    notificationsList = checkNotifications(notificationsSpy, "Configuration.MqttServerConfigurationChanged");
    QCOMPARE(notificationsList.count(), 1);

    // Get config again and verify it's updated
    response = injectAndWait("Configuration.GetMqttServerConfigurations");
    responseParams = response.toMap().value("params").toMap();
    QVERIFY2(responseParams.value("mqttServerConfigurations").toList().count() == 2, "There should be 2 MQTT server instances now");
    newConfig = responseParams.value("mqttServerConfigurations").toList().first().toMap();
    if (newConfig.value("id").toString() != "testconfig") {
        newConfig = responseParams.value("mqttServerConfigurations").toList().at(1).toMap();
    }
    QCOMPARE(newConfig.value("port").toInt(), 1886);

    // The client should get disconnected because of the server going down
    if (disconnectedSpy.count() == 0) disconnectedSpy.wait();
    QVERIFY2(disconnectedSpy.count() == 1, "Client didn't disconnect but server has gone away?");

    // Connect the client to the new port
    connectedSpy.clear();
    disconnectedSpy.clear();
    mqttClient->connectToHost("127.0.0.1", 1886);
    if (connectedSpy.count() == 0) {
        connectedSpy.wait();
    }
    QVERIFY2(connectedSpy.count() == 1, "Mqtt client didn't connect");
    QVERIFY2(connectedSpy.first().at(0).value<Mqtt::ConnectReturnCode>() == Mqtt::ConnectReturnCodeAccepted, "Connection not accepted");

    // Delete the server config
    notificationsSpy.clear();
    params.clear();
    params.insert("id", "testconfig");
    response = injectAndWait("Configuration.DeleteMqttServerConfiguration", params);
    QCOMPARE(response.toMap().value("params").toMap().value("configurationError").toString(), QString("ConfigurationErrorNoError"));

    // The client should get disconnected because of the server going down
    disconnectedSpy.wait();
    QVERIFY2(disconnectedSpy.count() == 1, "Client didn't disconnect but server has gone away?");

    QTest::qWait(200);
    notificationsList = checkNotifications(notificationsSpy, "Configuration.MqttServerConfigurationRemoved");
    QCOMPARE(notificationsList.count(), 1);

    // Get config again and verify it's updated
    response = injectAndWait("Configuration.GetMqttServerConfigurations");
    responseParams = response.toMap().value("params").toMap();
    QVERIFY2(responseParams.value("mqttServerConfigurations").toList().count() == 1, "There should be 1 MQTT server instance now");
}

void TestMqttBroker::testPolicyConfigurationAPI()
{
    // Set up notifications spy
    enableNotifications({"Configuration"});
    QSignalSpy notificationsSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    // Get polcies
    QVariant response = injectAndWait("Configuration.GetMqttPolicies");
    QVariantMap responseParams = response.toMap().value("params").toMap();
    QVERIFY2(responseParams.value("mqttPolicies").toList().count() == 0, "There should be no default MQTT polices");

    // Connect and fail
    MqttClient* mqttClient = new MqttClient("client1", this);
    mqttClient->setAutoReconnect(false);
    mqttClient->setUsername("testuser");
    mqttClient->setPassword("testpassword");
    QSignalSpy connectedSpy(mqttClient, &MqttClient::connected);
    QSignalSpy disconnectedSpy(mqttClient, &MqttClient::disconnected);
    mqttClient->connectToHost("127.0.0.1", 1883);
    if (connectedSpy.count() == 0) {
        connectedSpy.wait();
    }
    QVERIFY2(connectedSpy.count() == 1, "Mqtt client didn't connect");
    QCOMPARE(connectedSpy.first().at(0).value<Mqtt::ConnectReturnCode>(), Mqtt::ConnectReturnCodeIdentifierRejected);
    QCOMPARE(disconnectedSpy.count(), 1); // Connection should drop

    // Add a policy for client1
    QVariantMap params;
    QVariantMap policy;
    policy.insert("clientId", "client1");
    policy.insert("username", "testuser");
    policy.insert("password", "testpassword");
    policy.insert("allowedPublishTopicFilters", QStringList() << "#");
    policy.insert("allowedSubscribeTopicFilters", QStringList() << "#");
    params.insert("policy", policy);
    response = injectAndWait("Configuration.SetMqttPolicy", params);
    responseParams = response.toMap().value("params").toMap();
    QCOMPARE(responseParams.value("configurationError").toString(), QString("ConfigurationErrorNoError"));

    QTest::qWait(200);
    QVariantList notificationsList = checkNotifications(notificationsSpy, "Configuration.MqttPolicyChanged");
    QCOMPARE(notificationsList.count(), 1);

    // Get polcies, there should be one now
    response = injectAndWait("Configuration.GetMqttPolicies");
    responseParams = response.toMap().value("params").toMap();
    QCOMPARE(responseParams.value("mqttPolicies").toList().count(), 1);

    // Connect and succeed
    connectedSpy.clear();
    disconnectedSpy.clear();
    mqttClient->connectToHost("127.0.0.1", 1883);
    QVERIFY2(connectedSpy.wait(), "Connection failed");
    QCOMPARE(connectedSpy.first().at(0).value<Mqtt::ConnectReturnCode>(), Mqtt::ConnectReturnCodeAccepted);
    mqttClient->disconnectFromHost();

    // edit policy
    params.clear();
    policy.insert("username", "testuser");
    policy.insert("password", "testpassword2");
    policy.insert("allowedPublishTopicFilters", QStringList() << "#");
    policy.insert("allowedSubscribeTopicFilters", QStringList() << "#");
    params.insert("policy", policy);
    response = injectAndWait("Configuration.SetMqttPolicy", params);
    responseParams = response.toMap().value("params").toMap();
    QCOMPARE(responseParams.value("configurationError").toString(), QString("ConfigurationErrorNoError"));

    // Get polcies, there should be one now
    response = injectAndWait("Configuration.GetMqttPolicies");
    responseParams = response.toMap().value("params").toMap();
    QCOMPARE(responseParams.value("mqttPolicies").toList().count(), 1);

    // Connect and fail
    connectedSpy.clear();
    disconnectedSpy.clear();
    mqttClient->connectToHost("127.0.0.1", 1883);
    QVERIFY2(connectedSpy.wait(), "Connection failed");
    QCOMPARE(connectedSpy.first().at(0).value<Mqtt::ConnectReturnCode>(), Mqtt::ConnectReturnCodeBadUsernameOrPassword);
    mqttClient->disconnectFromHost();

    // add another policy
    params.clear();
    policy.insert("clientId", "client2");
    params.insert("policy", policy);
    response = injectAndWait("Configuration.SetMqttPolicy", params);
    responseParams = response.toMap().value("params").toMap();
    QCOMPARE(responseParams.value("configurationError").toString(), QString("ConfigurationErrorNoError"));

    // Get polcies, there should be 2 now
    response = injectAndWait("Configuration.GetMqttPolicies");
    responseParams = response.toMap().value("params").toMap();
    QCOMPARE(responseParams.value("mqttPolicies").toList().count(), 2);

    // Delete policy 2
    params.clear();
    params.insert("clientId", "client2");
    response = injectAndWait("Configuration.DeleteMqttPolicy", params);
    QCOMPARE(response.toMap().value("params").toMap().value("configurationError").toString(), QString("ConfigurationErrorNoError"));

    // Get polcies, there should be 1 now
    response = injectAndWait("Configuration.GetMqttPolicies");
    responseParams = response.toMap().value("params").toMap();
    QCOMPARE(responseParams.value("mqttPolicies").toList().count(), 1);

    // Delete a non existent policy
    params.clear();
    params.insert("clientId", "client5");
    response = injectAndWait("Configuration.DeleteMqttPolicy", params);
    QCOMPARE(response.toMap().value("params").toMap().value("configurationError").toString(), QString("ConfigurationErrorInvalidId"));

    // Get polcies, there should be 1 now
    response = injectAndWait("Configuration.GetMqttPolicies");
    responseParams = response.toMap().value("params").toMap();
    QCOMPARE(responseParams.value("mqttPolicies").toList().count(), 1);

    // Delete policy 1
    params.clear();
    params.insert("clientId", "client1");
    response = injectAndWait("Configuration.DeleteMqttPolicy", params);
    QCOMPARE(response.toMap().value("params").toMap().value("configurationError").toString(), QString("ConfigurationErrorNoError"));

    // Get polcies, there should be 0 now
    response = injectAndWait("Configuration.GetMqttPolicies");
    responseParams = response.toMap().value("params").toMap();
    QCOMPARE(responseParams.value("mqttPolicies").toList().count(), 0);

}

void TestMqttBroker::testConnectAuthentication_data()
{
    QTest::addColumn<MqttPolicy>("policy");
    QTest::addColumn<Mqtt::ConnectReturnCode>("connectReturnCode");

    MqttPolicy policy;
    policy.clientId = "aaa";
    policy.username = "bbb";
    policy.password = "ccc";

    QTest::newRow("no policy for client") << policy << Mqtt::ConnectReturnCodeIdentifierRejected;

    policy.clientId = "testclient";
    QTest::newRow("user mismatch") << policy << Mqtt::ConnectReturnCodeBadUsernameOrPassword;

    policy.username = "testuser";
    QTest::newRow("pass mismatch") << policy << Mqtt::ConnectReturnCodeBadUsernameOrPassword;

    policy.password = "testpassword";
    QTest::newRow("user mismatch") << policy << Mqtt::ConnectReturnCodeAccepted;

}

void TestMqttBroker::testConnectAuthentication()
{
    QFETCH(MqttPolicy, policy);
    QFETCH(Mqtt::ConnectReturnCode, connectReturnCode);

    NymeaCore::instance()->configuration()->updateMqttPolicy(policy);

    // Connect
    MqttClient* mqttClient = new MqttClient("testclient", this);
    mqttClient->setUsername("testuser");
    mqttClient->setPassword("testpassword");
    mqttClient->setAutoReconnect(false);
    QSignalSpy connectedSpy(mqttClient, &MqttClient::connected);
    QSignalSpy disconnectedSpy(mqttClient, &MqttClient::disconnected);
    mqttClient->connectToHost("127.0.0.1", 1883);
    if (connectedSpy.count() == 0) {
        connectedSpy.wait();
    }
    QVERIFY2(connectedSpy.count() == 1, "Mqtt client didn't connect");
    QCOMPARE(connectedSpy.first().at(0).value<Mqtt::ConnectReturnCode>(), connectReturnCode);
    if (connectReturnCode != Mqtt::ConnectReturnCodeAccepted) {
        QVERIFY2(disconnectedSpy.count() == 1 || disconnectedSpy.wait(), "Client did get disconnected");
    }

    mqttClient->deleteLater();
}

void TestMqttBroker::testPublishPolicy_data()
{
    QTest::addColumn<QStringList>("allowedPublishTopicFilters");
    QTest::addColumn<QString>("publishTopic");
    QTest::addColumn<bool>("allowed");

    QTest::newRow("#, /") << (QStringList() << "#") << "/" << true;
    QTest::newRow("a, b") << (QStringList() << "a") << "b" << false;
    QTest::newRow("a b, b") << (QStringList() << "a" << "b") << "b" << true;
    QTest::newRow("/a/#, /a/b/c") << (QStringList() << "/a/#") << "/a/b/c" << true;
    QTest::newRow("/a/#, /b/a/c") << (QStringList() << "/a/#") << "/b/a/c" << false;
    QTest::newRow("/+/b/#, /a/b") << (QStringList() << "/+/b/#") << "/a/b" << true;
    QTest::newRow("/+/b/#, /b") << (QStringList() << "/+/b/#") << "/b" << false;
}

void TestMqttBroker::testPublishPolicy()
{
    QFETCH(QStringList, allowedPublishTopicFilters);
    QFETCH(QString, publishTopic);
    QFETCH(bool, allowed);

    MqttPolicy policy;
    policy.clientId = "testclient";
    policy.username = "testuser";
    policy.password = "testpassword";
    policy.allowedPublishTopicFilters = allowedPublishTopicFilters;

    NymeaCore::instance()->configuration()->updateMqttPolicy(policy);

    QSignalSpy publishReceivedSpy(NymeaCore::instance()->serverManager()->mqttBroker(), &MqttBroker::publishReceived);

    MqttClient* mqttClient = new MqttClient("testclient", this);
    mqttClient->setUsername("testuser");
    mqttClient->setPassword("testpassword");
    mqttClient->setAutoReconnect(false);
    QSignalSpy connectedSpy(mqttClient, &MqttClient::connected);
    QSignalSpy disconnectedSpy(mqttClient, &MqttClient::disconnected);
    mqttClient->connectToHost("127.0.0.1", 1883);
    QVERIFY2(connectedSpy.count() == 1 || connectedSpy.wait(), "Mqtt client didn't connect");

    mqttClient->publish(publishTopic, "Hello nymea");

    publishReceivedSpy.wait(400);
    QCOMPARE(publishReceivedSpy.count(), (allowed ? 1 : 0));
}

void TestMqttBroker::testSubscribePolicy_data()
{
    QTest::addColumn<QStringList>("allowedSubscribeTopicFilters");
    QTest::addColumn<QString>("subscribeFilter");
    QTest::addColumn<bool>("allowed");

    QTest::newRow("#, #") << (QStringList() << "#") << "#" << true;
    QTest::newRow("#, a") << (QStringList() << "#") << "a" << true;
    QTest::newRow("a, a") << (QStringList() << "a") << "a" << true;
    QTest::newRow("a, b") << (QStringList() << "a") << "b" << false;
    QTest::newRow("a b, b") << (QStringList() << "a" << "b") << "b" << true;
    QTest::newRow("/a/#, /a/b/c") << (QStringList() << "/a/#") << "/a/b/c" << true;
    QTest::newRow("/a/#, /b/a/c") << (QStringList() << "/a/#") << "/b/a/c" << false;
    QTest::newRow("/+/b/#, /a/b") << (QStringList() << "/+/b/#") << "/a/b" << true;
    QTest::newRow("/+/b/#, /b") << (QStringList() << "/+/b/#") << "/b" << false;

}

void TestMqttBroker::testSubscribePolicy()
{
    QFETCH(QStringList, allowedSubscribeTopicFilters);
    QFETCH(QString, subscribeFilter);
    QFETCH(bool, allowed);

    MqttPolicy policy;
    policy.clientId = "testclient";
    policy.username = "testuser";
    policy.password = "testpassword";
    policy.allowedSubscribeTopicFilters = allowedSubscribeTopicFilters;

    NymeaCore::instance()->configuration()->updateMqttPolicy(policy);

    QSignalSpy clientSubscribedSpy(NymeaCore::instance()->serverManager()->mqttBroker(), &MqttBroker::clientSubscribed);

    MqttClient* mqttClient = new MqttClient("testclient", this);
    mqttClient->setUsername("testuser");
    mqttClient->setPassword("testpassword");
    mqttClient->setAutoReconnect(false);
    QSignalSpy connectedSpy(mqttClient, &MqttClient::connected);
    QSignalSpy disconnectedSpy(mqttClient, &MqttClient::disconnected);
    mqttClient->connectToHost("127.0.0.1", 1883);
    QVERIFY2(connectedSpy.count() == 1 || connectedSpy.wait(), "Mqtt client didn't connect");

    mqttClient->subscribe(subscribeFilter);

    clientSubscribedSpy.wait(400);
    QCOMPARE(clientSubscribedSpy.count(), (allowed ? 1 : 0));
}


#include "testmqttbroker.moc"
QTEST_MAIN(TestMqttBroker)
