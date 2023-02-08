/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeatestbase.h"
#include "nymeacore.h"
#include "nymeasettings.h"
#include "logging/logvaluetool.h"
#include "servers/mocktcpserver.h"

#include "../plugins/mock/extern-plugininfo.h"

#include <qglobal.h>

using namespace nymeaserver;

class TestLogging : public NymeaTestBase
{
    Q_OBJECT

private:

    inline void verifyLoggingError(const QVariant &response, Logging::LoggingError error = Logging::LoggingErrorNoError) {
        verifyError(response, "loggingError", enumValueName(error));
    }
    inline void verifyThingError(const QVariant &response, Thing::ThingError error = Thing::ThingErrorNoError) {
        verifyError(response, "thingError", enumValueName(error));
    }
    // DEPRECTATED
    inline void verifyDeviceError(const QVariant &response, Thing::ThingError error = Thing::ThingErrorNoError) {
        verifyError(response, "deviceError", enumValueName(error).replace("Thing", "Device"));
    }

private slots:
    void initTestCase();
    void init();

    void initLogs();

    void databaseSerializationTest_data();
    void databaseSerializationTest();

    void coverageCalls();

    void systemLogs();

    void invalidFilter_data();
    void invalidFilter();

    void eventLogs_data();
    void eventLogs();

    void actionLog();

    void thingLogs();

    void testDoubleValues();

    void testHouseKeeping();

    void testLimits();

    // this has to be the last test
    void removeThing();
};

void TestLogging::initTestCase()
{
    NymeaTestBase::initTestCase();
    QLoggingCategory::setFilterRules("*.debug=false\n"
                                     "LogEngine.debug=true\n"
                                     "Tests.debug=true\n"
                                     "MockDevice.debug=true\n"
                                     "DeviceManager.debug=true\n"
                                     "Application.debug=true\n");
}

void TestLogging::init()
{
    waitForDBSync();
}

void TestLogging::initLogs()
{
    QVariant response = injectAndWait("Logging.GetLogEntries");
    verifyLoggingError(response);

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(logEntries.count() > 0,
             QString("Expected at least one log entry.")
             .toUtf8());

    clearLoggingDatabase();
    waitForDBSync();

    response = injectAndWait("Logging.GetLogEntries");
    verifyLoggingError(response);
    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() == 0);

    restartServer();
    NymeaCore::instance()->logEngine()->setMaxLogEntries(1000, 10);
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
    qWarning() << "Clearing logging DB";
    clearLoggingDatabase();

    waitForDBSync();

    QVariantMap params;
    params.insert("loggingSources", QVariantList() << enumValueName(Logging::LoggingSourceSystem));
    params.insert("eventTypes", QVariantList() << enumValueName(Logging::LoggingEventTypeActiveChange));

    // there should be 0 log entries
    QVariant response = injectAndWait("Logging.GetLogEntries", params);
    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(logEntries.count() == 0,
             QString("Expected 0 log entries but got:\n%1")
             .arg(QString(QJsonDocument::fromVariant(logEntries).toJson()))
             .toUtf8());

    // check the active system log at boot
    qWarning() << "Restarting server";
    restartServer();
    qWarning() << "Restart done";
    waitForDBSync();

    // there should be 2 log entries, one for shutdown, one for startup (from server restart)
    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);
    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(logEntries.count() == 2,
             QString("Expected 2 log entries but got:\n%1")
             .arg(QString(QJsonDocument::fromVariant(logEntries).toJson()))
             .toUtf8());

    QVariantMap logEntryStartup = logEntries.first().toMap();
    QVariantMap logEntryShutdown = logEntries.last().toMap();
    // We cannot rely on the order those events
    if (!logEntryStartup.value("active").toBool()) {
        logEntryStartup = logEntries.last().toMap();
        logEntryShutdown = logEntries.first().toMap();
    }

    QCOMPARE(logEntryShutdown.value("active").toBool(), false);
    QCOMPARE(logEntryShutdown.value("eventType").toString(), enumValueName(Logging::LoggingEventTypeActiveChange));
    QCOMPARE(logEntryShutdown.value("source").toString(), enumValueName(Logging::LoggingSourceSystem));
    QCOMPARE(logEntryShutdown.value("loggingLevel").toString(), enumValueName(Logging::LoggingLevelInfo));


    QCOMPARE(logEntryStartup.value("active").toBool(), true);
    QCOMPARE(logEntryStartup.value("eventType").toString(), enumValueName(Logging::LoggingEventTypeActiveChange));
    QCOMPARE(logEntryStartup.value("source").toString(), enumValueName(Logging::LoggingSourceSystem));
    QCOMPARE(logEntryStartup.value("loggingLevel").toString(), enumValueName(Logging::LoggingLevelInfo));
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
    invalidEventTypes.insert("eventTypes", QVariantList() << enumValueName(Logging::LoggingEventTypeTrigger) << "blub");

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

