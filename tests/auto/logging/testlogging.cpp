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
#include "guhsettings.h"
#include "plugin/deviceplugin.h"

#include <QDebug>
#include <QSignalSpy>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

using namespace guhserver;

class TestLogging : public GuhTestBase
{
    Q_OBJECT

private:

private slots:
    void initLogs();
    void systemLogs();

    void invalidFilter_data();
    void invalidFilter();

    void eventLogs();
    void actionLog();

    // this has to be the last test
    void removeDevice();
};

void TestLogging::initLogs()
{
    QVariant response = injectAndWait("Logging.GetLogEntries");
    verifyLoggingError(response);

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    qDebug() << "Got" << logEntries.count() << "logs";
    QVERIFY(logEntries.count() > 0);

    clearLoggingDatabase();

    response = injectAndWait("Logging.GetLogEntries");
    verifyLoggingError(response);
    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() == 0);

    restartServer();
}

void TestLogging::systemLogs()
{
    // check the active system log at boot
    QVariantMap params;
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceSystem));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeActiveChange));

    // there should be 2 logs, one for shutdown, one for startup (from server restart)
    QVariant response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);
    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() == 2);

    QVariantMap logEntryShutdown = logEntries.first().toMap();

    QCOMPARE(logEntryShutdown.value("active").toBool(), false);
    QCOMPARE(logEntryShutdown.value("eventType").toString(), JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeActiveChange));
    QCOMPARE(logEntryShutdown.value("source").toString(), JsonTypes::loggingSourceToString(Logging::LoggingSourceSystem));
    QCOMPARE(logEntryShutdown.value("loggingLevel").toString(), JsonTypes::loggingLevelToString(Logging::LoggingLevelInfo));

    QVariantMap logEntryStartup = logEntries.last().toMap();

    QCOMPARE(logEntryStartup.value("active").toBool(), true);
    QCOMPARE(logEntryStartup.value("eventType").toString(), JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeActiveChange));
    QCOMPARE(logEntryStartup.value("source").toString(), JsonTypes::loggingSourceToString(Logging::LoggingSourceSystem));
    QCOMPARE(logEntryStartup.value("loggingLevel").toString(), JsonTypes::loggingLevelToString(Logging::LoggingLevelInfo));
}

void TestLogging::invalidFilter_data()
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

void TestLogging::invalidFilter()
{
    QFETCH(QVariantMap, filter);
    QVariant response = injectAndWait("Logging.GetLogEntries", filter);
    QVERIFY(!response.isNull());

    // verify json error
    QVERIFY(response.toMap().value("status").toString() == "error");
    qDebug() << response.toMap().value("error").toString();
}


void TestLogging::eventLogs()
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
    int port = device->paramValue("httpport").toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(port).arg(mockEvent1Id.toString())));
    QNetworkReply *reply = nam.get(request);

    // Lets wait for the notification
    clientSpy.wait(1000);
    reply->deleteLater();

    // Make sure the logg notification contains all the stuff we expect
    QVariantList loggEntryAddedVariants = checkNotifications(clientSpy, "Logging.LogEntryAdded");
    QVERIFY2(!loggEntryAddedVariants.isEmpty(), "Did not get Logging.LogEntryAdded notification.");
    qDebug() << "got" << loggEntryAddedVariants.count() << "Logging.LogEntryAdded notifications";

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

