/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

    void invalidMethod();

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
    QUrl url("https://localhost:3333/api/v1/deviceclasses");
    QVariant response = getAndWait(QNetworkRequest(url));
    QVariantList deviceClassesList = response.toList();
    QVERIFY2(deviceClassesList.count() > 0, "Not enought deviceclasses.");

    // Get each of thouse devices individualy
    foreach (const QVariant &deviceClass, deviceClassesList) {
        QVariantMap deviceClassMap = deviceClass.toMap();
        QNetworkRequest request;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
        request.setUrl(QUrl(QString("https://localhost:3333/api/v1/deviceclasses/%1").arg(deviceClassMap.value("id").toString())));

        response = getAndWait(request);
        QVERIFY2(!response.isNull(), "Could not get device");
    }

    // get with vendor filter
    QUrlQuery query;
    query.addQueryItem("vendorId", guhVendorId.toString());
    url.setQuery(query);

    response = getAndWait(QNetworkRequest(url));
    deviceClassesList = response.toList();
    QVERIFY2(deviceClassesList.count() > 0, "Not enought deviceclasses.");

    // get with invalid vendor filter
    query.clear();
    query.addQueryItem("vendorId", "uuid");
    url.setQuery(query);

    response = getAndWait(QNetworkRequest(url), 400);
    QCOMPARE(JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorVendorNotFound), response.toMap().value("error").toString());
}

void TestRestDeviceClasses::invalidMethod()
{
    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply *reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333/api/v1/deviceclasses/"));
    QNetworkReply *reply = nam.post(request, QByteArray());

    clientSpy.wait();
    QVERIFY2(clientSpy.count() != 0, "expected at least 1 response from webserver");

    bool ok = false;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, 400);

    reply->deleteLater();
}

void TestRestDeviceClasses::getActionTypes_data()
{
    QTest::addColumn<QString>("deviceClassId");
    QTest::addColumn<QString>("actionTypeId");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("all ActionTypes") << mockDeviceClassId.toString() << QString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("ActionType async") << mockDeviceClassId.toString() << mockActionIdAsync.toString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("ActionType no params") << mockDeviceClassId.toString() << mockActionIdNoParams.toString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("ActionType failing") << mockDeviceClassId.toString() << mockActionIdFailing.toString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("ActionType with params") << mockDeviceClassId.toString() << mockActionIdWithParams.toString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid DeviceClassId") << DeviceClassId::createDeviceClassId().toString() << mockActionIdNoParams.toString() << 404 << DeviceManager::DeviceErrorDeviceClassNotFound;
    QTest::newRow("invalid ActionTypeId") << mockDeviceClassId.toString() << ActionTypeId::createActionTypeId().toString() << 404 << DeviceManager::DeviceErrorActionTypeNotFound;
    QTest::newRow("invalid ActionTypeId format") << mockDeviceClassId.toString() << "uuid" << 400 << DeviceManager::DeviceErrorActionTypeNotFound;
    QTest::newRow("invalid DeviceClassId format") << "uuid" << "uuid" << 400 << DeviceManager::DeviceErrorDeviceClassNotFound;

}

