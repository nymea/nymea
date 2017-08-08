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

class TestRestDevices: public GuhTestBase
{
    Q_OBJECT

private slots:
    void getConfiguredDevices();

    void addConfiguredDevice_data();
    void addConfiguredDevice();

    void addPushButtonDevices_data();
    void addPushButtonDevices();

    void addDisplayPinDevices_data();
    void addDisplayPinDevices();

    void parentChildDevices();

    void executeAction_data();
    void executeAction();

    void getStateValue_data();
    void getStateValue();

    void editDevices_data();
    void editDevices();

    void reconfigureDevices_data();
    void reconfigureDevices();

    void reconfigureByDiscovery_data();
    void reconfigureByDiscovery();

};

void TestRestDevices::getConfiguredDevices()
{
    // Get all devices
    QVariant response = getAndWait(QNetworkRequest(QUrl("https://localhost:3333/api/v1/devices")));
    QVERIFY2(!response.isNull(), "Could not get device");
    QVariantList deviceList = response.toList();
    QVERIFY2(deviceList.count() >= 2, "not enought devices.");

    // Get each of those devices individualy
    foreach (const QVariant &device, deviceList) {
        QVariantMap deviceMap = device.toMap();
        QNetworkRequest request(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(deviceMap.value("id").toString())));
        response = getAndWait(request);
        QVERIFY2(!response.isNull(), "Could not get device");
    }
}

void TestRestDevices::addConfiguredDevice_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<QVariantList>("deviceParams");
    QTest::addColumn<int>("expectedStatusCode");

    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", httpportParamTypeId);
    httpportParam.insert("value", m_mockDevice1Port - 1);
    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", asyncParamTypeId);
    asyncParam.insert("value", true);
    QVariantMap notAsyncParam;
    notAsyncParam.insert("paramTypeId", asyncParamTypeId);
    notAsyncParam.insert("value", false);
    QVariantMap notBrokenParam;
    notBrokenParam.insert("paramTypeId", brokenParamTypeId);
    notBrokenParam.insert("value", false);
    QVariantMap brokenParam;
    brokenParam.insert("paramTypeId", brokenParamTypeId);
    brokenParam.insert("value", true);

    QVariantList deviceParams;

    deviceParams.clear(); deviceParams << httpportParam << notAsyncParam << notBrokenParam;
    QTest::newRow("User, JustAdd") << mockDeviceClassId << deviceParams << 200;

    deviceParams.clear(); deviceParams << httpportParam << asyncParam << notBrokenParam;
    QTest::newRow("User, JustAdd, Async") << mockDeviceClassId << deviceParams << 200;

    QTest::newRow("Invalid DeviceClassId") << DeviceClassId::createDeviceClassId() << deviceParams << 500;

    deviceParams.clear(); deviceParams << httpportParam << brokenParam;
    QTest::newRow("Setup failure") << mockDeviceClassId << deviceParams << 500;

    deviceParams.clear(); deviceParams << httpportParam << asyncParam << brokenParam;
    QTest::newRow("Setup failure, Async") << mockDeviceClassId << deviceParams << 500;

    QVariantList invalidDeviceParams;
    QTest::newRow("User, JustAdd, missing params") << mockDeviceClassId << invalidDeviceParams << 500;

    QVariantMap fakeparam;
    fakeparam.insert("paramTypeId", ParamTypeId::createParamTypeId());
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
    params.insert("name", "Mock device");
    params.insert("deviceParams", deviceParams);

    QNetworkRequest request(QUrl("https://localhost:3333/api/v1/devices"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QVariant response = postAndWait(request, params, expectedStatusCode);
    qDebug() << QJsonDocument::fromVariant(response).toJson();

    QVERIFY2(!response.isNull(), "Could not add device");

    if (expectedStatusCode == 200) {
        // remove added device
        DeviceId deviceId = DeviceId(response.toMap().value("id").toString());
        QVERIFY2(!deviceId.isNull(), "invalid device id for removing");

        QNetworkRequest  deleteRequest(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(deviceId.toString())));
        response = deleteAndWait(deleteRequest);
        QVERIFY2(!response.isNull(), "Could not delete device");
    }
}

