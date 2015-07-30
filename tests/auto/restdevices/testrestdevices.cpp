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

class TestRestDevices: public GuhTestBase
{
    Q_OBJECT

private slots:
    void getConfiguredDevices();

    void addConfiguredDevice_data();
    void addConfiguredDevice();

    void executeAction_data();
    void executeAction();

    void getStateValue_data();
    void getStateValue();

    void editDevices_data();
    void editDevices();

    void editByDiscovery_data();
    void editByDiscovery();

private:
    // for debugging
    void printResponse(QNetworkReply *reply, const QByteArray &data);

};

void TestRestDevices::getConfiguredDevices()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // Get all devices
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    request.setUrl(QUrl("http://localhost:3000/api/v1/devices"));
    QNetworkReply *reply;

    reply = nam->get(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantList deviceList = jsonDoc.toVariant().toList();
    QVERIFY2(deviceList.count() >= 2, "not enought devices.");

    // Get each of thouse devices individualy
    foreach (const QVariant &device, deviceList) {
        QVariantMap deviceMap = device.toMap();
        QNetworkRequest request;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
        request.setUrl(QUrl(QString("http://localhost:3000/api/v1/devices/%1").arg(deviceMap.value("id").toString())));
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

void TestRestDevices::addConfiguredDevice_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<QVariantList>("deviceParams");
    QTest::addColumn<int>("expectedStatusCode");

    QVariantMap nameParam;
    nameParam.insert("name", "name");
    nameParam.insert("value", "Test Mockdevice");
    QVariantMap httpportParam;
    httpportParam.insert("name", "httpport");
    httpportParam.insert("value", m_mockDevice1Port - 1);
    QVariantMap asyncParam;
    asyncParam.insert("name", "async");
    asyncParam.insert("value", true);
    QVariantMap notAsyncParam;
    notAsyncParam.insert("name", "async");
    notAsyncParam.insert("value", false);
    QVariantMap notBrokenParam;
    notBrokenParam.insert("name", "broken");
    notBrokenParam.insert("value", false);
    QVariantMap brokenParam;
    brokenParam.insert("name", "broken");
    brokenParam.insert("value", true);

    QVariantList deviceParams;

    deviceParams.clear(); deviceParams << nameParam << httpportParam << notAsyncParam << notBrokenParam;
    QTest::newRow("User, JustAdd") << mockDeviceClassId << deviceParams << 200;

    deviceParams.clear(); deviceParams << nameParam << httpportParam << asyncParam << notBrokenParam;
    QTest::newRow("User, JustAdd, Async") << mockDeviceClassId << deviceParams << 200;

    QTest::newRow("Invalid DeviceClassId") << DeviceClassId::createDeviceClassId() << deviceParams << 500;

    deviceParams.clear(); deviceParams << nameParam << httpportParam << brokenParam;
    QTest::newRow("Setup failure") << mockDeviceClassId << deviceParams << 500;

    deviceParams.clear(); deviceParams << nameParam << httpportParam << asyncParam << brokenParam;
    QTest::newRow("Setup failure, Async") << mockDeviceClassId << deviceParams << 500;

    QVariantList invalidDeviceParams;
    QTest::newRow("User, JustAdd, missing params") << mockDeviceClassId << invalidDeviceParams << 500;

    QVariantMap fakeparam;
    fakeparam.insert("name", "tropptth");
    invalidDeviceParams.append(fakeparam);
    QTest::newRow("User, JustAdd, invalid param") << mockDeviceClassId << invalidDeviceParams << 500;

    fakeparam.insert("value", "buhuu");
    invalidDeviceParams.clear();
    invalidDeviceParams.append(fakeparam);
    QTest::newRow("User, JustAdd, wrong param") << mockDeviceClassId << invalidDeviceParams << 500;
}

void TestRestDevices::addConfiguredDevice()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(QVariantList, deviceParams);
    QFETCH(int, expectedStatusCode);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("deviceParams", deviceParams);

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // Get all devices
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setUrl(QUrl("http://localhost:3000/api/v1/devices"));

    QByteArray payload = QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact);
    qDebug() << "sending" << payload;

    QNetworkReply *reply = nam->post(request, payload);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);

    QByteArray data = reply->readAll();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatusCode);

    reply->deleteLater();

    if (expectedStatusCode == 200) {
        // remove added device
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
        QCOMPARE(error.error, QJsonParseError::NoError);
        QVariantMap response = jsonDoc.toVariant().toMap();

        DeviceId deviceId = DeviceId(response.value("deviceId").toString());

        request.setUrl(QUrl(QString("http://localhost:3000/api/v1/devices/%1").arg(deviceId.toString())));
        clientSpy.clear();
        reply = nam->deleteResource(request);
        clientSpy.wait();
        QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
        statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        reply->deleteLater();
        QCOMPARE(statusCode, 200);
    }
    nam->deleteLater();
}