void TestRestDeviceClasses::getActionTypes()
{
    QFETCH(QString, deviceClassId);
    QFETCH(QString, actionTypeId);
    QFETCH(int, expectedStatusCode);
    QFETCH(DeviceManager::DeviceError, error);

    QNetworkRequest request;
    if (!actionTypeId.isEmpty()) {
        request.setUrl(QUrl(QString("https://localhost:3333/api/v1/deviceclasses/%1/actiontypes/%2").arg(deviceClassId).arg(actionTypeId)));
    } else {
        // Get all actiontypes
        request.setUrl(QUrl(QString("https://localhost:3333/api/v1/deviceclasses/%1/actiontypes").arg(deviceClassId)));
    }

    QVariant response = getAndWait(request, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not read get action type response");
    if (expectedStatusCode != 200)
        QCOMPARE(JsonTypes::deviceErrorToString(error), response.toMap().value("error").toString());

}

void TestRestDeviceClasses::getStateTypes_data()
{
    QTest::addColumn<QString>("deviceClassId");
    QTest::addColumn<QString>("stateTypeId");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("all ActionTypes") << mockDeviceClassId.toString() << QString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("StateType bool") << mockDeviceClassId.toString() << mockBoolStateId.toString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("StateType int") << mockDeviceClassId.toString() << mockIntStateId.toString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid DeviceClassId") << DeviceClassId::createDeviceClassId().toString() << mockBoolStateId.toString() << 404 << DeviceManager::DeviceErrorDeviceClassNotFound;
    QTest::newRow("invalid StateTypeId") << mockDeviceClassId.toString() << StateTypeId::createStateTypeId().toString() << 404 << DeviceManager::DeviceErrorStateTypeNotFound;
    QTest::newRow("invalid StateTypeId format") << mockDeviceClassId.toString() << "uuid" << 400 << DeviceManager::DeviceErrorStateTypeNotFound;
    QTest::newRow("invalid DeviceClassId format") << "uuid" << "uuid" << 400 << DeviceManager::DeviceErrorDeviceClassNotFound;
}

void TestRestDeviceClasses::getStateTypes()
{
    QFETCH(QString, deviceClassId);
    QFETCH(QString, stateTypeId);
    QFETCH(int, expectedStatusCode);
    QFETCH(DeviceManager::DeviceError, error);

    QNetworkRequest request;
    if (!stateTypeId.isEmpty()) {
        request.setUrl(QUrl(QString("https://localhost:3333/api/v1/deviceclasses/%1/statetypes/%2").arg(deviceClassId).arg(stateTypeId)));
    } else {
        // Get all actiontypes
        request.setUrl(QUrl(QString("https://localhost:3333/api/v1/deviceclasses/%1/statetypes").arg(deviceClassId)));
    }

    QVariant response = getAndWait(request, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not read get action type response");
    if (expectedStatusCode != 200)
        QCOMPARE(JsonTypes::deviceErrorToString(error), response.toMap().value("error").toString());

}

void TestRestDeviceClasses::getEventTypes_data()
{
    QTest::addColumn<QString>("deviceClassId");
    QTest::addColumn<QString>("eventTypeId");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("all ActionTypes") << mockDeviceClassId.toString() << QString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("EventType 1") << mockDeviceClassId.toString() << mockEvent1Id.toString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("EventType 2") << mockDeviceClassId.toString() << mockEvent2Id.toString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid DeviceClassId") << DeviceClassId::createDeviceClassId().toString() << mockEvent2Id.toString() << 404 << DeviceManager::DeviceErrorDeviceClassNotFound;
    QTest::newRow("invalid EventTypeId") << mockDeviceClassId.toString() << EventTypeId::createEventTypeId().toString() << 404 << DeviceManager::DeviceErrorEventTypeNotFound;
    QTest::newRow("invalid EventTypeId format") << mockDeviceClassId.toString() << "uuid" << 400 << DeviceManager::DeviceErrorEventTypeNotFound;
    QTest::newRow("invalid DeviceClassId format") << "uuid" << "uuid" << 400 << DeviceManager::DeviceErrorDeviceClassNotFound;
}

void TestRestDeviceClasses::getEventTypes()
{
    QFETCH(QString, deviceClassId);
    QFETCH(QString, eventTypeId);
    QFETCH(int, expectedStatusCode);
    QFETCH(DeviceManager::DeviceError, error);

    QNetworkRequest request;
    if (!eventTypeId.isNull()) {
        request.setUrl(QUrl(QString("https://localhost:3333/api/v1/deviceclasses/%1/eventtypes/%2").arg(deviceClassId).arg(eventTypeId)));
    } else {
        // Get all actiontypes
        request.setUrl(QUrl(QString("https://localhost:3333/api/v1/deviceclasses/%1/eventtypes").arg(deviceClassId)));
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

    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", resultCountParamTypeId);
    resultCountParam.insert("value", 1);

    QVariantMap invalidResultCountParam;
    invalidResultCountParam.insert("paramTypeId", resultCountParamTypeId);
    invalidResultCountParam.insert("value", 10);

    QVariantList discoveryParams;
    discoveryParams.append(resultCountParam);

    QVariantList invalidDiscoveryParams;
    invalidDiscoveryParams.append(invalidResultCountParam);

    QTest::newRow("valid deviceClassId without params") << mockDeviceClassId << 2 << QVariantList() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("valid deviceClassId with params") << mockDeviceClassId << 1 << discoveryParams << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid deviceClassId") << DeviceClassId::createDeviceClassId() << 0 << QVariantList() << 404 << DeviceManager::DeviceErrorDeviceClassNotFound;
    QTest::newRow("valid deviceClassId with invalid params") << mockDeviceClassId << 10 << invalidDiscoveryParams << 500 << DeviceManager::DeviceErrorInvalidParameter;
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
    QUrl url(QString("https://localhost:3333/api/v1/deviceclasses/%1/discover").arg(deviceClassId.toString()));
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
    QNetworkRequest request(QUrl("https://localhost:3333/api/v1/devices"));
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
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(deviceId.toString())));
    response = deleteAndWait(request);
    QVERIFY2(!response.isNull(), "Could not delete device");
}

QTEST_MAIN(TestRestDeviceClasses)
