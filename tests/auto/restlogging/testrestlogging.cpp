/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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
#include "guhsettings.h"
#include "plugin/deviceplugin.h"

#include <QDebug>
#include <QSignalSpy>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

using namespace guhserver;

class TestRestLogging : public GuhTestBase
{
    Q_OBJECT

private:

private slots:
    void initLogs();

    void invalidFilter_data();
    void invalidFilter();

    void invalidFilterJson();

    void eventLogs();

    void actionLog();

    // this has to be the last test
    void removeDevice();
};

void TestRestLogging::initLogs()
{
    QNetworkRequest request(QUrl("https://localhost:3333/api/v1/logs"));
    QVariant response = getAndWait(request);

    QVariantList logEntries = response.toList();
    qDebug() << "Got" << logEntries.count() << "logs";
    QVERIFY(logEntries.count() > 0);

    clearLoggingDatabase();

    response = getAndWait(request);
    logEntries = response.toList();
    qDebug() << "Got" << logEntries.count() << "logs";
    QVERIFY(logEntries.count() == 0);

}

void TestRestLogging::invalidFilter_data()
{
    QVariantMap invalidSourcesFilter;
    invalidSourcesFilter.insert("loggingSources", QVariantList() << "bla");

    QVariantMap invalidFilterValue;
    invalidFilterValue.insert("loggingSource", QVariantList() << "bla");

    QVariantMap invalidTypeIds;
    invalidTypeIds.insert("typeId", QVariantList() << "bla" << "blub");

    QVariantMap invalidEventTypes;
    invalidEventTypes.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger) << "blub");

    QTest::addColumn<QVariantMap>("filter");

    QTest::newRow("Invalid source") << invalidSourcesFilter;
    QTest::newRow("Invalid filter value") << invalidFilterValue;
    QTest::newRow("Invalid typeIds") << invalidTypeIds;
    QTest::newRow("Invalid eventTypes") << invalidEventTypes;
}

void TestRestLogging::invalidFilter()
{
    QFETCH(QVariantMap, filter);

    QUrl url("https://localhost:3333/api/v1/logs");
    QUrlQuery query;
    query.addQueryItem("filter", QJsonDocument::fromVariant(filter).toJson(QJsonDocument::Compact));
    url.setQuery(query);

    // there should be 2 logs, one for shutdown, one for startup (from server restart)
    QVariant response = getAndWait(QNetworkRequest(url));
    QVERIFY(!response.isNull());

    // TODO: validate filter for REST api
}

void TestRestLogging::invalidFilterJson()
{
    QUrl url("https://localhost:3333/api/v1/logs");
    QUrlQuery query;
    query.addQueryItem("filter", "blabla:!!");
    url.setQuery(query);

    getAndWait(QNetworkRequest(url), 400);
}

void TestRestLogging::eventLogs()
{
    QList<Device*> devices = GuhCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Device *device = devices.first();

    // enable notifications
    QCOMPARE(enableNotifications(), true);

    // Setup connection to mock client
    QNetworkAccessManager nam;
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // trigger event in mock device
    int port = device->paramValue(httpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(port).arg(mockEvent1Id.toString())));
    QNetworkReply *reply = nam.get(request);
    clientSpy.wait();

    // Lets wait for the notification
    QVariantList loggEntryAddedVariants = checkNotifications(clientSpy, "Logging.LogEntryAdded");
    QVERIFY2(!loggEntryAddedVariants.isEmpty(), "Did not get Logging.LogEntryAdded notification.");
    qDebug() << "got" << loggEntryAddedVariants.count() << "Logging.LogEntryAdded notifications";
    reply->deleteLater();


    bool found = false;
    qDebug() << "got" << loggEntryAddedVariants.count() << "Logging.LogEntryAdded";
    foreach (const QVariant &loggEntryAddedVariant, loggEntryAddedVariants) {
        QVariantMap logEntry = loggEntryAddedVariant.toMap().value("params").toMap().value("logEntry").toMap();
        if (logEntry.value("deviceId").toString() == device->id().toString()) {
            found = true;
            // Make sure the notification contains all the stuff we expect
            QCOMPARE(logEntry.value("typeId").toString(), mockEvent1Id.toString());
            QCOMPARE(logEntry.value("eventType").toString(), JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
            QCOMPARE(logEntry.value("source").toString(), JsonTypes::loggingSourceToString(Logging::LoggingSourceEvents));
            QCOMPARE(logEntry.value("loggingLevel").toString(), JsonTypes::loggingLevelToString(Logging::LoggingLevelInfo));
            break;
        }
    }
    if (!found)
        qDebug() << QJsonDocument::fromVariant(loggEntryAddedVariants).toJson();

    QVERIFY2(found, "Could not find the corresponding Logging.LogEntryAdded notification");

    // get this logentry with filter
    QVariantMap params;
    params.insert("deviceIds", QVariantList() << device->id());
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceEvents));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("typeIds", QVariantList() << mockEvent1Id);

    QVariant response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() == 1);

    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

