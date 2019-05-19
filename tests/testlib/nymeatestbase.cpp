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
#include "nymeasettings.h"
#include "servers/mocktcpserver.h"

using namespace nymeaserver;

Q_LOGGING_CATEGORY(dcTests, "Tests")

PluginId mockPluginId = PluginId("727a4a9a-c187-446f-aadf-f1b2220607d1");
VendorId guhVendorId = VendorId("2062d64d-3232-433c-88bc-0d33c0ba2ba6");
DeviceClassId mockDeviceClassId = DeviceClassId("753f0d32-0468-4d08-82ed-1964aab03298");
DeviceClassId mockDeviceAutoClassId = DeviceClassId("ab4257b3-7548-47ee-9bd4-7dc3004fd197");
DeviceClassId mockPushButtonDeviceClassId = DeviceClassId("9e03144c-e436-4eea-82d9-ccb33ef778db");
DeviceClassId mockDisplayPinDeviceClassId = DeviceClassId("296f1fd4-e893-46b2-8a42-50d1bceb8730");
DeviceClassId mockParentDeviceClassId = DeviceClassId("a71fbde9-9a38-4bf8-beab-c8aade2608ba");
DeviceClassId mockChildDeviceClassId = DeviceClassId("40893c9f-bc47-40c1-8bf7-b390c7c1b4fc");
DeviceClassId mockDeviceDiscoveryClassId = DeviceClassId("1bbaf751-36b7-4d3d-b05a-58dab2a3be8c");
DeviceClassId mockDeviceAsyncSetupClassId = DeviceClassId("c08a8b27-8200-413d-b96b-4cff78b864d9");
DeviceClassId mockDeviceBrokenClassId = DeviceClassId("ba5fb404-c9ce-4db4-8cd4-f48c61c24b13");
DeviceClassId mockDeviceBrokenAsyncSetupClassId = DeviceClassId("bd5b78c5-53c9-4417-8eac-8ab2bce97bd0");
EventTypeId mockEvent1Id = EventTypeId("45bf3752-0fc6-46b9-89fd-ffd878b5b22b");
EventTypeId mockEvent2Id = EventTypeId("863d5920-b1cf-4eb9-88bd-8f7b8583b1cf");
StateTypeId mockIntStateId = StateTypeId("80baec19-54de-4948-ac46-31eabfaceb83");
StateTypeId mockBoolStateId = StateTypeId("9dd6a97c-dfd1-43dc-acbd-367932742310");
StateTypeId mockDoubleStateId = StateTypeId("7cac53ee-7048-4dc9-b000-7b585390f34c");
StateTypeId mockBatteryCriticalStateId = StateTypeId("580bc611-1a55-41f3-996f-8d3ccf543db3");
StateTypeId mockPowerStateTypeId = StateTypeId("064aed0d-da4c-49d4-b236-60f97e98ff84");
ActionTypeId mockActionIdPower = ActionTypeId("064aed0d-da4c-49d4-b236-60f97e98ff84");
ActionTypeId mockActionIdWithParams = ActionTypeId("dea0f4e1-65e3-4981-8eaa-2701c53a9185");
ActionTypeId mockActionIdNoParams = ActionTypeId("defd3ed6-1a0d-400b-8879-a0202cf39935");
ActionTypeId mockActionIdAsync = ActionTypeId("fbae06d3-7666-483e-a39e-ec50fe89054e");
ActionTypeId mockActionIdFailing = ActionTypeId("df3cf33d-26d5-4577-9132-9823bd33fad0");
ActionTypeId mockActionIdAsyncFailing = ActionTypeId("bfe89a1d-3497-4121-8318-e77c37537219");