void TestRestDevices::addPushButtonDevices_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<bool>("waitForButtonPressed");

    QTest::newRow("Valid: Add PushButton device") << mockPushButtonDeviceClassId << 200 << true;
    QTest::newRow("Invalid: Add PushButton device (press to early)") << mockPushButtonDeviceClassId << 500 << false;
}

void TestRestDevices::addPushButtonDevices()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, expectedStatusCode);
    QFETCH(bool, waitForButtonPressed);

    // Discover device
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", resultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    // Discover
    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);

    // create URL
    QUrl url(QString("https://localhost:3333/api/v1/deviceclasses/%1/discover").arg(deviceClassId.toString()));
    QUrlQuery query;
    query.addQueryItem("params", QJsonDocument::fromVariant(discoveryParams).toJson(QJsonDocument::Compact));
    url.setQuery(query);

    QVariant response = getAndWait(QNetworkRequest(url));
    QVariantList foundDevices = response.toList();
    QCOMPARE(foundDevices.count(), 1);

    DeviceDescriptorId deviceDescriptoId(foundDevices.first().toMap().value("id").toString());

    // Pair
    params.clear();
    params.insert("deviceClassId", deviceClassId.toString());
    params.insert("name", "Push button mock device");
    params.insert("deviceDescriptorId", deviceDescriptoId);

    QNetworkRequest pairRequest(QUrl("https://localhost:3333/api/v1/devices/pair"));
    pairRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");

    response = postAndWait(pairRequest, params);
    QVERIFY2(!response.isNull(), "Could not pair device");

    PairingTransactionId pairingTransactionId(response.toMap().value("pairingTransactionId").toString());
    QString displayMessage = response.toMap().value("displayMessage").toString();

    qDebug() << "displayMessage" << displayMessage;

    if (waitForButtonPressed)
        QTest::qWait(3500);

    // Confirm pairing
    params.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());

    QNetworkRequest confirmPairingRequest(QUrl("https://localhost:3333/api/v1/devices/confirmpairing"));
    confirmPairingRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");

    response = postAndWait(confirmPairingRequest, params, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not confirm pairing device");

    if (expectedStatusCode == 200) {
        // remove added device
        DeviceId deviceId = DeviceId(response.toMap().value("id").toString());
        QVERIFY2(!deviceId.isNull(), "invalid device id for removing");

        QNetworkRequest  deleteRequest(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(deviceId.toString())));
        response = deleteAndWait(deleteRequest);
        QVERIFY2(!response.isNull(), "Could not delete device");
    }
}

void TestRestDevices::addDisplayPinDevices_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<QString>("secret");

    QTest::newRow("Valid: Add DisplayPin device") << mockDisplayPinDeviceClassId << 200 << "243681";
    QTest::newRow("Invalid: Add DisplayPin device (wrong pin)") << mockDisplayPinDeviceClassId << 500 << "243682";
}

void TestRestDevices::addDisplayPinDevices()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, expectedStatusCode);
    QFETCH(QString, secret);

    // Discover device
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", resultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    // Discover
    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);

    // create URL
    QUrl url(QString("https://localhost:3333/api/v1/deviceclasses/%1/discover").arg(deviceClassId.toString()));
    QUrlQuery query;
    query.addQueryItem("params", QJsonDocument::fromVariant(discoveryParams).toJson(QJsonDocument::Compact));
    url.setQuery(query);

    QVariant response = getAndWait(QNetworkRequest(url));
    QVariantList foundDevices = response.toList();
    QCOMPARE(foundDevices.count(), 1);

    DeviceDescriptorId deviceDescriptoId(foundDevices.first().toMap().value("id").toString());

    // Pair
    params.clear();
    params.insert("deviceClassId", deviceClassId.toString());
    params.insert("name", "Display pin mock device");
    params.insert("deviceDescriptorId", deviceDescriptoId);

    QNetworkRequest pairRequest(QUrl("https://localhost:3333/api/v1/devices/pair"));
    pairRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");

    response = postAndWait(pairRequest, params);
    QVERIFY2(!response.isNull(), "Could not pair device");

    PairingTransactionId pairingTransactionId(response.toMap().value("pairingTransactionId").toString());
    QString displayMessage = response.toMap().value("displayMessage").toString();

    qDebug() << "displayMessage" << displayMessage;

    // Confirm pairing
    params.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());
    params.insert("secret", secret);

    QNetworkRequest confirmPairingRequest(QUrl("https://localhost:3333/api/v1/devices/confirmpairing"));
    confirmPairingRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");

    response = postAndWait(confirmPairingRequest, params, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not confirm pairing device");

    if (expectedStatusCode == 200) {
        // remove added device
        DeviceId deviceId = DeviceId(response.toMap().value("id").toString());
        QVERIFY2(!deviceId.isNull(), "invalid device id for removing");

        QNetworkRequest  deleteRequest(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(deviceId.toString())));
        response = deleteAndWait(deleteRequest);
        QVERIFY2(!response.isNull(), "Could not delete device");
    }
}