void TestRestLogging::actionLog()
{
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockActionParam1ParamTypeId);
    param1.insert("value", 7);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockActionParam2ParamTypeId);
    param2.insert("value", true);
    actionParams.append(param2);

    QVariantMap params;
    params.insert("actionTypeId", mockActionIdWithParams);
    params.insert("deviceId", m_mockDeviceId);
    params.insert("params", actionParams);

    // enable notifications
    QCOMPARE(enableNotifications(), true);

    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // EXECUTE with params
    QVariant response = injectAndWait("Actions.ExecuteAction", params);
    verifyDeviceError(response);

    // Lets wait 3for the notification
    clientSpy.wait(200);
    QVariant notification = checkNotification(clientSpy, "Logging.LogEntryAdded");
    QVERIFY(!notification.isNull());

    QVariantMap logEntry = notification.toMap().value("params").toMap().value("logEntry").toMap();

    // Make sure the notification contains all the stuff we expect
    QCOMPARE(logEntry.value("typeId").toString(), mockActionIdWithParams.toString());
    QCOMPARE(logEntry.value("deviceId").toString(), m_mockDeviceId.toString());
    QCOMPARE(logEntry.value("eventType").toString(), JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    QCOMPARE(logEntry.value("source").toString(), JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    QCOMPARE(logEntry.value("loggingLevel").toString(), JsonTypes::loggingLevelToString(Logging::LoggingLevelInfo));

    // EXECUTE without params
    params.clear(); clientSpy.clear();
    params.insert("actionTypeId", mockActionIdNoParams);
    params.insert("deviceId", m_mockDeviceId);
    response = injectAndWait("Actions.ExecuteAction", params);
    verifyDeviceError(response);

    clientSpy.wait(200);
    notification = checkNotification(clientSpy, "Logging.LogEntryAdded");
    QVERIFY(!notification.isNull());

    // get this logentry with filter
    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("values", QVariantList() << "7, true");

    QUrl url("https://localhost:3333/api/v1/logs");
    QUrlQuery query;
    query.addQueryItem("filter", QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact));
    url.setQuery(query);

    response = getAndWait(QNetworkRequest(url));
    QVariantList logEntries = response.toList();
    QCOMPARE(logEntries.count(), 1);

    // EXECUTE broken action
    params.clear(); clientSpy.clear();
    params.insert("actionTypeId", mockActionIdFailing);
    params.insert("deviceId", m_mockDeviceId);
    response = injectAndWait("Actions.ExecuteAction", params);
    verifyDeviceError(response, DeviceManager::DeviceErrorSetupFailed);

    clientSpy.wait(200);
    notification = checkNotification(clientSpy, "Logging.LogEntryAdded");
    QVERIFY(!notification.isNull());

    logEntry = notification.toMap().value("params").toMap().value("logEntry").toMap();

    // Make sure the notification contains all the stuff we expect
    QCOMPARE(logEntry.value("typeId").toString(), mockActionIdFailing.toString());
    QCOMPARE(logEntry.value("deviceId").toString(), m_mockDeviceId.toString());
    QCOMPARE(logEntry.value("eventType").toString(), JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    QCOMPARE(logEntry.value("source").toString(), JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    QCOMPARE(logEntry.value("loggingLevel").toString(), JsonTypes::loggingLevelToString(Logging::LoggingLevelAlert));
    QCOMPARE(logEntry.value("errorCode").toString(), JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorSetupFailed));

    // get this logentry with filter
    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("values", QVariantList() << "7, true");

    query.clear();
    query.addQueryItem("filter", QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact));
    url.setQuery(query);

    response = getAndWait(QNetworkRequest(url));
    logEntries = response.toList();
    QCOMPARE(logEntries.count(), 1);

    // check different filters
    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("typeIds", QVariantList() << mockActionIdNoParams);

    query.clear();
    query.addQueryItem("filter", QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact));
    url.setQuery(query);

    response = getAndWait(QNetworkRequest(url));
    logEntries = response.toList();
    QCOMPARE(logEntries.count(), 1);

    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("typeIds", QVariantList() << mockActionIdNoParams << mockActionIdWithParams << mockActionIdFailing);

    query.clear();
    query.addQueryItem("filter", QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact));
    url.setQuery(query);

    response = getAndWait(QNetworkRequest(url));
    logEntries = response.toList();
    QCOMPARE(logEntries.count(), 3);

    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

void TestRestLogging::removeDevice()
{
    // enable notifications
    QCOMPARE(enableNotifications(), true);

    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    QNetworkRequest deleteRequest(QUrl(QString("https://localhost:3333/api/v1/devices/%1").arg(m_mockDeviceId.toString())));
    QVariant response = deleteAndWait(deleteRequest);
    QVERIFY2(!response.isNull(), "Could not delete device");

    clientSpy.wait(200);
    QVariant notification = checkNotification(clientSpy, "Logging.LogDatabaseUpdated");
    QVERIFY(!notification.isNull());

    // get this logentry with filter
    QUrl url("https://localhost:3333/api/v1/logs");
    response = getAndWait(QNetworkRequest(url));
    QVariantList logEntries = response.toList();
    QVERIFY2(!logEntries.count() != 0, "No log entries left");

    // verify that the logs from this device where removed from the db
    QVariantMap params;
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    QUrlQuery query;
    query.addQueryItem("filter", QJsonDocument::fromVariant(params).toJson(QJsonDocument::Compact));
    url.setQuery(query);

    response = getAndWait(QNetworkRequest(url));
    logEntries = response.toList();

    QCOMPARE(logEntries.count(), 0);

    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

#include "testrestlogging.moc"
QTEST_MAIN(TestRestLogging)