ParamTypeId configParamIntParamTypeId = ParamTypeId("e1f72121-a426-45e2-b475-8262b5cdf103");
ParamTypeId configParamBoolParamTypeId = ParamTypeId("c75723b6-ea4f-4982-9751-6c5e39c88145");
ParamTypeId httpportParamTypeId = ParamTypeId("d4f06047-125e-4479-9810-b54c189917f5");
ParamTypeId asyncParamTypeId = ParamTypeId("f2977061-4dd0-4ef5-85aa-3b7134743be3");
ParamTypeId brokenParamTypeId = ParamTypeId("ae8f8901-f2c1-42a5-8111-6d2fc8e4c1e4");
ParamTypeId resultCountParamTypeId = ParamTypeId("d222adb4-2f9c-4c3f-8655-76400d0fb6ce");
ParamTypeId mockActionParam1ParamTypeId = ParamTypeId("a2d3a256-a551-4712-a65b-ecd5a436a1cb");
ParamTypeId mockActionParam2ParamTypeId = ParamTypeId("304a4899-18be-4e3b-94f4-d03be52f3233");
ParamTypeId mockParamIntParamTypeId = ParamTypeId("0550e16d-60b9-4ba5-83f4-4d3cee656121");
ParamTypeId colorStateParamTypeId = ParamTypeId("20dc7c22-c50e-42db-837c-2bbced939f8e");
ParamTypeId percentageStateParamTypeId = ParamTypeId("72981c04-267a-4ba0-a59e-9921d2f3af9c");
ParamTypeId allowedValuesStateParamTypeId = ParamTypeId("05f63f9c-f61e-4dcf-ad55-3f13fde2765b");
ParamTypeId doubleStateParamTypeId = ParamTypeId("53cd7c55-49b7-441b-b970-9048f20f0e2c");
ParamTypeId boolStateParamTypeId = ParamTypeId("e680f7a4-b39e-46da-be41-fa3170fe3768");
ParamTypeId pinParamTypeId = ParamTypeId("da820e07-22dc-4173-9c07-2f49a4e265f9");
ParamTypeId boolValueStateParamTypeId = ParamTypeId("d24ede5f-4064-4898-bb84-cfb533b1fbc0");
ParamTypeId parentUuidParamTypeId = ParamTypeId("104b5288-404e-42d3-bf38-e40682e75681");
ParamTypeId textLineParamTypeId = ParamTypeId("e6acf0c7-4b8e-4296-ac62-855d20deb816");
ParamTypeId textAreaParamTypeId = ParamTypeId("716f0994-bc01-42b0-b64d-59236f7320d2");
ParamTypeId passwordParamTypeId = ParamTypeId("e5c0d14b-c9f1-4aca-a56e-85bfa6977150");
ParamTypeId searchParamTypeId = ParamTypeId("22add8c9-ee4f-43ad-8931-58e999313ac3");
ParamTypeId mailParamTypeId = ParamTypeId("a8494faf-3a0f-4cf3-84b7-4b39148a838d");
ParamTypeId ip4ParamTypeId = ParamTypeId("9e5f86a0-4bb3-4892-bff8-3fc4032af6e2");
ParamTypeId ip6ParamTypeId = ParamTypeId("43bf3832-dd48-4090-a836-656e8b60216e");
ParamTypeId urlParamTypeId = ParamTypeId("fa67229f-fcef-496f-b671-59a4b48f3ab5");
ParamTypeId macParamTypeId = ParamTypeId("e93db587-7919-48f3-8c88-1651de63c765");


// Parent device
EventTypeId mockParentChildEventId = EventTypeId("d24ede5f-4064-4898-bb84-cfb533b1fbc0");
ActionTypeId mockParentChildActionId = ActionTypeId("d24ede5f-4064-4898-bb84-cfb533b1fbc0");
StateTypeId mockParentChildStateId = StateTypeId("d24ede5f-4064-4898-bb84-cfb533b1fbc0");

NymeaTestBase::NymeaTestBase(QObject *parent) :
    QObject(parent),
    m_commandId(0)
{
    qRegisterMetaType<QNetworkReply*>();
    qsrand(QDateTime::currentMSecsSinceEpoch());
    m_mockDevice1Port = 1337 + (qrand() % 10000);
    m_mockDevice2Port = 7331 + (qrand() % 10000);

    // Important for settings
    QCoreApplication::instance()->setOrganizationName("nymea-test");
}

