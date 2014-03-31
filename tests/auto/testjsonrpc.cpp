#include "guhcore.h"
#include "devicemanager.h"
#include "mocktcpserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
//#include <QSignalSpy>

Q_IMPORT_PLUGIN(DevicePluginMockDevice)

class TestJSONRPC: public QObject
{
    Q_OBJECT
private slots:
    void initTestcase();

    void introspect();

    void getSupportedDevices();

    void enableDisableNotifications_data();
    void enableDisableNotifications();

    void version();

private:
    QVariant injectAndWait(const QByteArray data);

private:
    MockTcpServer *m_mockTcpServer;
    QUuid m_clientId;
};

void TestJSONRPC::initTestcase()
{
    QCoreApplication::instance()->setOrganizationName("guhyourhome-test");
    qDebug() << "creating core";
    GuhCore::instance();
    qDebug() << "creating spy";

    // Wait for the DeviceManager to signal that it has loaded plugins and everything
    QSignalSpy spy(GuhCore::instance()->deviceManager(), SIGNAL(loaded()));
    QVERIFY(spy.isValid());
    QVERIFY(spy.wait());

    // If Guh should create more than one TcpServer at some point, this needs to be updated.
    QCOMPARE(MockTcpServer::servers().count(), 1);
    m_mockTcpServer = MockTcpServer::servers().first();
    m_clientId = QUuid::createUuid();
}

QVariant TestJSONRPC::injectAndWait(const QByteArray data)
{
    QSignalSpy spy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    m_mockTcpServer->injectData(m_clientId, data);

    if (spy.count() == 0) {
        spy.wait();
    }

     // Make sure the response it a valid JSON string
     QJsonParseError error;
     QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.takeFirst().last().toByteArray(), &error);

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

    // Make sure the response's id is the same as our command
    QCOMPARE(jsonDoc.toVariant().toMap().value("id").toInt(), 42);
}

void TestJSONRPC::getSupportedDevices()
{
    QVariant supportedDevices = injectAndWait("{\"id\":1, \"method\":\"Devices.GetSupportedDevices\"}");

    // Make sure there is exactly 1 supported device class with the name Mock Wifi Device
    QCOMPARE(supportedDevices.toMap().value("params").toMap().value("deviceClasses").toList().count(), 1);
    QString deviceName = supportedDevices.toMap().value("params").toMap().value("deviceClasses").toList().first().toMap().value("name").toString();
    QCOMPARE(deviceName, QString("Mock WiFi Device"));
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

    QVariant response = injectAndWait(QString("{\"id\":1, \"method\":\"JSONRPC.SetNotificationStatus\", \"params\":{\"enabled\": " + enabled + " }}").toLatin1());

    QCOMPARE(response.toMap().value("params").toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("enabled").toString(), enabled);

}

void TestJSONRPC::version()
{
    QVariant response = injectAndWait("{\"id\":1, \"method\":\"JSONRPC.Version\"}");

    QCOMPARE(response.toMap().value("params").toMap().value("version").toString(), QString("0.0.0"));
}

QTEST_MAIN(TestJSONRPC)
#include "testjsonrpc.moc"