void TestLogging::actionLog()
{
    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("name", "mockActionParam1");
    param1.insert("value", 7);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("name", "mockActionParam2");
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

    // Lets wait for the notification
    clientSpy.wait(200);

    QVariantList loggEntryAddedVariants = checkNotifications(clientSpy, "Logging.LogEntryAdded");
    QVERIFY2(!loggEntryAddedVariants.isEmpty(), "Did not get Logging.LogEntryAdded notification.");
    qDebug() << "got" << loggEntryAddedVariants.count() << "Logging.LogEntryAdded notifications";

    bool found = false;
    qDebug() << "got" << loggEntryAddedVariants.count() << "Logging.LogEntryAdded";
    foreach (const QVariant &loggEntryAddedVariant, loggEntryAddedVariants) {
        QVariantMap logEntry = loggEntryAddedVariant.toMap().value("params").toMap().value("logEntry").toMap();
        if (logEntry.value("deviceId").toString() == m_mockDeviceId.toString()) {
            found = true;
            // Make sure the notification contains all the stuff we expect
            QCOMPARE(logEntry.value("typeId").toString(), mockActionIdWithParams.toString());
            QCOMPARE(logEntry.value("eventType").toString(), JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
            QCOMPARE(logEntry.value("source").toString(), JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
            QCOMPARE(logEntry.value("loggingLevel").toString(), JsonTypes::loggingLevelToString(Logging::LoggingLevelInfo));
            break;
        }
    }
    if (!found)
        qDebug() << QJsonDocument::fromVariant(loggEntryAddedVariants).toJson();

    QVERIFY2(found, "Could not find the corresponding Logging.LogEntryAdded notification");

    // EXECUTE without params
    params.clear(); clientSpy.clear();
    params.insert("actionTypeId", mockActionIdNoParams);
    params.insert("deviceId", m_mockDeviceId);
    response = injectAndWait("Actions.ExecuteAction", params);
    verifyDeviceError(response);

    clientSpy.wait(200);

    QVariant notification = checkNotification(clientSpy, "Logging.LogEntryAdded");
    QVERIFY(!notification.isNull());

    // get this logentry with filter
    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("values", QVariantList() << "7, true");

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QCOMPARE(logEntries.count(), 1);

    // EXECUTE broken action
    params.clear(); clientSpy.clear();
    params.insert("actionTypeId", mockActionIdFailing);
    params.insert("deviceId", m_mockDeviceId);
    response = injectAndWait("Actions.ExecuteAction", params);
    verifyDeviceError(response, DeviceManager::DeviceErrorSetupFailed);

    clientSpy.wait(200);

    loggEntryAddedVariants = checkNotifications(clientSpy, "Logging.LogEntryAdded");
    QVERIFY2(!loggEntryAddedVariants.isEmpty(), "Did not get Logging.LogEntryAdded notification.");
    qDebug() << "got" << loggEntryAddedVariants.count() << "Logging.LogEntryAdded notifications";

    found = false;
    qDebug() << "got" << loggEntryAddedVariants.count() << "Logging.LogEntryAdded";
    foreach (const QVariant &loggEntryAddedVariant, loggEntryAddedVariants) {
        QVariantMap logEntry = loggEntryAddedVariant.toMap().value("params").toMap().value("logEntry").toMap();
        if (logEntry.value("deviceId").toString() == m_mockDeviceId.toString()) {
            found = true;
            // Make sure the notification contains all the stuff we expect
            QCOMPARE(logEntry.value("typeId").toString(), mockActionIdFailing.toString());
            QCOMPARE(logEntry.value("eventType").toString(), JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
            QCOMPARE(logEntry.value("source").toString(), JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
            QCOMPARE(logEntry.value("loggingLevel").toString(), JsonTypes::loggingLevelToString(Logging::LoggingLevelAlert));
            QCOMPARE(logEntry.value("errorCode").toString(), JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorSetupFailed));
            break;
        }
    }
    if (!found)
        qDebug() << QJsonDocument::fromVariant(loggEntryAddedVariants).toJson();

    QVERIFY2(found, "Could not find the corresponding Logging.LogEntryAdded notification");

    // get this logentry with filter
    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("values", QVariantList() << "7, true");

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QCOMPARE(logEntries.count(), 1);

    // check different filters
    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("typeIds", QVariantList() << mockActionIdNoParams);

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QCOMPARE(logEntries.count(), 1);

    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("typeIds", QVariantList() << mockActionIdNoParams << mockActionIdWithParams << mockActionIdFailing);

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() == 3);

    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

void TestLogging::removeDevice()
{
    // enable notifications
    QCOMPARE(enableNotifications(), true);

    // get this logentry with filter
    QVariantMap params;
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    QVariant response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);
    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() > 0);

    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Remove the device
    params.clear();
    params.insert("deviceId", m_mockDeviceId);
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response);

    clientSpy.wait(200);
    QVariant notification = checkNotification(clientSpy, "Logging.LogDatabaseUpdated");
    QVERIFY(!notification.isNull());

    // verify that the logs from this device where removed from the db
    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);
    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QCOMPARE(logEntries.count(), 0);

    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

#include "testlogging.moc"
QTEST_MAIN(TestLogging)