void NymeaTestBase::initTestCase()
{
    qDebug() << "NymeaTestBase starting.";

    // If testcase asserts cleanup won't do. Lets clear any previous test run settings leftovers
    qDebug() << "Reset test settings";
    NymeaSettings rulesSettings(NymeaSettings::SettingsRoleRules);
    rulesSettings.clear();
    NymeaSettings deviceSettings(NymeaSettings::SettingsRoleDevices);
    deviceSettings.clear();
    NymeaSettings pluginSettings(NymeaSettings::SettingsRolePlugins);
    pluginSettings.clear();
    NymeaSettings statesSettings(NymeaSettings::SettingsRoleDeviceStates);
    statesSettings.clear();

    // Reset to default settings
    NymeaSettings nymeadSettings(NymeaSettings::SettingsRoleGlobal);
    nymeadSettings.clear();

    QLoggingCategory::setFilterRules("*.debug=false\nTests.debug=true");

    // Start the server
    NymeaCore::instance()->init();

    // Wait unitl the server is initialized
    QSignalSpy coreInitializedSpy(NymeaCore::instance(), SIGNAL(initialized()));
    QVERIFY(coreInitializedSpy.wait());

    // Yes, we're intentionally mixing upper/lower case email here... username should not be case sensitive
    NymeaCore::instance()->userManager()->removeUser("dummy@guh.io");
    NymeaCore::instance()->userManager()->createUser("dummy@guh.io", "DummyPW1!");
    m_apiToken = NymeaCore::instance()->userManager()->authenticate("Dummy@guh.io", "DummyPW1!", "testcase");

    if (MockTcpServer::servers().isEmpty()) {
        qWarning() << "no mock tcp server found";
        exit(-1);
    }

    // Add the mockdevice
    m_mockTcpServer = MockTcpServer::servers().first();
    m_clientId = QUuid::createUuid();
    m_mockTcpServer->clientConnected(m_clientId);

    QVariant response = injectAndWait("JSONRPC.Hello");

    createMockDevice();

    response = injectAndWait("Devices.GetConfiguredDevices", {});
    foreach (const QVariant &device, response.toMap().value("params").toMap().value("devices").toList()) {
        if (device.toMap().value("deviceClassId").toUuid() == mockDeviceAutoClassId) {
            m_mockDeviceAutoId = DeviceId(device.toMap().value("id").toString());
        }
    }
}

void NymeaTestBase::cleanupTestCase()
{
    NymeaCore::instance()->destroy();
}

void NymeaTestBase::cleanup()
{
    // In case a test deleted the mock device, lets recreate it.
    if (NymeaCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId).count() == 0) {
        createMockDevice();
    }
}

QVariant NymeaTestBase::injectAndWait(const QString &method, const QVariantMap &params, const QUuid &clientId)
{
    QVariantMap call;
    call.insert("id", m_commandId);
    call.insert("method", method);
    call.insert("params", params);
    call.insert("token", m_apiToken);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(call);
    QSignalSpy spy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    m_mockTcpServer->injectData(clientId.isNull() ? m_clientId : clientId, jsonDoc.toJson(QJsonDocument::Compact) + "\n");

    if (spy.count() == 0) {
        spy.wait();
    }

    for (int i = 0; i < spy.count(); i++) {
        // Make sure the response it a valid JSON string
        QJsonParseError error;
        jsonDoc = QJsonDocument::fromJson(spy.at(i).last().toByteArray(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parser error" << error.errorString() << spy.at(i).last().toByteArray();
            return QVariant();
        }
        QVariantMap response = jsonDoc.toVariant().toMap();

        // skip notifications
        if (response.contains("notification"))
            continue;

        if (response.value("id").toInt() == m_commandId) {
            m_commandId++;
            return jsonDoc.toVariant();
        }
    }
    m_commandId++;
    return QVariant();
}

QVariant NymeaTestBase::checkNotification(const QSignalSpy &spy, const QString &notification)
{
    //qDebug() << "Got" << spy.count() << "notifications while waiting for" << notification;
    for (int i = 0; i < spy.count(); i++) {
        // Make sure the response it a valid JSON string
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.at(i).last().toByteArray(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parser error" << error.errorString();
            return QVariant();
        }

        QVariantMap response = jsonDoc.toVariant().toMap();
        if (response.value("notification").toString() == notification) {
            return jsonDoc.toVariant();
        }
    }
    return QVariant();
}

QVariantList NymeaTestBase::checkNotifications(const QSignalSpy &spy, const QString &notification)
{
//    qWarning() << "Got" << spy.count() << "notifications while waiting for" << notification;
    QVariantList notificationList;
    for (int i = 0; i < spy.count(); i++) {
        // Make sure the response it a valid JSON string
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.at(i).last().toByteArray(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parser error" << error.errorString();
            return notificationList;
        }

        QVariantMap response = jsonDoc.toVariant().toMap();
        if (response.value("notification").toString() == notification) {
            notificationList.append(jsonDoc.toVariant());
        }
    }
    return notificationList;
}

QVariant NymeaTestBase::getAndWait(const QNetworkRequest &request, const int &expectedStatus)
{
    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply *reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkReply *reply = nam.get(request);

    if (clientSpy.count() == 0) {
        clientSpy.wait();
    }
    qDebug() << "*** finished" << reply->isFinished() << reply->error() << reply->errorString();

    if (clientSpy.count() == 0) {
        qWarning() << "Got no response for get request";
        reply->deleteLater();
        return QVariant();
    }

    QByteArray data = reply->readAll();
    verifyReply(reply, data, expectedStatus);

    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parser error" << error.errorString();
        return QVariant();
    }

    return jsonDoc.toVariant();
}

