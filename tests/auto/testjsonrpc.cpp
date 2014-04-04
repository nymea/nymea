/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#include "guhcore.h"
#include "devicemanager.h"
#include "mocktcpserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

Q_IMPORT_PLUGIN(DevicePluginMock)

int mockDevice1Port = 1337;
int mockDevice2Port = 7331;

class TestJSONRPC: public QObject
{
    Q_OBJECT
private slots:
    void initTestcase();
    void cleanupTestCase();

    void introspect();
    void version();

    void getSupportedDevices();

    void enableDisableNotifications_data();
    void enableDisableNotifications();

    void stateChangeEmitsNotifications();


private:
    QVariant injectAndWait(const QString &method, const QVariantMap &params);

private:
    MockTcpServer *m_mockTcpServer;
    QUuid m_clientId;
    int m_commandId;
    QUuid m_mockDeviceId;
};

void TestJSONRPC::initTestcase()
{
    QCoreApplication::instance()->setOrganizationName("guh-test");
    m_commandId = 0;

    GuhCore::instance();

    // Wait for the DeviceManager to signal that it has loaded plugins and everything
    QSignalSpy spy(GuhCore::instance()->deviceManager(), SIGNAL(loaded()));
    QVERIFY(spy.isValid());
    QVERIFY(spy.wait());

    // If Guh should create more than one TcpServer at some point, this needs to be updated.
    QCOMPARE(MockTcpServer::servers().count(), 1);
    m_mockTcpServer = MockTcpServer::servers().first();
    m_clientId = QUuid::createUuid();

    // Lets add one instance of the mockdevice
    QVariantMap params;
    params.insert("deviceClassId", "{753f0d32-0468-4d08-82ed-1964aab03298}");
    QVariantMap deviceParams;
    deviceParams.insert("httpport", mockDevice1Port);
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);

    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);

    m_mockDeviceId = response.toMap().value("params").toMap().value("deviceId").toUuid();
    QVERIFY2(!m_mockDeviceId.isNull(), "Newly created mock device must not be null.");
}

void TestJSONRPC::cleanupTestCase()
{
    QSettings settings;
    settings.clear();
}

QVariant TestJSONRPC::injectAndWait(const QString &method, const QVariantMap &params = QVariantMap())
{
    QVariantMap call;
    call.insert("id", m_commandId++);
    call.insert("method", method);
    call.insert("params", params);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(call);
    QSignalSpy spy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    m_mockTcpServer->injectData(m_clientId, jsonDoc.toJson());

    if (spy.count() == 0) {
        spy.wait();
    }

     // Make sure the response it a valid JSON string
     QJsonParseError error;
     jsonDoc = QJsonDocument::fromJson(spy.takeFirst().last().toByteArray(), &error);

     return jsonDoc.toVariant();
}

void TestJSONRPC::introspect()
{
    QSignalSpy spy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));
    QVERIFY(spy.isValid());

    m_mockTcpServer->injectData(m_clientId, "{\"id\":42, \"method\":\"JSONRPC.Introspect\"}");

    if (spy.count() == 0) {
        spy.wait();
    }

    // Make sure we got exactly one response
    QVERIFY(spy.count() == 1);

    // Make sure the introspect response goes to the correct clientId
    QCOMPARE(spy.first().first().toString(), m_clientId.toString());

    // Make sure the response it a valid JSON string
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.first().last().toByteArray(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);

    // Make sure the response\"s id is the same as our command
    QCOMPARE(jsonDoc.toVariant().toMap().value("id").toInt(), 42);
}

void TestJSONRPC::getSupportedDevices()
{
    QVariant supportedDevices = injectAndWait("Devices.GetSupportedDevices");

    // Make sure there is exactly 1 supported device class with the name Mock Wifi Device
    QCOMPARE(supportedDevices.toMap().value("params").toMap().value("deviceClasses").toList().count(), 1);
    QString deviceName = supportedDevices.toMap().value("params").toMap().value("deviceClasses").toList().first().toMap().value("name").toString();
    QCOMPARE(deviceName, QString("Mock Device"));
}

void TestJSONRPC::enableDisableNotifications_data()
{
    QTest::addColumn<QString>("enabled");

    QTest::newRow("enabled") << "true";
    QTest::newRow("disabled") << "false";
}

void TestJSONRPC::enableDisableNotifications()
{
    QFETCH(QString, enabled);

    QVariantMap params;
    params.insert("enabled", enabled);
    QVariant response = injectAndWait("JSONRPC.SetNotificationStatus", params);

    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);
    QCOMPARE(response.toMap().value("params").toMap().value("enabled").toString(), enabled);

}

void TestJSONRPC::version()
{
    QVariant response = injectAndWait("JSONRPC.Version");

    QCOMPARE(response.toMap().value("params").toMap().value("version").toString(), QString("0.0.0"));
}

void TestJSONRPC::stateChangeEmitsNotifications()
{
    QVariantMap params;
    params.insert("enabled", true);
    QVariant response = injectAndWait("JSONRPC.SetNotificationStatus", params);
    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);

    // Setup connection to mock client
    QNetworkAccessManager nam;

    QSignalSpy mockSpy(&nam, SIGNAL(finished()));
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));


    // trigger state change in mock device
    int newVal = 1111;
    QUuid stateTypeId("80baec19-54de-4948-ac46-31eabfaceb83");
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(mockDevice1Port).arg(stateTypeId.toString()).arg(newVal)));
    QNetworkReply *reply = nam.get(request);
    reply->deleteLater();

    // Lets wait for the notification
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);

    // Make sure the notification contains all the stuff we expect
    QJsonDocument jsonDoc = QJsonDocument::fromJson(clientSpy.at(0).at(1).toByteArray());
    QCOMPARE(jsonDoc.toVariant().toMap().value("notification").toString(), QString("Devices.StateChanged"));
    QCOMPARE(jsonDoc.toVariant().toMap().value("params").toMap().value("stateTypeId").toUuid(), stateTypeId);
    QCOMPARE(jsonDoc.toVariant().toMap().value("params").toMap().value("value").toInt(), newVal);

    // Now turn off notifications
    params.clear();
    params.insert("enabled", false);
    response = injectAndWait("JSONRPC.SetNotificationStatus", params);
    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);

    // Fire the a statechange once again
    clientSpy.clear();
    newVal = 42;
    request.setUrl(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(mockDevice1Port).arg(stateTypeId.toString()).arg(newVal)));
    reply = nam.get(request);
    reply->deleteLater();

    // Lets wait a max of 100ms for the notification
    clientSpy.wait(100);
    // but make sure it doesn't come
    QCOMPARE(clientSpy.count(), 0);

    // Now check that the state has indeed changed even though we didn't get a notification
    params.clear();
    params.insert("deviceId", m_mockDeviceId);
    params.insert("stateTypeId", stateTypeId);
    response = injectAndWait("Devices.GetStateValue", params);

    qDebug() << "response" << response;
    QCOMPARE(response.toMap().value("params").toMap().value("value").toInt(), newVal);

}

QTEST_MAIN(TestJSONRPC)
#include "testjsonrpc.moc"
