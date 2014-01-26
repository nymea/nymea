#include "hivecore.h"
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

private:
    MockTcpServer *m_mockTcpServer;
    int m_clientId;
};

void TestJSONRPC::initTestcase()
{
    QCoreApplication::instance()->setOrganizationName("hiveyourhome-test");
    qDebug() << "creating core";
    HiveCore::instance();
    qDebug() << "creating spy";

    // Wait for the DeviceManager to signal that it has loaded plugins and everything
    QSignalSpy spy(HiveCore::instance()->deviceManager(), SIGNAL(loaded()));
    QVERIFY(spy.isValid());
    QVERIFY(spy.wait());

    // If Hive should create more than one TcpServer at some point, this needs to be updated.
    QCOMPARE(MockTcpServer::servers().count(), 1);
    m_mockTcpServer = MockTcpServer::servers().first();
    m_clientId = 0;
}

void TestJSONRPC::introspect()
{
    QSignalSpy spy(m_mockTcpServer, SIGNAL(outgoingData(int,QByteArray)));
    QVERIFY(spy.isValid());

    m_mockTcpServer->injectData(m_clientId, "{\"id\":42, \"method\":\"JSONRPC.Introspect\"}");

    if (spy.count() == 0) {
        spy.wait();
    }

    // Make sure we got exactly one response
    QVERIFY(spy.count() == 1);

    // Make sure the introspect response goes to the correct clientId
    QCOMPARE(spy.first().first().toInt(), m_clientId);

    // Make sure the response it a valid JSON string
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.first().last().toByteArray(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);

    // Make sure the response's id is the same as our command
    QCOMPARE(jsonDoc.toVariant().toMap().value("id").toInt(), 42);
}

void TestJSONRPC::getSupportedDevices()
{
    QSignalSpy spy(m_mockTcpServer, SIGNAL(outgoingData(int,QByteArray)));

    m_mockTcpServer->injectData(m_clientId, "{\"id\":1, \"method\":\"Devices.GetSupportedDevices\"}");

    if (spy.count() == 0) {
        spy.wait();
    }

    // Make sure the response it a valid JSON string
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.first().last().toByteArray(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);

    QVariant supportedDevices = jsonDoc.toVariant();

    qDebug() << spy.first().last();

    // Make sure there is exactly 1 supported device class with the name Mock Wifi Device
    QCOMPARE(supportedDevices.toMap().value("params").toMap().value("deviceClasses").toList().count(), 1);
    QString deviceName = supportedDevices.toMap().value("params").toMap().value("deviceClasses").toList().first().toMap().value("name").toString();
    QCOMPARE(deviceName, QString("Mock WiFi Device"));
}

QTEST_MAIN(TestJSONRPC)
#include "testjsonrpc.moc"
