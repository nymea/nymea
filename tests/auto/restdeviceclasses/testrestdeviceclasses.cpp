/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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
#include "webserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QHttpPart>
#include <QMetaType>

using namespace guhserver;

class TestRestDeviceClasses: public GuhTestBase
{
    Q_OBJECT

private slots:
    void getSupportedDevices();

    void getActionTypes_data();
    void getActionTypes();

    void getStateTypes_data();
    void getStateTypes();

    void getEventTypes_data();
    void getEventTypes();

    void discoverDevices_data();
    void discoverDevices();

};

#include "testrestdeviceclasses.moc"

void TestRestDeviceClasses::getSupportedDevices()
{
    // Get all deviceclasses
    QVariant response = getAndWait(QNetworkRequest(QUrl("http://localhost:3333/api/v1/deviceclasses")));
    QVariantList deviceClassesList = response.toList();
    QVERIFY2(deviceClassesList.count() > 0, "Not enought deviceclasses.");

    // Get each of thouse devices individualy
    foreach (const QVariant &deviceClass, deviceClassesList) {
        QVariantMap deviceClassMap = deviceClass.toMap();
        QNetworkRequest request;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
        request.setUrl(QUrl(QString("http://localhost:3333/api/v1/deviceclasses/%1").arg(deviceClassMap.value("id").toString())));

        response = getAndWait(request);
        QVERIFY2(!response.isNull(), "Could not get device");
    }
}

void TestRestDeviceClasses::getActionTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<ActionTypeId>("actionTypeId");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("all ActionTypes") << mockDeviceClassId << ActionTypeId() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("ActionType async") << mockDeviceClassId << mockActionIdAsync << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("ActionType no params") << mockDeviceClassId << mockActionIdNoParams << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("ActionType failing") << mockDeviceClassId << mockActionIdFailing << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("ActionType with params") << mockDeviceClassId << mockActionIdWithParams << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid DeviceClassId") << DeviceClassId::createDeviceClassId() << mockActionIdNoParams << 404 << DeviceManager::DeviceErrorDeviceClassNotFound;
    QTest::newRow("invalid ActionTypeId") << mockDeviceClassId << ActionTypeId::createActionTypeId() << 404 << DeviceManager::DeviceErrorActionTypeNotFound;
}

void TestRestDeviceClasses::getActionTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(ActionTypeId, actionTypeId);
    QFETCH(int, expectedStatusCode);
    QFETCH(DeviceManager::DeviceError, error);

    QNetworkRequest request;
    if (!actionTypeId.isNull()) {
        request.setUrl(QUrl(QString("http://localhost:3333/api/v1/deviceclasses/%1/actiontypes/%2").arg(deviceClassId.toString()).arg(actionTypeId.toString())));
    } else {
        // Get all actiontypes
        request.setUrl(QUrl(QString("http://localhost:3333/api/v1/deviceclasses/%1/actiontypes").arg(deviceClassId.toString())));
    }

    QVariant response = getAndWait(request, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not read get action type response");
    if (expectedStatusCode != 200)
        QCOMPARE(JsonTypes::deviceErrorToString(error), response.toMap().value("error").toString());

}

void TestRestDeviceClasses::getStateTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("all ActionTypes") << mockDeviceClassId << StateTypeId() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("StateType bool") << mockDeviceClassId << mockBoolStateId << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("StateType int") << mockDeviceClassId << mockIntStateId << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid DeviceClassId") << DeviceClassId::createDeviceClassId() << mockBoolStateId << 404 << DeviceManager::DeviceErrorDeviceClassNotFound;
    QTest::newRow("invalid StateTypeId") << mockDeviceClassId << StateTypeId::createStateTypeId() << 404 << DeviceManager::DeviceErrorStateTypeNotFound;
}

void TestRestDeviceClasses::getStateTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(int, expectedStatusCode);
    QFETCH(DeviceManager::DeviceError, error);

    QNetworkRequest request;
    if (!stateTypeId.isNull()) {
        request.setUrl(QUrl(QString("http://localhost:3333/api/v1/deviceclasses/%1/statetypes/%2").arg(deviceClassId.toString()).arg(stateTypeId.toString())));
    } else {
        // Get all actiontypes
        request.setUrl(QUrl(QString("http://localhost:3333/api/v1/deviceclasses/%1/statetypes").arg(deviceClassId.toString())));
    }

    QVariant response = getAndWait(request, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not read get action type response");
    if (expectedStatusCode != 200)
        QCOMPARE(JsonTypes::deviceErrorToString(error), response.toMap().value("error").toString());

}

