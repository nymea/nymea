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

class TestActions: public GuhTestBase
{
    Q_OBJECT

private slots:
    void executeAction_data();
    void executeAction();

    void getActionTypes_data();
    void getActionTypes();

};

void TestActions::executeAction_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<ActionTypeId>("actionTypeId");
    QTest::addColumn<QVariantList>("actionParams");
    QTest::addColumn<bool>("success");

    QVariantList params;
    QVariantMap param1;
    param1.insert("name", "mockActionParam1");
    param1.insert("value", 5);
    params.append(param1);
    QVariantMap param2;
    param2.insert("name", "mockActionParam2");
    param2.insert("value", true);
    params.append(param2);

    QTest::newRow("valid action") << m_mockDeviceId << mockActionIdWithParams << params << true;
    QTest::newRow("invalid deviceId") << DeviceId::createDeviceId() << mockActionIdWithParams << params << false;
    QTest::newRow("invalid actionTypeId") << m_mockDeviceId << ActionTypeId::createActionTypeId() << params << false;
    QTest::newRow("missing params") << m_mockDeviceId << mockActionIdWithParams << QVariantList() << false;
    QTest::newRow("async action") << m_mockDeviceId << mockActionIdAsync << QVariantList() << true;
    QTest::newRow("broken action") << m_mockDeviceId << mockActionIdFailing << QVariantList() << false;
    QTest::newRow("async broken action") << m_mockDeviceId << mockActionIdAsyncFailing << QVariantList() << false;
}

void TestActions::executeAction()
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
    verifySuccess(response, success);

    // Fetch action execution history from mock device
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(m_mockDevice1Port)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
    QByteArray data = reply->readAll();

    if (success) {
        QVERIFY2(actionTypeId == ActionTypeId(data), QString("ActionTypeId mismatch. Got %1, Expected: %2")
                 .arg(ActionTypeId(data).toString()).arg(actionTypeId.toString()).toLatin1().data());
    } else {
        QVERIFY2(data.length() == 0, QString("Data is %1, should be empty.").arg(QString(data)).toLatin1().data());
    }

    // cleanup for the next run
    spy.clear();
    request.setUrl(QUrl(QString("http://localhost:%1/clearactionhistory").arg(m_mockDevice1Port)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    spy.clear();
    request.setUrl(QUrl(QString("http://localhost:%1/actionhistory").arg(m_mockDevice1Port)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
    data = reply->readAll();
    qDebug() << "cleared data:" << data;

}

void TestActions::getActionTypes_data()
{
    QTest::addColumn<ActionTypeId>("actionTypeId");
    QTest::addColumn<bool>("success");

    QTest::newRow("valid actiontypeid") << mockActionIdWithParams << true;
    QTest::newRow("invalid actiontypeid") << ActionTypeId::createActionTypeId() << false;
}

void TestActions::getActionTypes()
{
    QFETCH(ActionTypeId, actionTypeId);
    QFETCH(bool, success);

    QVariantMap params;
    params.insert("actionTypeId", actionTypeId.toString());
    QVariant response = injectAndWait("Actions.GetActionType", params);

    verifySuccess(response, success);

    if (success) {
        QVERIFY2(ActionTypeId(response.toMap().value("params").toMap().value("actionType").toMap().value("id").toString()) == actionTypeId, "Didnt get reply for same actionTypeId as requested.");
    }
}


#include "testactions.moc"
QTEST_MAIN(TestActions)
