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

#include "servers/mocktcpserver.h"

using namespace nymeaserver;

class TestEvents: public NymeaTestBase
{
    Q_OBJECT

private slots:
    void triggerEvent();
    void triggerStateChangeEvent();

    void params();

    void getEventType_data();
    void getEventType();
};

void TestEvents::triggerEvent()
{
    enableNotifications({"Events"});

    QList<Device*> devices = NymeaCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Device *device = devices.first();

    QSignalSpy spy(NymeaCore::instance(), SIGNAL(eventTriggered(const Event&)));
    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Setup connection to mock client
    QNetworkAccessManager nam;

    // trigger event in mock device
    int port = device->paramValue(mockDeviceHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(port).arg(mockEvent1EventTypeId.toString())));
    QNetworkReply *reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);

    // Lets wait for the notification
    spy.wait();
    QVERIFY(spy.count() > 0);
    for (int i = 0; i < spy.count(); i++ ){
        Event event = spy.at(i).at(0).value<Event>();
        if (event.deviceId() == device->id()) {
            // Make sure the event contains all the stuff we expect
            QCOMPARE(event.eventTypeId(), mockEvent1EventTypeId);
        }
    }

    // Check for the notification on JSON API
    QVariantList notifications;
    notifications = checkNotifications(notificationSpy, "Events.EventTriggered");
    QVERIFY2(notifications.count() == 1, "Should get Events.EventTriggered notification");
    QVERIFY2(notifications.first().toMap().contains("deprecationWarning"), "Deprecation warning not included in notification");

    QVariantMap notificationContent = notifications.first().toMap().value("params").toMap();
    QCOMPARE(notificationContent.value("event").toMap().value("deviceId").toUuid().toString(), device->id().toString());
    QCOMPARE(notificationContent.value("event").toMap().value("eventTypeId").toUuid().toString(), mockEvent1EventTypeId.toString());
}

void TestEvents::triggerStateChangeEvent()
{
    enableNotifications({"Events"});

    QList<Device*> devices = NymeaCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Device *device = devices.first();

    QSignalSpy spy(NymeaCore::instance(), SIGNAL(eventTriggered(const Event&)));
    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Setup connection to mock client
    QNetworkAccessManager nam;

    // trigger state changed event in mock device
    int port = device->paramValue(mockDeviceHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(port).arg(mockIntStateTypeId.toString()).arg(11)));
    QNetworkReply *reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);

    // Lets wait for the notification
    spy.wait();
    QVERIFY(spy.count() > 0);
    for (int i = 0; i < spy.count(); i++ ){
        Event event = spy.at(i).at(0).value<Event>();
        if (event.deviceId() == device->id()) {
            // Make sure the event contains all the stuff we expect
            QCOMPARE(event.eventTypeId().toString(), mockIntStateTypeId.toString());
            QCOMPARE(event.param(ParamTypeId(mockIntStateTypeId.toString())).value().toInt(), 11);
        }
    }

    // Check for the notification on JSON API
    QVariantList notifications;
    notifications = checkNotifications(notificationSpy, "Events.EventTriggered");
    QVERIFY2(notifications.count() == 1, "Should get Devices.EventTriggered notification");
    QVERIFY2(notifications.first().toMap().contains("deprecationWarning"), "Deprecation warning not included in notification!");

    QVariantMap notificationContent = notifications.first().toMap().value("params").toMap();

    QCOMPARE(notificationContent.value("event").toMap().value("deviceId").toUuid().toString(), device->id().toString());
    QCOMPARE(notificationContent.value("event").toMap().value("eventTypeId").toUuid().toString(), mockIntEventTypeId.toString());
}

void TestEvents::params()
{
    Event event;
    ParamList params;
    ParamTypeId id = ParamTypeId::createParamTypeId();
    Param p(id, "foo bar");
    params.append(p);
    event.setParams(params);

    QVERIFY(event.param(id).value().toString() == "foo bar");
    QVERIFY(!event.param(ParamTypeId::createParamTypeId()).value().isValid());
}

void TestEvents::getEventType_data()
{
    QTest::addColumn<EventTypeId>("eventTypeId");
    QTest::addColumn<Device::DeviceError>("error");

    QTest::newRow("valid eventypeid") << mockEvent1EventTypeId << Device::DeviceErrorNoError;
    QTest::newRow("invalid eventypeid") << EventTypeId::createEventTypeId() << Device::DeviceErrorEventTypeNotFound;
}

void TestEvents::getEventType()
{
    QFETCH(EventTypeId, eventTypeId);
    QFETCH(Device::DeviceError, error);

    QVariantMap params;
    params.insert("eventTypeId", eventTypeId.toString());
    QVariant response = injectAndWait("Events.GetEventType", params);

    verifyError(response, "deviceError", enumValueName(error));

    qCDebug(dcTests()) << "*content" << response;
    QVERIFY2(response.toMap().contains("deprecationWarning"), "Deprecation warning not shown in reply");

    if (error == Device::DeviceErrorNoError) {
        QVERIFY2(EventTypeId(response.toMap().value("params").toMap().value("eventType").toMap().value("id").toString()) == eventTypeId, "Didn't get a reply for the same actionTypeId as requested.");
    }
}

#include "testevents.moc"
QTEST_MAIN(TestEvents)