QVariant NymeaTestBase::deleteAndWait(const QNetworkRequest &request, const int &expectedStatus)
{
    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply *reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkReply *reply = nam.deleteResource(request);

    if (clientSpy.count() == 0) {
        clientSpy.wait();
    }

    if (clientSpy.count() == 0) {
        qWarning() << "Got no response for delete request";
        reply->deleteLater();
        return QVariant();
    }

    QByteArray data = reply->readAll();
    verifyReply(reply, data, expectedStatus);

    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parser error" << error.errorString();
        return QVariant();
    }

    return jsonDoc.toVariant();
}

QVariant NymeaTestBase::postAndWait(const QNetworkRequest &request, const QVariant &params, const int &expectedStatus)
{
    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply *reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QByteArray payload = QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = nam.post(request, payload);

    if (clientSpy.count() == 0) {
        clientSpy.wait();
    }

    if (clientSpy.count() == 0) {
        qWarning() << "Got no response for post request";
        reply->deleteLater();
        return QVariant();
    }



    QByteArray data = reply->readAll();
    verifyReply(reply, data, expectedStatus);

    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parser error" << error.errorString();
        return QVariant();
    }

    return jsonDoc.toVariant();
}


QVariant NymeaTestBase::putAndWait(const QNetworkRequest &request, const QVariant &params, const int &expectedStatus)
{
    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply *reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QByteArray payload = QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = nam.put(request, payload);

    if (clientSpy.count() == 0) {
        clientSpy.wait();
    }

    if (clientSpy.count() == 0) {
        qWarning() << "Got no response for put request";
        reply->deleteLater();
        return QVariant();
    }

    QByteArray data = reply->readAll();
    verifyReply(reply, data, expectedStatus);

    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parser error" << error.errorString();
        return QVariant();
    }

    return jsonDoc.toVariant();
}

void NymeaTestBase::verifyReply(QNetworkReply *reply, const QByteArray &data, const int &expectedStatus)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatus);

    Q_UNUSED(data)
//    if (!data.isEmpty()) {
//        QJsonParseError error;
//        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
//        QCOMPARE(error.error, QJsonParseError::NoError);
//        Q_UNUSED(jsonDoc);
//    }
}

bool NymeaTestBase::enableNotifications()
{
    QVariantMap notificationParams;
    notificationParams.insert("enabled", true);
    QVariant response = injectAndWait("JSONRPC.SetNotificationStatus", notificationParams);
    if (response.toMap().value("params").toMap().value("enabled").toBool() != true) {
        return false;
    }
    qDebug() << "Notifications enabled.";
    return true;
}

bool NymeaTestBase::disableNotifications()
{
    QVariantMap notificationParams;
    notificationParams.insert("enabled", false);
    QVariant response = injectAndWait("JSONRPC.SetNotificationStatus", notificationParams);
    if (response.toMap().value("params").toMap().value("enabled").toBool() != false) {
        return false;
    }
    qDebug() << "Notifications disabled.";
    return true;
}

void NymeaTestBase::restartServer()
{
    // Destroy and recreate the core instance...
    NymeaCore::instance()->destroy();
    NymeaCore::instance()->init();
    QSignalSpy coreSpy(NymeaCore::instance(), SIGNAL(initialized()));
    coreSpy.wait();
    m_mockTcpServer = MockTcpServer::servers().first();
    m_mockTcpServer->clientConnected(m_clientId);

    injectAndWait("JSONRPC.Hello");
}

void NymeaTestBase::clearLoggingDatabase()
{
    NymeaCore::instance()->logEngine()->clearDatabase();
}

void NymeaTestBase::createMockDevice()
{
    QVariantMap params;
    params.insert("name", "Test Mock Device");
    params.insert("deviceClassId", "{753f0d32-0468-4d08-82ed-1964aab03298}");

    QVariantList deviceParams;
    QVariantMap httpPortParam;
    httpPortParam.insert("paramTypeId", httpportParamTypeId.toString());
    httpPortParam.insert("value", m_mockDevice1Port);
    deviceParams.append(httpPortParam);
    params.insert("deviceParams", deviceParams);

    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);

    verifyDeviceError(response);

    m_mockDeviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY2(!m_mockDeviceId.isNull(), "Newly created mock device must not be null.");
}

