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
#include "nymeacore.h"
#include "devicemanager.h"
#include "nymeasettings.h"
#include "logging/logentry.h"
#include "logging/logvaluetool.h"
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

    void databaseSerializationTest_data();
    void databaseSerializationTest();

    void coverageCalls();

    void systemLogs();

    void invalidFilter_data();
    void invalidFilter();

    void eventLogs();

    void actionLog();

    void deviceLogs();

    void testDoubleValues();

    void testHouseKeeping();


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

void TestLogging::databaseSerializationTest_data()
{
    QUuid uuid = QUuid("3782732b-61b4-48e8-8d6d-b5205159d7cd");

    QVariantMap variantMap;
    variantMap.insert("string", "value");
    variantMap.insert("int", 5);
    variantMap.insert("double", 3.14);
    variantMap.insert("uuid", uuid);

    QVariantList variantList;
    variantList.append(variantMap);
    variantList.append("String");
    variantList.append(3.14);
    variantList.append(uuid);

    QTest::addColumn<QVariant>("value");

    QTest::newRow("QString") << QVariant(QString("Hello"));
    QTest::newRow("Integer") << QVariant((int)2);
    QTest::newRow("Double") << QVariant((double)2.34);
    QTest::newRow("Float") << QVariant((float)2.34);
    QTest::newRow("QColor") << QVariant(QColor(0,255,128));
    QTest::newRow("QByteArray") << QVariant(QByteArray("\nthisisatestarray\n"));
    QTest::newRow("QUuid") << QVariant(uuid);
    QTest::newRow("QVariantMap") << QVariant(variantMap);
    QTest::newRow("QVariantList") << QVariant(variantList);
}

void TestLogging::databaseSerializationTest()
{
    QFETCH(QVariant, value);

    QString serializedValue = LogValueTool::serializeValue(value);
    QVariant deserializedValue = LogValueTool::deserializeValue(serializedValue);

    qDebug() << "Stored:" << value;
    qDebug() << "Loaded:" << deserializedValue;
    QCOMPARE(deserializedValue, value);
}

void TestLogging::coverageCalls()
{
    LogEntry entry(QDateTime::currentDateTime(), Logging::LoggingLevelInfo, Logging::LoggingSourceSystem);
    qDebug() << entry;

    LogFilter filter;
    qDebug() << filter.queryString() << filter.timeFilters();
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
    QList<Device*> devices = NymeaCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId);
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
    clearLoggingDatabase();

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

    // Lets wait for the notification
    clientSpy.wait(500);

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

    loggEntryAddedVariants = checkNotifications(clientSpy, "Logging.LogEntryAdded");
    QVERIFY(!loggEntryAddedVariants.isEmpty());

    // get this logentry with filter
    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));

    // FIXME: currently is filtering for values not supported
    //params.insert("values", QVariantList() << "7, true");

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(!logEntries.isEmpty(), "No logs received");

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

    // FIXME: filter for values currently not working
    //params.insert("values", QVariantList() << "7, true");

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(!logEntries.isEmpty(), "No logs received");

    // check different filters
    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("typeIds", QVariantList() << mockActionIdNoParams);

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(!logEntries.isEmpty(), "No logs received");

    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("typeIds", QVariantList() << mockActionIdNoParams << mockActionIdWithParams << mockActionIdFailing);

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(!logEntries.isEmpty(), "No logs received");

    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

void TestLogging::deviceLogs()
{
    QVariantMap params;
    params.insert("deviceClassId", mockParentDeviceClassId);
    params.insert("name", "Parent device");

    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    verifyDeviceError(response);

    DeviceId deviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!deviceId.isNull());

    // get this logentry with filter
    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId << deviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions)
                  << JsonTypes::loggingSourceToString(Logging::LoggingSourceEvents)
                  << JsonTypes::loggingSourceToString(Logging::LoggingSourceStates));
    params.insert("loggingLevels", QVariantList() << JsonTypes::loggingLevelToString(Logging::LoggingLevelInfo)
                  << JsonTypes::loggingLevelToString(Logging::LoggingLevelAlert));
    params.insert("values", QVariantList() << "7, true" << "9, false");

    QVariantMap timeFilter;
    timeFilter.insert("startDate", QDateTime::currentDateTime().toTime_t() - 5);
    timeFilter.insert("endDate", QDateTime::currentDateTime().toTime_t());

    QVariantMap timeFilter2;
    timeFilter2.insert("endDate", QDateTime::currentDateTime().toTime_t() - 20);

    params.insert("timeFilters", QVariantList() << timeFilter << timeFilter2);

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

}