void TestLogging::eventLogs_data()
{
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<QVariant>("initValue");
    QTest::addColumn<QVariant>("newValue");
    QTest::addColumn<bool>("expectLogEntry");

    QTest::newRow("logged event") << mockConnectedStateTypeId << QVariant(false) << QVariant(true) << true;
    QTest::newRow("not logged event") << mockSignalStrengthStateTypeId << QVariant(10) << QVariant(20) << false;
}

void TestLogging::eventLogs()
{
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(QVariant, initValue);
    QFETCH(QVariant, newValue);
    QFETCH(bool, expectLogEntry);

    QList<Thing*> devices = NymeaCore::instance()->thingManager()->findConfiguredThings(mockThingClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Thing *device = devices.first();

    // Setup connection to mock client
    QNetworkAccessManager nam;

    int port = device->paramValue(mockThingHttpportParamTypeId).toInt();

    // init state in mock device
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(port).arg(stateTypeId.toString()).arg(initValue.toString())));
    QNetworkReply *reply = nam.get(request);
    {
    QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
    finishedSpy.wait();
    }

    // Now snoop in for the events
    clearLoggingDatabase();
    enableNotifications({"Integrations", "Logging"});
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // trigger state change in mock device
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(port).arg(stateTypeId.toString()).arg(newValue.toString())));
    reply = nam.get(request);
    {
    QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
    finishedSpy.wait();
    }

    // Lets wait for the notification
    QTest::qWait(200);
    clientSpy.wait(1000);
    reply->deleteLater();

    // Make sure the logg notification contains all the stuff we expect
    QVariantList logEntryAddedVariants = checkNotifications(clientSpy, "Logging.LogEntryAdded");
    qDebug() << "got" << logEntryAddedVariants.count() << "Logging.LogEntryAdded notifications";

    bool found = false;
    qDebug() << "got" << logEntryAddedVariants.count() << "Logging.LogEntryAdded";
    foreach (const QVariant &logEntryAddedVariant, logEntryAddedVariants) {
        QVariantMap logEntry = logEntryAddedVariant.toMap().value("params").toMap().value("logEntry").toMap();
        if (logEntry.value("thingId").toUuid() == device->id()) {
            found = true;
            // Make sure the notification contains all the stuff we expect
            QCOMPARE(logEntry.value("typeId").toUuid().toString(), stateTypeId.toString());
            QCOMPARE(logEntry.value("eventType").toString(), enumValueName(Logging::LoggingEventTypeTrigger));
            QCOMPARE(logEntry.value("source").toString(), enumValueName(Logging::LoggingSourceStates));
            QCOMPARE(logEntry.value("loggingLevel").toString(), enumValueName(Logging::LoggingLevelInfo));
            break;
        }
    }

    QVERIFY2(found == expectLogEntry, "Could not find the corresponding Logging.LogEntryAdded notification");

    if (expectLogEntry) {
        // get this logentry with filter
        QVariantMap params;
        params.insert("thingIds", QVariantList() << device->id());
        params.insert("loggingSources", QVariantList() << enumValueName(Logging::LoggingSourceStates));
        params.insert("eventTypes", QVariantList() << enumValueName(Logging::LoggingEventTypeTrigger));
        params.insert("typeIds", QVariantList() << stateTypeId);

        QVariant response = injectAndWait("Logging.GetLogEntries", params);
        verifyLoggingError(response);

        QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
        qCDebug(dcTests()) << qUtf8Printable(QJsonDocument::fromVariant(logEntries).toJson());
        QCOMPARE(logEntries.count(), 1);
    }


    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