void TestRestDevices::parentChildDevices()
{
    // Add parent device
    QVariantMap params;
    params.insert("deviceClassId", mockParentDeviceClassId);

    QNetworkRequest request(QUrl(QString("https://localhost:3333/api/v1/devices")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    QVariant response = postAndWait(request, params);
    QVERIFY2(!response.isNull(), "Could not read add device response");

    DeviceId parentDeviceId = DeviceId(response.toMap().value("id").toString());
    QVERIFY2(parentDeviceId != DeviceId(), "DeviceId not returned");

    // find child device
    response = getAndWait(QNetworkRequest(QUrl("https://localhost:3333/api/v1/devices")));
    QVariantList deviceList = response.toList();
    DeviceId childDeviceId;
    foreach (const QVariant deviceVariant, deviceList) {
        QVariantMap deviceMap = deviceVariant.toMap();
        if (deviceMap.value("deviceClassId").toString() == mockChildDeviceClassId.toString()) {
            if (deviceMap.value("parentId") == parentDeviceId.toString()) {
                childDeviceId = DeviceId(deviceMap.value("id").toString());
            }
        }
    }
    QVERIFY2(!childDeviceId.isNull(), "Could not find child device");

    // try to remove child device
    QNetworkRequest deleteRequest(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(childDeviceId.toString())));
    response = deleteAndWait(deleteRequest, 400);
    //QVERIFY2(!response.isNull(), "Could not delete device");
    QCOMPARE(JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorDeviceIsChild), response.toMap().value("error").toString());

    // check if the child device is still there
    response = getAndWait(QNetworkRequest(QUrl("https://localhost:3333/api/v1/devices")));
    deviceList = response.toList();
    bool found = false;
    foreach (const QVariant deviceVariant, deviceList) {
        QVariantMap deviceMap = deviceVariant.toMap();
        if (deviceMap.value("deviceClassId").toString() == mockChildDeviceClassId.toString()) {
            if (deviceMap.value("id") == childDeviceId.toString()) {
                found = true;
                break;
            }
        }
    }
    QVERIFY2(found, "Could not find child device.");

    // remove the parent device
    deleteRequest.setUrl(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(parentDeviceId.toString())));
    response = deleteAndWait(deleteRequest);
    QVERIFY2(!response.isNull(), "Could not delete device");

    // check if the child device is still there
    response = getAndWait(QNetworkRequest(QUrl("https://localhost:3333/api/v1/devices")));
    deviceList = response.toList();
    found = false;
    foreach (const QVariant deviceVariant, deviceList) {
        QVariantMap deviceMap = deviceVariant.toMap();
        if (deviceMap.value("deviceClassId").toString() == mockChildDeviceClassId.toString()) {
            if (deviceMap.value("id") == childDeviceId.toString()) {
                found = true;
                break;
            }
        }
    }
    QVERIFY2(!found, "Could not find child device.");
}

void TestRestDevices::executeAction_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<ActionTypeId>("actionTypeId");
    QTest::addColumn<QVariantList>("actionParams");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QVariantList params;
    QVariantMap param1;
    param1.insert("paramTypeId", mockActionParam1ParamTypeId);
    param1.insert("value", 5);
    params.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockActionParam2ParamTypeId);
    param2.insert("value", true);
    params.append(param2);

    QTest::newRow("valid action") << m_mockDeviceId << mockActionIdWithParams << params << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid deviceId") << DeviceId::createDeviceId() << mockActionIdWithParams << params << 404 << DeviceManager::DeviceErrorDeviceNotFound;
    QTest::newRow("invalid actionTypeId") << m_mockDeviceId << ActionTypeId::createActionTypeId() << params << 404 << DeviceManager::DeviceErrorActionTypeNotFound;
    QTest::newRow("missing params") << m_mockDeviceId << mockActionIdWithParams << QVariantList() << 500 << DeviceManager::DeviceErrorMissingParameter;
    QTest::newRow("async action") << m_mockDeviceId << mockActionIdAsync << QVariantList() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("broken action") << m_mockDeviceId << mockActionIdFailing << QVariantList() << 500 << DeviceManager::DeviceErrorSetupFailed;
    QTest::newRow("async broken action") << m_mockDeviceId << mockActionIdAsyncFailing << QVariantList() << 500 << DeviceManager::DeviceErrorSetupFailed;
}