void TestRestDevices::executeAction_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<ActionTypeId>("actionTypeId");
    QTest::addColumn<QVariantList>("actionParams");
    QTest::addColumn<int>("expectedStatusCode");

    QVariantList params;
    QVariantMap param1;
    param1.insert("name", "mockActionParam1");
    param1.insert("value", 5);
    params.append(param1);
    QVariantMap param2;
    param2.insert("name", "mockActionParam2");
    param2.insert("value", true);
    params.append(param2);

    QTest::newRow("valid action") << m_mockDeviceId << mockActionIdWithParams << params << 200;
    QTest::newRow("invalid deviceId") << DeviceId::createDeviceId() << mockActionIdWithParams << params << 404;
    QTest::newRow("invalid actionTypeId") << m_mockDeviceId << ActionTypeId::createActionTypeId() << params << 404;
    QTest::newRow("missing params") << m_mockDeviceId << mockActionIdWithParams << QVariantList() << 500;
    QTest::newRow("async action") << m_mockDeviceId << mockActionIdAsync << QVariantList() << 200;
    QTest::newRow("broken action") << m_mockDeviceId << mockActionIdFailing << QVariantList() << 500;
    QTest::newRow("async broken action") << m_mockDeviceId << mockActionIdAsyncFailing << QVariantList() << 500;
}

void TestRestDevices::executeAction()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(ActionTypeId, actionTypeId);
    QFETCH(QVariantList, actionParams);
    QFETCH(int, expectedStatusCode);

    // execute action
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    QVariantMap payloadMap;
    payloadMap.insert("params", actionParams);

    QNetworkRequest request(QUrl(QString("http://localhost:3000/api/v1/devices/%1/execute/%2").arg(deviceId.toString()).arg(actionTypeId.toString())));
    spy.clear();
    QNetworkReply *reply = nam.post(request, QJsonDocument::fromVariant(payloadMap).toJson(QJsonDocument::Compact));
    spy.wait();
    QCOMPARE(spy.count(), 1);

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatusCode);
    reply->deleteLater();

    // Fetch action execution history from mock device
    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/actionhistory").arg(m_mockDevice1Port)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    QByteArray data = reply->readAll();
    reply->deleteLater();

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
}

void TestRestDevices::getStateValue_data()
{
    QList<Device*> devices = GuhCore::instance()->findConfiguredDevices(mockDeviceClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Device *device = devices.first();

    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("existing state") << device->id() << mockIntStateId << 200;
    QTest::newRow("all states") << device->id() << StateTypeId() << 200;
    QTest::newRow("invalid device") << DeviceId::createDeviceId() << mockIntStateId << 404;
    QTest::newRow("invalid statetype") << device->id() << StateTypeId::createStateTypeId() << 404;
}

void TestRestDevices::getStateValue()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(int, expectedStatusCode);

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;

    if (!stateTypeId.isNull()) {
        request.setUrl(QUrl(QString("http://localhost:3000/api/v1/devices/%1/states/%2").arg(deviceId.toString()).arg(stateTypeId.toString())));
    } else {
        // Get all states
        request.setUrl(QUrl(QString("http://localhost:3000/api/v1/devices/%1/states").arg(deviceId.toString())));
    }
    qDebug() << request.url();

    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);

    QByteArray data = reply->readAll();
    qDebug() << data;

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatusCode);
    reply->deleteLater();
}

void TestRestDevices::editDevices_data()
{
    QVariantList asyncChangeDeviceParams;
    QVariantMap asyncParamDifferent;
    asyncParamDifferent.insert("name", "async");
    asyncParamDifferent.insert("value", true);
    asyncChangeDeviceParams.append(asyncParamDifferent);

    QVariantList httpportChangeDeviceParams;
    QVariantMap httpportParamDifferent;
    httpportParamDifferent.insert("name", "httpport");
    httpportParamDifferent.insert("value", 8893); // if change -> change also newPort in editDevices()
    httpportChangeDeviceParams.append(httpportParamDifferent);

    QVariantList brokenChangedDeviceParams;
    QVariantMap brokenParamDifferent;
    brokenParamDifferent.insert("name", "broken");
    brokenParamDifferent.insert("value", true);
    brokenChangedDeviceParams.append(brokenParamDifferent);

    QVariantList nameChangedDeviceParams;
    QVariantMap nameParam;
    nameParam.insert("name", "name");
    nameParam.insert("value", "Awesome Mockdevice");
    nameChangedDeviceParams.append(nameParam);


    QVariantList asyncAndPortChangeDeviceParams;
    asyncAndPortChangeDeviceParams.append(asyncParamDifferent);
    asyncAndPortChangeDeviceParams.append(httpportParamDifferent);


    QVariantList changeAllWritableDeviceParams;
    changeAllWritableDeviceParams.append(nameParam);
    changeAllWritableDeviceParams.append(asyncParamDifferent);
    changeAllWritableDeviceParams.append(httpportParamDifferent);


    QTest::addColumn<bool>("broken");
    QTest::addColumn<QVariantList>("newDeviceParams");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("valid - change async param") << false << asyncChangeDeviceParams << 200;
    QTest::newRow("valid - change httpport param") << false <<  httpportChangeDeviceParams << 200;
    QTest::newRow("valid - change httpport and async param") << false << asyncAndPortChangeDeviceParams << 200;
    QTest::newRow("invalid - change name param (not writable)") << false << nameChangedDeviceParams << 500;
    QTest::newRow("invalid - change all params (except broken)") << false << changeAllWritableDeviceParams << 500;
}