void TestLogging::actionLog()
{
    clearLoggingDatabase();

    QVariantList actionParams;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 7);
    actionParams.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    actionParams.append(param2);

    QVariantMap params;
    params.insert("actionTypeId", mockWithParamsActionTypeId);
    params.insert("thingId", m_mockThingId);
    params.insert("params", actionParams);

    enableNotifications({"Logging"});

    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // EXECUTE with params
    QVariant response = injectAndWait("Integrations.ExecuteAction", params);
    verifyThingError(response);

    // wait for the outgoing data
    // 3 packets: ExecuteAction reply,  LogDatabaseUpdated signal and LogEntryAdded signal
    while (clientSpy.count() < 3) {
        bool success = clientSpy.wait();
        if (!success) {
            break;
        }
    }

    QVariantList logEntryAddedVariants = checkNotifications(clientSpy, "Logging.LogEntryAdded");
    qWarning() << QJsonDocument::fromVariant(logEntryAddedVariants).toJson();
    QVERIFY2(!logEntryAddedVariants.isEmpty(), "Did not get Logging.LogEntryAdded notification.");

    bool found = false;
    foreach (const QVariant &loggEntryAddedVariant, logEntryAddedVariants) {
        QVariantMap logEntry = loggEntryAddedVariant.toMap().value("params").toMap().value("logEntry").toMap();
        if (logEntry.value("thingId").toUuid() == m_mockThingId) {
            found = true;
            // Make sure the notification contains all the stuff we expect
            QCOMPARE(logEntry.value("typeId").toUuid().toString(), mockWithParamsActionTypeId.toString());
            QCOMPARE(logEntry.value("eventType").toString(), enumValueName(Logging::LoggingEventTypeTrigger));
            QCOMPARE(logEntry.value("source").toString(), enumValueName(Logging::LoggingSourceActions));
            QCOMPARE(logEntry.value("loggingLevel").toString(), enumValueName(Logging::LoggingLevelInfo));
            break;
        }
    }

    QVERIFY2(found, "Could not find the corresponding Logging.LogEntryAdded notification");

    // EXECUTE without params
    params.clear(); clientSpy.clear();
    params.insert("actionTypeId", mockWithoutParamsActionTypeId);
    params.insert("thingId", m_mockThingId);
    response = injectAndWait("Integrations.ExecuteAction", params);
    verifyThingError(response);

    clientSpy.wait(200);

    logEntryAddedVariants = checkNotifications(clientSpy, "Logging.LogEntryAdded");
    QVERIFY(!logEntryAddedVariants.isEmpty());

    // get this logentry with filter
    params.clear();
    params.insert("thingIds", QVariantList() << m_mockThingId);
    params.insert("loggingSources", QVariantList() << enumValueName(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << enumValueName(Logging::LoggingEventTypeTrigger));

    // FIXME: currently is filtering for values not supported
    //params.insert("values", QVariantList() << "7, true");

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(!logEntries.isEmpty(), "No logs received");

    // EXECUTE broken action
    params.clear(); clientSpy.clear();
    params.insert("actionTypeId", mockFailingActionTypeId);
    params.insert("thingId", m_mockThingId);
    response = injectAndWait("Integrations.ExecuteAction", params);
    verifyThingError(response, Thing::ThingErrorSetupFailed);

    clientSpy.wait(200);

    logEntryAddedVariants = checkNotifications(clientSpy, "Logging.LogEntryAdded");
    QVERIFY2(!logEntryAddedVariants.isEmpty(), "Did not get Logging.LogEntryAdded notification.");

    found = false;
    foreach (const QVariant &loggEntryAddedVariant, logEntryAddedVariants) {
        QVariantMap logEntry = loggEntryAddedVariant.toMap().value("params").toMap().value("logEntry").toMap();
        if (logEntry.value("thingId").toUuid() == m_mockThingId) {
            found = true;
            // Make sure the notification contains all the stuff we expect
            QCOMPARE(logEntry.value("typeId").toUuid().toString(), mockFailingActionTypeId.toString());
            QCOMPARE(logEntry.value("eventType").toString(), enumValueName(Logging::LoggingEventTypeTrigger));
            QCOMPARE(logEntry.value("source").toString(), enumValueName(Logging::LoggingSourceActions));
            QCOMPARE(logEntry.value("loggingLevel").toString(), enumValueName(Logging::LoggingLevelAlert));
            QCOMPARE(logEntry.value("errorCode").toString(), enumValueName(Thing::ThingErrorSetupFailed));
            break;
        }
    }

    QVERIFY2(found, "Could not find the corresponding Logging.LogEntryAdded notification");

    // get this logentry with filter
    params.clear();
    params.insert("thingIds", QVariantList() << m_mockThingId);
    params.insert("loggingSources", QVariantList() << enumValueName(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << enumValueName(Logging::LoggingEventTypeTrigger));

    // FIXME: filter for values currently not working
    //params.insert("values", QVariantList() << "7, true");

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(!logEntries.isEmpty(), "No logs received");

    // check different filters
    params.clear();
    params.insert("thingIds", QVariantList() << m_mockThingId);
    params.insert("loggingSources", QVariantList() << enumValueName(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << enumValueName(Logging::LoggingEventTypeTrigger));
    params.insert("typeIds", QVariantList() << mockWithoutParamsActionTypeId);

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(!logEntries.isEmpty(), "No logs received");

    params.clear();
    params.insert("thingIds", QVariantList() << m_mockThingId);
    params.insert("loggingSources", QVariantList() << enumValueName(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << enumValueName(Logging::LoggingEventTypeTrigger));
    params.insert("typeIds", QVariantList() << mockWithoutParamsActionTypeId << mockWithParamsActionTypeId << mockFailingActionTypeId);

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(!logEntries.isEmpty(), "No logs received");

    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

void TestLogging::thingLogs()
{
    QVariantMap params;
    params.insert("thingClassId", parentMockThingClassId);
    params.insert("name", "Parent thing");

    QVariant response = injectAndWait("Integrations.AddThing", params);
    verifyThingError(response);

    ThingId thingId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY(!thingId.isNull());

    // get this logentry with filter
    params.clear();
    params.insert("thingIds", QVariantList() << m_mockThingId << thingId);
    params.insert("loggingSources", QVariantList() << enumValueName(Logging::LoggingSourceActions)
                  << enumValueName(Logging::LoggingSourceEvents)
                  << enumValueName(Logging::LoggingSourceStates));
    params.insert("loggingLevels", QVariantList() << enumValueName(Logging::LoggingLevelInfo)
                  << enumValueName(Logging::LoggingLevelAlert));
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
    enableNotifications({"Logging"});

    // Add display pin device which contains a double value

    // Discover device
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", displayPinMockDiscoveryResultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QVariantMap params;
    params.insert("thingClassId", displayPinMockThingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Integrations.DiscoverThings", params);

    verifyThingError(response, Thing::ThingErrorNoError);

    // Pair device
    ThingDescriptorId descriptorId = ThingDescriptorId(response.toMap().value("params").toMap().value("thingDescriptors").toList().first().toMap().value("id").toString());
    params.clear();
    params.insert("thingClassId", displayPinMockThingClassId);
    params.insert("name", "Display pin mock device");
    params.insert("thingDescriptorId", descriptorId.toString());
    response = injectAndWait("Integrations.PairThing", params);

    verifyThingError(response);

    PairingTransactionId pairingTransactionId(response.toMap().value("params").toMap().value("pairingTransactionId").toString());
    QString displayMessage = response.toMap().value("params").toMap().value("displayMessage").toString();

    qCDebug(dcTests) << "displayMessage" << displayMessage;

    params.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());
    params.insert("secret", "243681");
    response = injectAndWait("Integrations.ConfirmPairing", params);

    verifyThingError(response);

    ThingId thingId(response.toMap().value("params").toMap().value("thingId").toString());

    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Set the double state value and sniff for LogEntryAdded notification
    double value = 23.80;
    QVariantMap actionParam;
    actionParam.insert("paramTypeId", displayPinMockDoubleActionDoubleParamTypeId.toString());
    actionParam.insert("value", value);

    params.clear(); response.clear();
    params.insert("thingId", thingId);
    params.insert("actionTypeId", displayPinMockDoubleActionTypeId.toString());
    params.insert("params", QVariantList() << actionParam);

    response = injectAndWait("Integrations.ExecuteAction", params);
    verifyThingError(response);

    notificationSpy.wait();
    QVariantList logNotificationsList = checkNotifications(notificationSpy, "Logging.LogEntryAdded");
    QVERIFY2(!logNotificationsList.isEmpty(), "Did not get Logging.LogEntryAdded notification.");

    foreach (const QVariant &logNotificationVariant, logNotificationsList) {
        QVariantMap logNotification = logNotificationVariant.toMap().value("params").toMap().value("logEntry").toMap();

        if (logNotification.value("typeId").toString() == displayPinMockDoubleActionDoubleParamTypeId.toString()) {
            if (logNotification.value("typeId").toString() == displayPinMockDoubleActionDoubleParamTypeId.toString()) {

                // If state source
                if (logNotification.value("source").toString() == enumValueName(Logging::LoggingSourceStates)) {
                    QString logValue = logNotification.value("value").toString();
                    qDebug() << QString::number(value) << logValue;
                    QCOMPARE(logValue, QString::number(value));
                }

                // If action source notification
                if (logNotification.value("source").toString() == enumValueName(Logging::LoggingSourceActions)) {
                    QString logValue = logNotification.value("value").toString();
                    qDebug() << QString::number(value) << logValue;
                    QCOMPARE(logValue, QString::number(value));
                }
            }
        }
    }


    // Remove device
    params.clear();
    params.insert("thingId", thingId.toString());
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response);
}

void TestLogging::testHouseKeeping()
{
    QVariantMap params;
    params.insert("thingClassId", mockThingClassId);
    params.insert("name", "TestDeviceToBeRemoved");
    QVariantList thingParams;
    QVariantMap httpParam;
    httpParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpParam.insert("value", 6667);
    thingParams.append(httpParam);
    params.insert("thingParams", thingParams);
    QVariant response = injectAndWait("Integrations.AddThing", params);
    ThingId thingId = ThingId(response.toMap().value("params").toMap().value("thingId").toUuid());
    QVERIFY2(!thingId.isNull(), "Something went wrong creating the thing for testing.");

    // Trigger something that creates a logging entry
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(6667).arg(mockConnectedStateTypeId.toString()).arg(false)));
    QNetworkReply *reply = nam.get(request);
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
    spy.wait();

    waitForDBSync();

    params.clear();
    params.insert("thingIds", QVariantList() << thingId);
    response = injectAndWait("Logging.GetLogEntries", params);
    QVERIFY2(response.toMap().value("params").toMap().value("logEntries").toList().count() > 0, "Couldn't find state change event in log...");

    // Manually delete this device from config
    NymeaSettings settings(NymeaSettings::SettingsRoleThings);
    settings.beginGroup("ThingConfig");
    settings.remove(thingId.toString());
    settings.endGroup();

    restartServer();

    waitForDBSync();

    params.clear();
    params.insert("thingIds", QVariantList() << thingId);
    response = injectAndWait("Logging.GetLogEntries", params);
    qCDebug(dcTests()) << qUtf8Printable(QJsonDocument::fromVariant(response).toJson());
    QVERIFY2(response.toMap().value("status").toString() == QString("success"), "GetLogEntries failed");
    QVERIFY2(response.toMap().value("params").toMap().value("logEntries").toList().count() == 0, "Device state change event still in log. Should've been cleaned by housekeeping.");
}

void TestLogging::testLimits()
{
    clearLoggingDatabase();

    for (int i = 0; i < 50; i++) {
        QVariantList actionParams;
        QVariantMap param1;
        param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
        param1.insert("value", i);
        actionParams.append(param1);
        QVariantMap param2;
        param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
        param2.insert("value", true);
        actionParams.append(param2);

        QVariantMap params;
        params.insert("actionTypeId", mockWithParamsActionTypeId);
        params.insert("thingId", m_mockThingId);
        params.insert("params", actionParams);

        // EXECUTE with params
        QVariant response = injectAndWait("Integrations.ExecuteAction", params);
        verifyThingError(response);
    }

    waitForDBSync();

    QVariantMap params;
    QVariantMap response;

    // No limits, should be all 50 entries
    params.clear();
    response = injectAndWait("Logging.GetLogEntries", params).toMap();
    QCOMPARE(response.value("params").toMap().value("count").toInt(), 50);
    QCOMPARE(response.value("params").toMap().value("logEntries").toList().count(), 50);

    // Add a limit of 20
    params.clear();
    params.insert("limit", 20);
    response = injectAndWait("Logging.GetLogEntries", params).toMap();
    QCOMPARE(response.value("params").toMap().value("count").toInt(), 20);
    QCOMPARE(response.value("params").toMap().value("logEntries").toList().count(), 20);

    // Add a offset of 40, keeping a limit of 20. should return 10 entries
    params.clear();
    params.insert("limit", 20);
    params.insert("offset", 40);
    response = injectAndWait("Logging.GetLogEntries", params).toMap();
    QCOMPARE(response.value("params").toMap().value("count").toInt(), 10);
    QCOMPARE(response.value("params").toMap().value("logEntries").toList().count(), 10);
}

void TestLogging::removeThing()
{
    // enable notifications
    enableNotifications({"Logging"});

    // get this logentry with filter
    QVariantMap params;
    params.insert("thingIds", QVariantList() << m_mockThingId);
    QVariant response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);
    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() > 0);

    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Remove the device
    params.clear();
    params.insert("thingId", m_mockThingId);
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response);

    clientSpy.wait(200);
    QVariant notification = checkNotification(clientSpy, "Logging.LogDatabaseUpdated");
    QVERIFY(!notification.isNull());

    // verify that the logs from this device where removed from the db
    params.clear();
    params.insert("thingIds", QVariantList() << m_mockThingId);
    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);
    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QCOMPARE(logEntries.count(), 0);

    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

#include "testlogging.moc"
QTEST_MAIN(TestLogging)