void TestLogging::testDoubleValues()
{
    QCOMPARE(enableNotifications(), true);

    // Add display pin device which contains a double value

    // Discover device
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", resultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QVariantMap params;
    params.insert("deviceClassId", mockDisplayPinDeviceClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response, DeviceManager::DeviceErrorNoError);

    // Pair device
    DeviceDescriptorId descriptorId = DeviceDescriptorId(response.toMap().value("params").toMap().value("deviceDescriptors").toList().first().toMap().value("id").toString());
    params.clear();
    params.insert("deviceClassId", mockDisplayPinDeviceClassId);
    params.insert("name", "Display pin mock device");
    params.insert("deviceDescriptorId", descriptorId.toString());
    response = injectAndWait("Devices.PairDevice", params);

    verifyDeviceError(response);

    PairingTransactionId pairingTransactionId(response.toMap().value("params").toMap().value("pairingTransactionId").toString());
    QString displayMessage = response.toMap().value("params").toMap().value("displayMessage").toString();

    qDebug() << "displayMessage" << displayMessage;

    params.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());
    params.insert("secret", "243681");
    response = injectAndWait("Devices.ConfirmPairing", params);

    verifyDeviceError(response);

    DeviceId deviceId(response.toMap().value("params").toMap().value("deviceId").toString());

    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Set the double state value and sniff for LogEntryAdded notification
    double value = 23.80;
    QVariantMap actionParam;
    actionParam.insert("paramTypeId", doubleStateParamTypeId.toString());
    actionParam.insert("value", value);

    params.clear(); response.clear();
    params.insert("deviceId", deviceId);
    params.insert("actionTypeId", doubleStateParamTypeId.toString());
    params.insert("params", QVariantList() << actionParam);

    response = injectAndWait("Actions.ExecuteAction", params);
    verifyDeviceError(response);

    notificationSpy.wait(1000);
    QVariantList logNotificationsList = checkNotifications(notificationSpy, "Logging.LogEntryAdded");
    QVERIFY2(!logNotificationsList.isEmpty(), "Did not get Logging.LogEntryAdded notification.");

    foreach (const QVariant &logNotificationVariant, logNotificationsList) {
        QVariantMap logNotification = logNotificationVariant.toMap().value("params").toMap().value("logEntry").toMap();

        if (logNotification.value("typeId").toString() == doubleStateParamTypeId.toString()) {
            if (logNotification.value("typeId").toString() == doubleStateParamTypeId.toString()) {

                // If state source
                if (logNotification.value("source").toString() == JsonTypes::loggingSourceToString(Logging::LoggingSourceStates)) {
                    QString logValue = logNotification.value("value").toString();
                    qDebug() << QString::number(value) << logValue;
                    QCOMPARE(logValue, QString::number(value));
                }

                // If action source notification
                if (logNotification.value("source").toString() == JsonTypes::loggingSourceToString(Logging::LoggingSourceActions)) {
                    QString logValue = logNotification.value("value").toString();
                    qDebug() << QString::number(value) << logValue;
                    QCOMPARE(logValue, QString::number(value));
                }
            }
        }
    }


    // Remove device
    params.clear();
    params.insert("deviceId", deviceId.toString());
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response);
}

void TestLogging::testHouseKeeping()
{
    QVariantMap params;
    params.insert("deviceClassId", mockDeviceClassId);
    params.insert("name", "TestDeviceToBeRemoved");
    QVariantList deviceParams;
    QVariantMap httpParam;
    httpParam.insert("paramTypeId", httpportParamTypeId);
    httpParam.insert("value", 6667);
    deviceParams.append(httpParam);
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    DeviceId deviceId = DeviceId::fromUuid(response.toMap().value("params").toMap().value("deviceId").toUuid());
    QVERIFY2(!deviceId.isNull(), "Something went wrong creating the device for testing.");

    // Trigger something that creates a logging entry
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(6667).arg(mockIntStateId.toString()).arg(4321)));
    QNetworkReply *reply = nam.get(request);
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
    spy.wait();

    params.clear();
    params.insert("deviceIds", QVariantList() << deviceId);
    response = injectAndWait("Logging.GetLogEntries", params);
    QVERIFY2(response.toMap().value("params").toMap().value("logEntries").toList().count() > 0, "Couldn't find state change event in log...");

    // Manually delete this device from config
    NymeaSettings settings(NymeaSettings::SettingsRoleDevices);
    settings.beginGroup("DeviceConfig");
    settings.remove(deviceId.toString());
    settings.endGroup();

    restartServer();

    response = injectAndWait("Logging.GetLogEntries", params);
    QVERIFY2(response.toMap().value("params").toMap().value("logEntries").toList().count() == 0, "Device state change event still in log. Should've been cleaned by housekeeping.");
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