void TestRestDevices::editDevices()
{
    QFETCH(bool, broken);
    QFETCH(QVariantList, newDeviceParams);
    QFETCH(int, expectedStatusCode);

    // add device
    QVariantMap params;
    params.insert("deviceClassId", mockDeviceClassId);
    QVariantList deviceParams;
    QVariantMap nameParam;
    nameParam.insert("name", "name");
    nameParam.insert("value", "Test edit mockdevice");
    deviceParams.append(nameParam);
    QVariantMap asyncParam;
    asyncParam.insert("name", "async");
    asyncParam.insert("value", false);
    deviceParams.append(asyncParam);
    QVariantMap brokenParam;
    brokenParam.insert("name", "broken");
    brokenParam.insert("value", broken);
    deviceParams.append(brokenParam);
    QVariantMap httpportParam;
    httpportParam.insert("name", "httpport");
    httpportParam.insert("value", 8892);
    deviceParams.append(httpportParam);
    params.insert("deviceParams", deviceParams);

    // add a mockdevice
    QNetworkAccessManager *nam = new QNetworkAccessManager();
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request(QUrl(QString("http://localhost:3000/api/v1/devices")));

    QNetworkReply *reply = nam->post(request, QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact));
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);

    QByteArray data = reply->readAll();
    int statusCode;

    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 200);
    reply->deleteLater();
    QVariantMap responseMap = QJsonDocument::fromJson(data).toVariant().toMap();
    DeviceId deviceId = DeviceId(responseMap.value("deviceId").toString());
    qDebug() << deviceId.toString();
    QVERIFY2(deviceId != DeviceId(), "DeviceId not returned");


    // now EDIT the added device
    QVariantMap editParams;
    editParams.insert("deviceId", deviceId);
    editParams.insert("deviceParams", newDeviceParams);

    request.setUrl(QUrl(QString("http://localhost:3000/api/v1/devices/%1").arg(deviceId.toString())));
    clientSpy.clear();
    reply = nam->put(request, QJsonDocument::fromVariant(editParams).toJson(QJsonDocument::Compact));
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatusCode);
    reply->deleteLater();

    // if the edit should have been successfull
    if (expectedStatusCode == 200) {

        request.setUrl(QUrl(QString("http://localhost:3000/api/v1/devices/%1").arg(deviceId.toString())));
        clientSpy.clear();
        reply = nam->get(request);
        clientSpy.wait();
        QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
        statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        QVariantMap deviceMap = QJsonDocument::fromJson(reply->readAll()).toVariant().toMap();

        verifyParams(newDeviceParams, deviceMap.value("params").toList(), false);

        QCOMPARE(statusCode, 200);
        reply->deleteLater();
    }

    // delete it
    request.setUrl(QUrl(QString("http://localhost:3000/api/v1/devices/%1").arg(deviceId.toString())));
    clientSpy.clear();
    reply = nam->deleteResource(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater();
    QCOMPARE(statusCode, 200);
}

void TestRestDevices::editByDiscovery_data()
{

}

void TestRestDevices::editByDiscovery()
{

}

void TestRestDevices::printResponse(QNetworkReply *reply, const QByteArray &data)
{
    qDebug() << "-------------------------------";
    qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    foreach (const  QNetworkReply::RawHeaderPair &headerPair, reply->rawHeaderPairs()) {
        qDebug() << headerPair.first << ":" << headerPair.second;
    }
    qDebug() << "-------------------------------";
    qDebug() << data;
    qDebug() << "-------------------------------";

}

#include "testrestdevices.moc"
QTEST_MAIN(TestRestDevices)
