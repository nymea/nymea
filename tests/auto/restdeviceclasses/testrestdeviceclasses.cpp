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
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // Get all devices
    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:3000/api/v1/deviceclasses"));
    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantList deviceClassesList = jsonDoc.toVariant().toList();
    QVERIFY2(deviceClassesList.count() >= 1, "not enought deviceclasses.");

    // Get each of thouse devices individualy
    foreach (const QVariant &deviceClass, deviceClassesList) {
        QVariantMap deviceClassMap = deviceClass.toMap();
        QNetworkRequest request;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
        request.setUrl(QUrl(QString("http://localhost:3000/api/v1/deviceclasses/%1").arg(deviceClassMap.value("id").toString())));
        clientSpy.clear();
        QNetworkReply *reply = nam->get(request);
        clientSpy.wait();
        QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
        jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
        QCOMPARE(error.error, QJsonParseError::NoError);
        reply->deleteLater();
    }
    nam->deleteLater();
}

void TestRestDeviceClasses::getActionTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<ActionTypeId>("actionTypeId");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("all ActionTypes") << mockDeviceClassId << ActionTypeId() << 200;
    QTest::newRow("ActionType async") << mockDeviceClassId << mockActionIdAsync << 200;
    QTest::newRow("ActionType no params") << mockDeviceClassId << mockActionIdNoParams << 200;
    QTest::newRow("ActionType failing") << mockDeviceClassId << mockActionIdFailing << 200;
    QTest::newRow("ActionType with params") << mockDeviceClassId << mockActionIdWithParams << 200;
    QTest::newRow("invalid DeviceClassId") << DeviceClassId::createDeviceClassId() << mockActionIdNoParams << 404;
    QTest::newRow("invalid ActionTypeId") << mockDeviceClassId << ActionTypeId::createActionTypeId() << 404;
}

void TestRestDeviceClasses::getActionTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(ActionTypeId, actionTypeId);
    QFETCH(int, expectedStatusCode);

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    if (!actionTypeId.isNull()) {
        request.setUrl(QUrl(QString("http://localhost:3000/api/v1/deviceclasses/%1/actiontypes/%2").arg(deviceClassId.toString()).arg(actionTypeId.toString())));
    } else {
        // Get all actiontypes
        request.setUrl(QUrl(QString("http://localhost:3000/api/v1/deviceclasses/%1/actiontypes").arg(deviceClassId.toString())));
    }

    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatusCode);
    reply->deleteLater();
}

void TestRestDeviceClasses::getStateTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("all ActionTypes") << mockDeviceClassId << StateTypeId() << 200;
    QTest::newRow("StateType bool") << mockDeviceClassId << mockBoolStateId << 200;
    QTest::newRow("StateType int") << mockDeviceClassId << mockIntStateId << 200;
    QTest::newRow("invalid DeviceClassId") << DeviceClassId::createDeviceClassId() << mockBoolStateId << 404;
    QTest::newRow("invalid StateTypeId") << mockDeviceClassId << StateTypeId::createStateTypeId() << 404;
}

void TestRestDeviceClasses::getStateTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(int, expectedStatusCode);

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    if (!stateTypeId.isNull()) {
        request.setUrl(QUrl(QString("http://localhost:3000/api/v1/deviceclasses/%1/statetypes/%2").arg(deviceClassId.toString()).arg(stateTypeId.toString())));
    } else {
        // Get all actiontypes
        request.setUrl(QUrl(QString("http://localhost:3000/api/v1/deviceclasses/%1/statetypes").arg(deviceClassId.toString())));
    }

    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatusCode);
    reply->deleteLater();
}

void TestRestDeviceClasses::getEventTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<EventTypeId>("eventTypeId");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("all ActionTypes") << mockDeviceClassId << EventTypeId() << 200;
    QTest::newRow("EventType 1") << mockDeviceClassId << mockEvent1Id << 200;
    QTest::newRow("EventType 2") << mockDeviceClassId << mockEvent2Id << 200;
    QTest::newRow("invalid DeviceClassId") << DeviceClassId::createDeviceClassId() << mockEvent2Id << 404;
    QTest::newRow("invalid EventTypeId") << mockDeviceClassId << EventTypeId::createEventTypeId() << 404;
}

void TestRestDeviceClasses::getEventTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(EventTypeId, eventTypeId);
    QFETCH(int, expectedStatusCode);

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    if (!eventTypeId.isNull()) {
        request.setUrl(QUrl(QString("http://localhost:3000/api/v1/deviceclasses/%1/eventtypes/%2").arg(deviceClassId.toString()).arg(eventTypeId.toString())));
    } else {
        // Get all actiontypes
        request.setUrl(QUrl(QString("http://localhost:3000/api/v1/deviceclasses/%1/eventtypes").arg(deviceClassId.toString())));
    }

    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatusCode);
    reply->deleteLater();
}

void TestRestDeviceClasses::discoverDevices_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<QVariantList>("discoveryParams");
    QTest::addColumn<int>("expectedStatusCode");

    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("name", "resultCount");
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QTest::newRow("valid deviceClassId without params") << mockDeviceClassId << 2 << QVariantList() << 200;
    QTest::newRow("valid deviceClassId with params") << mockDeviceClassId << 1 << discoveryParams << 200;
    QTest::newRow("invalid deviceClassId") << DeviceClassId::createDeviceClassId() << 0 << QVariantList() << 404;
}

void TestRestDeviceClasses::discoverDevices()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);
    QFETCH(QVariantList, discoveryParams);
    QFETCH(int, expectedStatusCode);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // DISCOVER
    QUrl url(QString("http://localhost:3000/api/v1/deviceclasses/%1/discover").arg(deviceClassId.toString()));

    if (!discoveryParams.isEmpty()) {
        QUrlQuery query;
        query.addQueryItem("params", QJsonDocument::fromVariant(discoveryParams).toJson(QJsonDocument::Compact));
        url.setQuery(query);
    }

    QNetworkRequest request(url);
    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatusCode);
    QByteArray data = reply->readAll();
    reply->deleteLater();

    if (expectedStatusCode != 200)
        return;

    // check response
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantList foundDevices = jsonDoc.toVariant().toList();
    QCOMPARE(foundDevices.count(), resultCount);

    // ADD the discovered device
    request.setUrl(QUrl("http://localhost:3000/api/v1/devices"));
    DeviceDescriptorId descriptorId = DeviceDescriptorId(foundDevices.first().toMap().value("id").toString());
    qDebug() << descriptorId;
    params.clear();
    params.insert("deviceClassId", deviceClassId);
    params.insert("deviceDescriptorId", descriptorId.toString());

    clientSpy.clear();
    QByteArray payload = QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact);
    reply = nam->post(request, payload);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);
    data = reply->readAll();
    qDebug() << reply->rawHeaderList();
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater();
    QCOMPARE(statusCode, expectedStatusCode);

    // REMOVE added device
    jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantMap response = jsonDoc.toVariant().toMap();

    DeviceId deviceId = DeviceId(response.value("deviceId").toString());

    request = QNetworkRequest(QUrl(QString("http://localhost:3000/api/v1/devices/%1").arg(deviceId.toString())));
    clientSpy.clear();
    reply = nam->deleteResource(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater();
    QCOMPARE(statusCode, expectedStatusCode);
    nam->deleteLater();
}

QTEST_MAIN(TestRestDeviceClasses)