void TestRestDevices::executeAction()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(ActionTypeId, actionTypeId);
    QFETCH(QVariantList, actionParams);
    QFETCH(int, expectedStatusCode);
    QFETCH(DeviceManager::DeviceError, error);

    // execute action
    QVariantMap params;
    params.insert("params", actionParams);

    QNetworkRequest request(QUrl(QString("https://localhost:3333/api/v1/devices/%1/execute/%2").arg(deviceId.toString()).arg(actionTypeId.toString())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    QVariant response = postAndWait(request, params, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not read execute action response");
    QCOMPARE(JsonTypes::deviceErrorToString(error), response.toMap().value("error").toString());

    // Fetch action execution history from mock device
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    request.setUrl(QUrl(QString("http://localhost:%1/actionhistory").arg(m_mockDevice1Port)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
    QByteArray data = reply->readAll();

    if (error == DeviceManager::DeviceErrorNoError) {
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

void TestRestDevices::getStateValue_data()
{
    QList<Device*> devices = GuhCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Device *device = devices.first();

    QTest::addColumn<QString>("deviceId");
    QTest::addColumn<QString>("stateTypeId");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("existing state") << device->id().toString() << mockIntStateId.toString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("all states") << device->id().toString() << QString() << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid device") << DeviceId::createDeviceId().toString() << mockIntStateId.toString() << 404 << DeviceManager::DeviceErrorDeviceNotFound;
    QTest::newRow("invalid device id format") << "uuid" << StateTypeId::createStateTypeId().toString() << 400 << DeviceManager::DeviceErrorDeviceNotFound;
    QTest::newRow("invalid statetype") << device->id().toString() << StateTypeId::createStateTypeId().toString() << 404 << DeviceManager::DeviceErrorStateTypeNotFound;
    QTest::newRow("invalid statetype format") << device->id().toString() << "uuid" << 400 << DeviceManager::DeviceErrorStateTypeNotFound;
}

void TestRestDevices::getStateValue()
{
    QFETCH(QString, deviceId);
    QFETCH(QString, stateTypeId);
    QFETCH(int, expectedStatusCode);
    QFETCH(DeviceManager::DeviceError, error);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");

    if (!stateTypeId.isNull()) {
        request.setUrl(QUrl(QString("https://localhost:3333/api/v1/devices/%1/states/%2").arg(deviceId).arg(stateTypeId)));
    } else {
        // Get all states
        request.setUrl(QUrl(QString("https://localhost:3333/api/v1/devices/%1/states").arg(deviceId)));
    }
    QVariant response = getAndWait(request, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not read get state value response");
    if (expectedStatusCode != 200)
        QCOMPARE(JsonTypes::deviceErrorToString(error), response.toMap().value("error").toString());

}

void TestRestDevices::editDevices_data()
{
    QTest::addColumn<QString>("name");

    QTest::newRow("change name") << "New device name";
    QTest::newRow("change name") << "Foo device";
    QTest::newRow("change name") << "Bar device";
}

void TestRestDevices::editDevices()
{
    QFETCH(QString, name);
    QString originalName = "Test device";

    QVariantList deviceParams;
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", httpportParamTypeId);
    httpportParam.insert("value", m_mockDevice1Port - 2);
    deviceParams.append(httpportParam);

    QVariantMap params;
    params.insert("deviceClassId", mockDeviceClassId.toString());
    params.insert("name", originalName);
    params.insert("deviceParams", deviceParams);

    QNetworkRequest addRequest(QUrl("https://localhost:3333/api/v1/devices"));
    addRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QVariant response = postAndWait(addRequest, params);
    DeviceId deviceId = DeviceId(response.toMap().value("id").toString());
    QVERIFY2(!deviceId.isNull(), "invalid device id");

    // edit device
    params.clear();
    params.insert("name", name);

    QNetworkRequest deviceRequest(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(deviceId.toString())));
    deviceRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    response = postAndWait(deviceRequest, params);
    QVERIFY2(response.toMap().value("error").toString() == JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorNoError), "Could not edit device name");

    // check device name
    response = getAndWait(deviceRequest);
    QCOMPARE(response.toMap().value("name").toString(), name);

    // Remove the device
    response = deleteAndWait(deviceRequest);
    QVERIFY2(response.toMap().value("error").toString() == JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorNoError), "Could not remove device");
}

void TestRestDevices::reconfigureDevices_data()
{
    QVariantList asyncChangeDeviceParams;
    QVariantMap asyncParamDifferent;
    asyncParamDifferent.insert("paramTypeId", asyncParamTypeId);
    asyncParamDifferent.insert("value", true);
    asyncChangeDeviceParams.append(asyncParamDifferent);

    QVariantList httpportChangeDeviceParams;
    QVariantMap httpportParamDifferent;
    httpportParamDifferent.insert("paramTypeId", httpportParamTypeId);
    httpportParamDifferent.insert("value", 8895); // if change -> change also newPort in reconfigureDevices()
    httpportChangeDeviceParams.append(httpportParamDifferent);

    QVariantList brokenChangedDeviceParams;
    QVariantMap brokenParamDifferent;
    brokenParamDifferent.insert("paramTypeId", brokenParamTypeId);
    brokenParamDifferent.insert("value", true);
    brokenChangedDeviceParams.append(brokenParamDifferent);

    QVariantList asyncAndPortChangeDeviceParams;
    asyncAndPortChangeDeviceParams.append(asyncParamDifferent);
    asyncAndPortChangeDeviceParams.append(httpportParamDifferent);

    QVariantList changeAllWritableDeviceParams;
    changeAllWritableDeviceParams.append(asyncParamDifferent);
    changeAllWritableDeviceParams.append(httpportParamDifferent);

    QTest::addColumn<bool>("broken");
    QTest::addColumn<QVariantList>("newDeviceParams");
    QTest::addColumn<int>("expectedStatusCode");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("invalid - change async param") << false << asyncChangeDeviceParams << 500 << DeviceManager::DeviceErrorParameterNotWritable;
    QTest::newRow("valid - change httpport param") << false <<  httpportChangeDeviceParams << 200 << DeviceManager::DeviceErrorNoError;
    QTest::newRow("valid - change httpport and async param") << false << asyncAndPortChangeDeviceParams << 500 << DeviceManager::DeviceErrorParameterNotWritable;
    QTest::newRow("invalid - change all params (except broken)") << false << changeAllWritableDeviceParams << 500 << DeviceManager::DeviceErrorParameterNotWritable;
}

void TestRestDevices::reconfigureDevices()
{
    QFETCH(bool, broken);
    QFETCH(QVariantList, newDeviceParams);
    QFETCH(int, expectedStatusCode);
    QFETCH(DeviceManager::DeviceError, error);

    // add device
    QVariantMap params;
    params.insert("deviceClassId", mockDeviceClassId);
    params.insert("name", "Edit mock device");
    QVariantList deviceParams;
    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", asyncParamTypeId);
    asyncParam.insert("value", false);
    deviceParams.append(asyncParam);
    QVariantMap brokenParam;
    brokenParam.insert("paramTypeId", brokenParamTypeId);
    brokenParam.insert("value", broken);
    deviceParams.append(brokenParam);
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", httpportParamTypeId);
    httpportParam.insert("value", 8896);
    deviceParams.append(httpportParam);
    params.insert("deviceParams", deviceParams);

    // ADD a mockdevice
    QNetworkRequest request(QUrl(QString("https://localhost:3333/api/v1/devices")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");

    QVariant response = postAndWait(request, params);
    QVERIFY2(!response.isNull(), "Could not read add device response");

    DeviceId deviceId = DeviceId(response.toMap().value("id").toString());
    QVERIFY2(deviceId != DeviceId(), "DeviceId not returned");

    // now RECONFIGURE the added device
    QVariantMap editParams;
    editParams.insert("deviceId", deviceId);
    editParams.insert("deviceParams", newDeviceParams);

    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(deviceId.toString())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");

    response = putAndWait(request, editParams, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not read edit device response");

    // if the reconfigure should have been successfull
    if (expectedStatusCode == 200) {
        request.setUrl(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(deviceId.toString())));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
        response = getAndWait(request);
        QVariantMap deviceMap = response.toMap();
        verifyParams(newDeviceParams, deviceMap.value("params").toList(), false);
    } else {
        QCOMPARE(JsonTypes::deviceErrorToString(error), response.toMap().value("error").toString());
    }

    // delete it
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(deviceId.toString())));
    response = deleteAndWait(request);
    QVERIFY2(!response.isNull(), "Could not delete device");
}

void TestRestDevices::reconfigureByDiscovery_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<QVariantList>("discoveryParams");
    QTest::addColumn<int>("expectedStatusCode");

    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", resultCountParamTypeId);
    resultCountParam.insert("value", 2);
    discoveryParams.append(resultCountParam);

    QTest::newRow("discover 2 devices with params") << mockDeviceClassId << 2 << discoveryParams << 200;
}

void TestRestDevices::reconfigureByDiscovery()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);
    QFETCH(QVariantList, discoveryParams);
    QFETCH(int, expectedStatusCode);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);

    QUrl url(QString("https://localhost:3333/api/v1/deviceclasses/%1/discover").arg(deviceClassId.toString()));

    if (!discoveryParams.isEmpty()) {
        QUrlQuery query;
        query.addQueryItem("params", QJsonDocument::fromVariant(discoveryParams).toJson(QJsonDocument::Compact));
        url.setQuery(query);
    }

    QNetworkRequest request(url);
    QVariantList foundDevices = getAndWait(request).toList();
    QCOMPARE(foundDevices.count(), resultCount);

    // add Discovered Device 1 port 55555
    request.setUrl(QUrl("https://localhost:3333/api/v1/devices"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
    DeviceDescriptorId descriptorId1;
    foreach (const QVariant &descriptor, foundDevices) {
        // find the device with port 55555
        if (descriptor.toMap().value("description").toString() == "55555") {
            descriptorId1 = DeviceDescriptorId(descriptor.toMap().value("id").toString());
            qDebug() << descriptorId1.toString();
            break;
        }
    }
    params.clear();
    params.insert("deviceClassId", deviceClassId);
    params.insert("name", "Discovered mock device");
    params.insert("deviceDescriptorId", descriptorId1.toString());

    QVariant response = postAndWait(request, params, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not delete device");

    DeviceId deviceId = DeviceId(response.toMap().value("id").toString());
    QVERIFY(!deviceId.isNull());

    // and now rediscover, and edit the first device with the second
    params.clear();
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);

    url = QUrl(QString("https://localhost:3333/api/v1/deviceclasses/%1/discover").arg(deviceClassId.toString()));
    QUrlQuery query2;
    query2.addQueryItem("params", QJsonDocument::fromVariant(discoveryParams).toJson(QJsonDocument::Compact));
    url.setQuery(query2);

    response = getAndWait(QNetworkRequest(url), expectedStatusCode);

    foundDevices = response.toList();
    QCOMPARE(foundDevices.count(), resultCount);

    // get the second device
    DeviceDescriptorId descriptorId2;
    foreach (const QVariant &descriptor, foundDevices) {
        // find the device with port 55556
        if (descriptor.toMap().value("description").toString() == "55556") {
            descriptorId2 = DeviceDescriptorId(descriptor.toMap().value("id").toString());
            break;
        }
    }
    QVERIFY(!descriptorId2.isNull());
    qDebug() << "edit device 1 (55555) with descriptor 2 (55556) " << descriptorId2;

    // RECONFIGURE
    response.clear();
    params.clear();
    params.insert("deviceId", deviceId.toString());
    params.insert("deviceDescriptorId", descriptorId2);

    request = QNetworkRequest(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(deviceId.toString())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");

    response = putAndWait(request, params, expectedStatusCode);
    QVERIFY2(!response.isNull(), "Could not delete device");

    // remove added device
    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(deviceId.toString())));
    response = deleteAndWait(request);
    QVERIFY2(!response.isNull(), "Could not delete device");
}

#include "testrestdevices.moc"
QTEST_MAIN(TestRestDevices)
