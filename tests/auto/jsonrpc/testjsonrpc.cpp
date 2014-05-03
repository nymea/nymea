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

#include "guhtestbase.h"
#include "guhcore.h"
#include "devicemanager.h"
#include "mocktcpserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QMetaType>

class TestJSONRPC: public GuhTestBase
{
    Q_OBJECT

private slots:
    void testBasicCall();
    void introspect();

    void executeAction_data();
    void executeAction();

    void getActionTypes_data();
    void getActionTypes();

    void getEventTypes_data();
    void getEventTypes();

    void getStateTypes_data();
    void getStateTypes();

    void enableDisableNotifications_data();
    void enableDisableNotifications();

    void stateChangeEmitsNotifications();

    void getRules();

private:
    QStringList extractRefs(const QVariant &variant);

};

QStringList TestJSONRPC::extractRefs(const QVariant &variant)
{
    if (variant.canConvert(QVariant::String)) {
        if (variant.toString().startsWith("$ref")) {
            return QStringList() << variant.toString();
        }
    }
    if (variant.canConvert(QVariant::List)) {
        QStringList refs;
        foreach (const QVariant tmp, variant.toList()) {
            refs << extractRefs(tmp);
        }
        return refs;
    }
    if (variant.canConvert(QVariant::Map)) {
        QStringList refs;
        foreach (const QVariant tmp, variant.toMap()) {
            refs << extractRefs(tmp);
        }
        return refs;
    }
    return QStringList();
}

void TestJSONRPC::testBasicCall()
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

void TestJSONRPC::introspect()
{
    QVariant response = injectAndWait("JSONRPC.Introspect");
    QVariantMap methods = response.toMap().value("params").toMap().value("methods").toMap();
    QVariantMap notifications = response.toMap().value("params").toMap().value("notifications").toMap();
    QVariantMap types = response.toMap().value("params").toMap().value("types").toMap();

    QVERIFY2(methods.count() > 0, "No methods in Introspect response!");
    QVERIFY2(notifications.count() > 0, "No notifications in Introspect response!");
    QVERIFY2(types.count() > 0, "No types in Introspect response!");

    // Make sure all $ref: pointers have their according type defined
    QVariantMap allItems = methods.unite(notifications).unite(types);
    foreach (const QVariant &item, allItems) {
        foreach (const QString &ref, extractRefs(item)) {
            QString typeId = ref;
            typeId.remove("$ref:");
            QVERIFY2(types.contains(typeId), QString("Undefined ref: %1").arg(ref).toLatin1().data());
        }
    }
}

void TestJSONRPC::executeAction_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<ActionTypeId>("actionTypeId");
    QTest::addColumn<QVariantList>("actionParams");
    QTest::addColumn<bool>("success");

    QVariantList params;
    QVariantMap param1;
    param1.insert("mockActionParam1", 5);
    params.append(param1);
    QVariantMap param2;
    param2.insert("mockActionParam2", true);
    params.append(param2);

    QTest::newRow("valid action") << m_mockDeviceId << mockAction1Id << params << true;
    QTest::newRow("invalid device TypeId") << DeviceId("f2965936-0dd0-4014-8f31-4c2ef7fc5952") << mockAction1Id << params << false;
    QTest::newRow("invalid action TypeId") << m_mockDeviceId << ActionTypeId("f2965936-0dd0-4014-8f31-4c2ef7fc5952") << params << false;
}

void TestJSONRPC::executeAction()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(ActionTypeId, actionTypeId);
    QFETCH(QVariantList, actionParams);
    QFETCH(bool, success);

    QVariantMap params;
    params.insert("actionTypeId", actionTypeId);
    params.insert("deviceId", deviceId);
    params.insert("params", actionParams);
    QVariant response = injectAndWait("Actions.ExecuteAction", params);
    qDebug() << "executeActionresponse" << response;
    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), success);

    // Fetch action execution history from mock device
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(m_mockDevice1Port)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    if (success) {
        QCOMPARE(actionTypeId, ActionTypeId(reply->readAll()));
    } else {
        QCOMPARE(reply->readAll().length(), 0);
    }

    // cleanup for the next run
    spy.clear();
    request.setUrl(QUrl(QString("http://localhost:%1/clearactionhistory").arg(m_mockDevice1Port)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
}

void TestJSONRPC::getActionTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid deviceclass") << mockDeviceClassId << 2;
    QTest::newRow("invalid deviceclass") << DeviceClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestJSONRPC::getActionTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    QVariant response = injectAndWait("Devices.GetActionTypes", params);

    QVariantList actionTypes = response.toMap().value("params").toMap().value("actionTypes").toList();
    QCOMPARE(actionTypes.count(), resultCount);
    if (resultCount > 0) {
        QCOMPARE(actionTypes.first().toMap().value("id").toString(), mockAction1Id.toString());
    }
}

void TestJSONRPC::getEventTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid deviceclass") << mockDeviceClassId << 2;
    QTest::newRow("invalid deviceclass") << DeviceClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestJSONRPC::getEventTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    QVariant response = injectAndWait("Devices.GetEventTypes", params);

    QVariantList eventTypes = response.toMap().value("params").toMap().value("eventTypes").toList();
    QCOMPARE(eventTypes.count(), resultCount);
    if (resultCount > 0) {
        QCOMPARE(eventTypes.first().toMap().value("id").toString(), mockEvent1Id.toString());
    }
}

void TestJSONRPC::getStateTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid deviceclass") << mockDeviceClassId << 2;
    QTest::newRow("invalid deviceclass") << DeviceClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestJSONRPC::getStateTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    QVariant response = injectAndWait("Devices.GetStateTypes", params);

    QVariantList stateTypes = response.toMap().value("params").toMap().value("stateTypes").toList();
    QCOMPARE(stateTypes.count(), resultCount);
    if (resultCount > 0) {
        QCOMPARE(stateTypes.first().toMap().value("id").toString(), mockIntStateId.toString());
    }
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

void TestJSONRPC::stateChangeEmitsNotifications()
{
    QVariantMap params;
    params.insert("enabled", true);
    QVariant response = injectAndWait("JSONRPC.SetNotificationStatus", params);
    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);

    // Setup connection to mock client
    QNetworkAccessManager nam;

    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));


    // trigger state change in mock device
    int newVal = 1111;
    QUuid stateTypeId("80baec19-54de-4948-ac46-31eabfaceb83");
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockDevice1Port).arg(stateTypeId.toString()).arg(newVal)));
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
    request.setUrl(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(m_mockDevice1Port).arg(stateTypeId.toString()).arg(newVal)));
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

    QCOMPARE(response.toMap().value("params").toMap().value("value").toInt(), newVal);

}

void TestJSONRPC::getRules()
{
    QVariant response = injectAndWait("Rules.GetRules", QVariantMap());
    qDebug() << "got rules response" << response;
}

#include "testjsonrpc.moc"

QTEST_MAIN(TestJSONRPC)
