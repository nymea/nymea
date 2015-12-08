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
#include "mocktcpserver.h"
#include "guhcore.h"
#include "guhsettings.h"
#include "devicemanager.h"
#include "logging/logengine.h"
#include "jsontypes.h"

#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSignalSpy>
#include <QtTest>
#include <QDebug>
#include <QMetaType>
#include <QNetworkReply>

using namespace guhserver;

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
ActionTypeId mockActionIdWithParams = ActionTypeId("dea0f4e1-65e3-4981-8eaa-2701c53a9185");
ActionTypeId mockActionIdNoParams = ActionTypeId("defd3ed6-1a0d-400b-8879-a0202cf39935");
ActionTypeId mockActionIdAsync = ActionTypeId("fbae06d3-7666-483e-a39e-ec50fe89054e");
ActionTypeId mockActionIdFailing = ActionTypeId("df3cf33d-26d5-4577-9132-9823bd33fad0");
ActionTypeId mockActionIdAsyncFailing = ActionTypeId("bfe89a1d-3497-4121-8318-e77c37537219");

GuhTestBase::GuhTestBase(QObject *parent) :
    QObject(parent),
    m_commandId(0)
{
    qRegisterMetaType<QNetworkReply*>();
    qsrand(QDateTime::currentMSecsSinceEpoch());
    m_mockDevice1Port = 1337 + (qrand() % 1000);
    m_mockDevice2Port = 7331 + (qrand() % 1000);
    QCoreApplication::instance()->setOrganizationName("guh-test");
}

void GuhTestBase::initTestCase()
{
    // If testcase asserts cleanup won't do. Lets clear any previous test run settings leftovers
    GuhSettings rulesSettings(GuhSettings::SettingsRoleRules);
    rulesSettings.clear();
    GuhSettings deviceSettings(GuhSettings::SettingsRoleDevices);
    deviceSettings.clear();

    GuhCore::instance();

    // Wait for the DeviceManager to signal that it has loaded plugins and everything
    QSignalSpy spy(GuhCore::instance()->deviceManager(), SIGNAL(loaded()));
    QVERIFY(spy.isValid());
    QVERIFY(spy.wait());

    if (MockTcpServer::servers().isEmpty()) {
        qWarning() << "no mock tcp server found";
        exit(-1);
    }

    m_mockTcpServer = MockTcpServer::servers().first();
    m_clientId = QUuid::createUuid();

    // Lets add one instance of the mockdevice
    QVariantMap params;
    params.insert("deviceClassId", "{753f0d32-0468-4d08-82ed-1964aab03298}");

    QVariantList deviceParams;
    QVariantMap httpPortParam;
    httpPortParam.insert("name", "httpport");
    httpPortParam.insert("value", m_mockDevice1Port);
    deviceParams.append(httpPortParam);
    params.insert("deviceParams", deviceParams);

    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);

    verifyDeviceError(response);

    m_mockDeviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY2(!m_mockDeviceId.isNull(), "Newly created mock device must not be null.");
}

void GuhTestBase::cleanupTestCase()
{
    GuhCore::instance()->destroy();
}

QVariant GuhTestBase::injectAndWait(const QString &method, const QVariantMap &params)
{
    QVariantMap call;
    call.insert("id", m_commandId);
    call.insert("method", method);
    call.insert("params", params);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(call);
    QSignalSpy spy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    m_mockTcpServer->injectData(m_clientId, jsonDoc.toJson());

    if (spy.count() == 0) {
        spy.wait();
    }

    for (int i = 0; i < spy.count(); i++) {
        // Make sure the response it a valid JSON string
        QJsonParseError error;
        jsonDoc = QJsonDocument::fromJson(spy.at(i).last().toByteArray(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parser error" << error.errorString();
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

QVariant GuhTestBase::checkNotification(const QSignalSpy &spy, const QString &notification)
{
    qDebug() << "Got" << spy.count() << "notifications while waiting for" << notification;
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

QVariant GuhTestBase::getAndWait(const QNetworkRequest &request, const int &expectedStatus)
{
    QNetworkAccessManager nam;
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkReply *reply = nam.get(request);

    clientSpy.wait();

    if (clientSpy.count() != 1) {
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

QVariant GuhTestBase::deleteAndWait(const QNetworkRequest &request, const int &expectedStatus)
{
    QNetworkAccessManager nam;
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkReply *reply = nam.deleteResource(request);

    clientSpy.wait();

    if (clientSpy.count() != 1) {
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

QVariant GuhTestBase::postAndWait(const QNetworkRequest &request, const QVariant &params, const int &expectedStatus)
{
    QNetworkAccessManager nam;
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QByteArray payload = QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = nam.post(request, payload);

    clientSpy.wait();

    if (clientSpy.count() != 1) {
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


QVariant GuhTestBase::putAndWait(const QNetworkRequest &request, const QVariant &params, const int &expectedStatus)
{
    QNetworkAccessManager nam;
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QByteArray payload = QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = nam.put(request, payload);

    clientSpy.wait();

    if (clientSpy.count() != 1) {
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

void GuhTestBase::verifyReply(QNetworkReply *reply, const QByteArray &data, const int &expectedStatus)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatus);

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    Q_UNUSED(jsonDoc);
}

bool GuhTestBase::enableNotifications()
{
    QVariantMap notificationParams;
    notificationParams.insert("enabled", true);
    QVariant response = injectAndWait("JSONRPC.SetNotificationStatus", notificationParams);
    if (response.toMap().value("params").toMap().value("enabled").toBool() != true) {
        return false;
    }
    return true;
}

bool GuhTestBase::disableNotifications()
{
    QVariantMap notificationParams;
    notificationParams.insert("enabled", false);
    QVariant response = injectAndWait("JSONRPC.SetNotificationStatus", notificationParams);
    if (response.toMap().value("params").toMap().value("enabled").toBool() != false) {
        return false;
    }
    return true;
}

void GuhTestBase::restartServer()
{
    // Destroy and recreate the core instance...
    GuhCore::instance()->destroy();
    QSignalSpy spy(GuhCore::instance()->deviceManager(), SIGNAL(loaded()));
    spy.wait();
    m_mockTcpServer = MockTcpServer::servers().first();
}

void GuhTestBase::clearLoggingDatabase()
{
    GuhCore::instance()->logEngine()->clearDatabase();
}

