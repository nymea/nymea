/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 **
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeatestbase.h"
#include "nymeacore.h"

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
    QList<Device*> devices = NymeaCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Device *device = devices.first();

    QSignalSpy spy(NymeaCore::instance(), SIGNAL(eventTriggered(const Event&)));

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
}

void TestEvents::triggerStateChangeEvent()
{
    QList<Device*> devices = NymeaCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Device *device = devices.first();

    QSignalSpy spy(NymeaCore::instance(), SIGNAL(eventTriggered(const Event&)));

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

    verifyDeviceError(response, error);

    if (error == Device::DeviceErrorNoError) {
        QVERIFY2(EventTypeId(response.toMap().value("params").toMap().value("eventType").toMap().value("id").toString()) == eventTypeId, "Didn't get a reply for the same actionTypeId as requested.");
    }
}

#include "testevents.moc"
QTEST_MAIN(TestEvents)