void TestRestDeviceClasses::getEventTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<EventTypeId>("eventTypeId");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("all ActionTypes") << mockDeviceClassId << EventTypeId() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("EventType 1") << mockDeviceClassId << mockEvent1Id << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("EventType 2") << mockDeviceClassId << mockEvent2Id << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid DeviceClassId") << DeviceClassId::createDeviceClassId() << mockEvent2Id << 404 << DeviceManager::DeviceErrorDeviceClassNotFound;
    QTest::newRow("invalid EventTypeId") << mockDeviceClassId << EventTypeId::createEventTypeId() << 404 << DeviceManager::DeviceErrorEventTypeNotFound;
}

void TestRestDeviceClasses::getEventTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(EventTypeId, eventTypeId);
    QFETCH(int, expectedStatusCode);
    QFETCH(DeviceManager::DeviceError, error);

    QNetworkRequest request;
    if (!eventTypeId.isNull()) {
        request.setUrl(QUrl(QString("http://localhost:3333/api/v1/deviceclasses/%1/eventtypes/%2").arg(deviceClassId.toString()).arg(eventTypeId.toString())));
    } else {
        // Get all actiontypes
        request.setUrl(QUrl(QString("http://localhost:3333/api/v1/deviceclasses/%1/eventtypes").arg(deviceClassId.toString())));
    }

    QVariant response = getAndWait(request, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not read get action type response");
    if (expectedStatusCode != 200)
        QCOMPARE(JsonTypes::deviceErrorToString(error), response.toMap().value("error").toString());

}

void TestRestDeviceClasses::discoverDevices_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<QVariantList>("discoveryParams");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("name", "resultCount");
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QTest::newRow("valid deviceClassId without params") << mockDeviceClassId << 2 << QVariantList() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("valid deviceClassId with params") << mockDeviceClassId << 1 << discoveryParams << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid deviceClassId") << DeviceClassId::createDeviceClassId() << 0 << QVariantList() << 404 << DeviceManager::DeviceErrorDeviceClassNotFound;
}

void TestRestDeviceClasses::discoverDevices()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);
    QFETCH(QVariantList, discoveryParams);
    QFETCH(int, expectedStatusCode);
    QFETCH(DeviceManager::DeviceError, error);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);

    // DISCOVER
    QUrl url(QString("http://localhost:3333/api/v1/deviceclasses/%1/discover").arg(deviceClassId.toString()));
    if (!discoveryParams.isEmpty()) {
        QUrlQuery query;
        query.addQueryItem("params", QJsonDocument::fromVariant(discoveryParams).toJson(QJsonDocument::Compact));
        url.setQuery(query);
    }

    QVariant response = getAndWait(QNetworkRequest(url), expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not read response");

    if (expectedStatusCode != 200) {
        QCOMPARE(JsonTypes::deviceErrorToString(error), response.toMap().value("error").toString());
        return;
    }

    // check response
    QVariantList foundDevices = response.toList();
    QCOMPARE(foundDevices.count(), resultCount);

    // ADD the discovered device
    QNetworkRequest request(QUrl("http://localhost:3333/api/v1/devices"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    DeviceDescriptorId descriptorId = DeviceDescriptorId(foundDevices.first().toMap().value("id").toString());

    params.clear();
    params.insert("deviceClassId", deviceClassId);
    params.insert("deviceDescriptorId", descriptorId.toString());

    response = postAndWait(request, params, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not read response");

    DeviceId deviceId = DeviceId(response.toMap().value("id").toString());
    QVERIFY2(!deviceId.isNull(), "got invalid device id");

    // REMOVE added device
    request.setUrl(QUrl(QString("http://localhost:3333/api/v1/devices/%1").arg(deviceId.toString())));
    response = deleteAndWait(request);
    QVERIFY2(!response.isNull(), "Could not delete device");
}

QTEST_MAIN(TestRestDeviceClasses)
