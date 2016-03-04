/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

using namespace guhserver;

class TestEvents: public GuhTestBase
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
    QList<Device*> devices = GuhCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Device *device = devices.first();

    QSignalSpy spy(GuhCore::instance(), SIGNAL(eventTriggered(const Event&)));

    // Setup connection to mock client
    QNetworkAccessManager nam;

    // trigger event in mock device
    int port = device->paramValue("httpport").toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(port).arg(mockEvent1Id.toString())));
    QNetworkReply *reply = nam.get(request);
    reply->deleteLater();

    // Lets wait for the notification
    spy.wait();
    QVERIFY(spy.count() > 0);
    for (int i = 0; i < spy.count(); i++ ){
        Event event = spy.at(i).at(0).value<Event>();
        if (event.deviceId() == device->id()) {
            // Make sure the event contains all the stuff we expect
            QCOMPARE(event.eventTypeId(), mockEvent1Id);
        }
    }
}

void TestEvents::triggerStateChangeEvent()
{
    QList<Device*> devices = GuhCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Device *device = devices.first();

    QSignalSpy spy(GuhCore::instance(), SIGNAL(eventTriggered(const Event&)));

    // Setup connection to mock client
    QNetworkAccessManager nam;

    // trigger state changed event in mock device
    int port = device->paramValue("httpport").toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(port).arg(mockIntStateId.toString()).arg(11)));
    QNetworkReply *reply = nam.get(request);
    reply->deleteLater();

    // Lets wait for the notification
    spy.wait();
    QVERIFY(spy.count() > 0);
    for (int i = 0; i < spy.count(); i++ ){
        Event event = spy.at(i).at(0).value<Event>();
        if (event.deviceId() == device->id()) {
            // Make sure the event contains all the stuff we expect
            QCOMPARE(event.eventTypeId().toString(), mockIntStateId.toString());
            QCOMPARE(event.param("value").value().toInt(), 11);
        }
    }
}

void TestEvents::params()
{
    Event event;
    ParamList params;
    Param p("foo", "bar");
    params.append(p);
    event.setParams(params);

    QVERIFY(event.param("foo").value().toString() == "bar");
    QVERIFY(!event.param("baz").value().isValid());
}

void TestEvents::getEventType_data()
{
    QTest::addColumn<EventTypeId>("eventTypeId");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("valid eventypeid") << mockEvent1Id << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid eventypeid") << EventTypeId::createEventTypeId() << DeviceManager::DeviceErrorEventTypeNotFound;
}

void TestEvents::getEventType()
{
    QFETCH(EventTypeId, eventTypeId);
    QFETCH(DeviceManager::DeviceError, error);

    QVariantMap params;
    params.insert("eventTypeId", eventTypeId.toString());
    QVariant response = injectAndWait("Events.GetEventType", params);

    verifyDeviceError(response, error);

    if (error == DeviceManager::DeviceErrorNoError) {
        QVERIFY2(EventTypeId(response.toMap().value("params").toMap().value("eventType").toMap().value("id").toString()) == eventTypeId, "Didnt get reply for same actionTypeId as requested.");
    }
}

#include "testevents.moc"
QTEST_MAIN(TestEvents)
