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
#include "logging/logengine.h"
#include "servers/mocktcpserver.h"

#include "../plugins/mock/extern-plugininfo.h"

#include <qglobal.h>

#include "version.h"

using namespace nymeaserver;

class TestLogging : public NymeaTestBase
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void initLogs();

    void systemLogs();

    void invalidFilter_data();
    void invalidFilter();

    void stateChangeLogs_data();
    void stateChangeLogs();

    void eventLog();

    void actionLog();

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
    QVariantMap params;
    params.insert("sources", QStringList{"core"});
    QVariant response = injectAndWait("Logging.GetLogEntries", params);

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(logEntries.count() > 0,
             QString("Expected at least one log entry.")
             .toUtf8());

    clearLoggingDatabase("core");
    waitForDBSync();

    response = injectAndWait("Logging.GetLogEntries");
    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() == 0);

    restartServer();
}

void TestLogging::systemLogs()
{
    qCDebug(dcTests()) << "Clearing logging DB";

    clearLoggingDatabase("core");

    QVariantMap params;
    params.insert("sources", QStringList{"core"});
    params.insert("sortOrder", enumValueName(Qt::DescendingOrder));

    // there should be 0 log entries
    QVariant response = injectAndWait("Logging.GetLogEntries", params);
    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(logEntries.count() == 0,
             QString("Expected 0 log entries but got:\n%1")
             .arg(QString(QJsonDocument::fromVariant(logEntries).toJson()))
             .toUtf8());

    // check the active system log at boot
    qCDebug(dcTests) << "Restarting server";
    restartServer();
    qCDebug(dcTests) << "Restart done";
    waitForDBSync();

    // there should be 2 log entries, one for shutdown, one for startup (from server restart)
    response = injectAndWait("Logging.GetLogEntries", params);
    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(logEntries.count() == 2,
             QString("Expected 2 log entries but got:\n%1")
             .arg(QString(QJsonDocument::fromVariant(logEntries).toJson()))
             .toUtf8());

    QVariantMap logEntryStartup = logEntries.first().toMap();
    QVariantMap logEntryShutdown = logEntries.last().toMap();

    QCOMPARE(logEntryShutdown.value("values").toMap().value("event").toString(), QString("stopped"));
    QCOMPARE(logEntryShutdown.value("values").toMap().value("shutdownReason").toString(), enumValueName(NymeaCore::ShutdownReasonRestart));
    QCOMPARE(logEntryShutdown.value("values").toMap().value("version").toString(), QString(NYMEA_VERSION_STRING));


    QCOMPARE(logEntryStartup.value("values").toMap().value("event").toString(), QString("started"));
    QCOMPARE(logEntryStartup.value("values").toMap().value("version").toString(), QString(NYMEA_VERSION_STRING));
}

void TestLogging::invalidFilter_data()
{
    QVariantMap invalidSourcesFilter;
    invalidSourcesFilter.insert("sources", QStringList{"bla"});

    QTest::addColumn<QVariantMap>("filter");

    QTest::newRow("Invalid source") << invalidSourcesFilter;
}

void TestLogging::invalidFilter()
{
    QFETCH(QVariantMap, filter);
    QVariant response = injectAndWait("Logging.GetLogEntries", filter);
    QVERIFY(!response.isNull());

    // verify json error
    QVERIFY(response.toMap().value("status").toString() == "success");
    QVERIFY(response.toMap().value("params").toMap().contains("logEntries"));
    QVERIFY(response.toMap().value("params").toMap().value("logEntries").toList().isEmpty());
    qDebug() << response.toMap().value("error").toString();
}

void TestLogging::stateChangeLogs_data()
{
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<QString>("stateName");
    QTest::addColumn<QVariant>("initValue");
    QTest::addColumn<QVariant>("newValue");
    QTest::addColumn<bool>("expectLogEntry");

    QTest::newRow("logged state") << mockConnectedStateTypeId << "connected" << QVariant(false) << QVariant(true) << true;
    QTest::newRow("not logged state") << mockDoubleStateTypeId << "double" << QVariant(10) << QVariant(20) << false;
}

void TestLogging::stateChangeLogs()
{
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(QString, stateName);
    QFETCH(QVariant, initValue);
    QFETCH(QVariant, newValue);
    QFETCH(bool, expectLogEntry);

    QList<Thing*> things = NymeaCore::instance()->thingManager()->findConfiguredThings(mockThingClassId);
    QVERIFY2(things.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Thing *thing = things.first();

    // Setup connection to mock client
    QNetworkAccessManager nam;

    int port = thing->paramValue(mockThingHttpportParamTypeId).toInt();

    QString logSourceName = "state-" + thing->id().toString() + "-" + stateName;

    // init state in mock device
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(port).arg(stateTypeId.toString()).arg(initValue.toString())));
    QNetworkReply *reply = nam.get(request);
    {
    QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
    finishedSpy.wait();
    }

    waitForDBSync();

    // Now snoop in for the events
    clearLoggingDatabase(logSourceName);

    enableNotifications({"Integrations"});
    QSignalSpy clientSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

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

    QVariantList stateChangeNotification = checkNotifications(clientSpy, "Integrations.StateChanged");

    waitForDBSync();

    QVariant response = injectAndWait("Logging.GetLogEntries", {{"sources", QStringList{logSourceName}}});
    QVERIFY(!response.isNull());

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(logEntries.count() == (expectLogEntry ? 1 : 0), "Unexpected amount of log entries in DB");

    if (expectLogEntry) {
        QVariantMap entry = logEntries.first().toMap();
        QVERIFY2(entry.value("values").toMap().value(stateName) == QVariant(newValue), "Log entry value not matching");
    }

    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

void TestLogging::eventLog()
{
    QList<Thing*> things = NymeaCore::instance()->thingManager()->findConfiguredThings(mockThingClassId);
    QVERIFY2(things.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Thing *thing = things.first();

    EventTypeId eventTypeId = mockEvent2EventTypeId;
    QString eventName = "event2";

    QString logSourceName = "event-" + thing->id().toString() + "-" + eventName;
    clearLoggingDatabase(logSourceName);

    enableNotifications({"Integrations"});
    QSignalSpy clientSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    // trigger state change in mock device
    QNetworkAccessManager nam;
    int port = thing->paramValue(mockThingHttpportParamTypeId).toInt();
    QNetworkRequest request = QNetworkRequest(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2&%3=%4").arg(port).arg(eventTypeId.toString()).arg(mockEvent2EventIntParamParamTypeId.toString()).arg(42)));
    QNetworkReply *reply = nam.get(request);
    {
    QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
    finishedSpy.wait();
    }

    clientSpy.wait();
    reply->deleteLater();

    QVariantList stateChangeNotification = checkNotifications(clientSpy, "Integrations.EventTriggered");

    waitForDBSync();

    QVariant response = injectAndWait("Logging.GetLogEntries", {{"sources", QStringList{logSourceName}}});
    QVERIFY(!response.isNull());

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(logEntries.count() == 1, "Unexpected amount of log entries in DB");

    QVariantMap entry = logEntries.first().toMap();
    QCOMPARE(entry.value("source").toString(), logSourceName);
    QByteArray paramsJson = entry.value("values").toMap().value("params").toByteArray();
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(paramsJson, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QCOMPARE(jsonDoc.toVariant().toMap().value("intParam").toInt(), 42);

    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

void TestLogging::actionLog()
{
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

    QSignalSpy clientSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    // EXECUTE with params
    QVariant response = injectAndWait("Integrations.ExecuteAction", params);

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
        if (logEntry.value("source").toString() == "action-" + m_mockThingId.toString() + "-withParams" ) {
            found = true;
            QCOMPARE(logEntry.value("values").toMap().value("status").toString(), enumValueName(Thing::ThingErrorNoError));
            QCOMPARE(logEntry.value("values").toMap().value("triggeredBy").toString(), enumValueName(Action::TriggeredByUser));
            QJsonParseError error;
            QVariantMap actionParams = QJsonDocument::fromJson(logEntry.value("values").toMap().value("params").toByteArray(), &error).toVariant().toMap();
            QCOMPARE(error.error, QJsonParseError::NoError);
            QCOMPARE(actionParams.value("param1").toInt(), 7);
            QCOMPARE(actionParams.value("param2").toBool(), true);
        }
    }

    QVERIFY2(found, "Could not find the corresponding Logging.LogEntryAdded notification");

    // EXECUTE without params
    params.clear(); clientSpy.clear();
    params.insert("actionTypeId", mockWithoutParamsActionTypeId);
    params.insert("thingId", m_mockThingId);
    response = injectAndWait("Integrations.ExecuteAction", params);

    clientSpy.wait();

    logEntryAddedVariants = checkNotifications(clientSpy, "Logging.LogEntryAdded");
    QVERIFY(!logEntryAddedVariants.isEmpty());

    // get this logentry with filter
    params.clear();
    params.insert("sources", QStringList{"action-" + m_mockThingId.toString() + "-withoutParams"});
    response = injectAndWait("Logging.GetLogEntries", params);

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(!logEntries.isEmpty(), "No logs received");

    // EXECUTE broken action
    params.clear(); clientSpy.clear();
    params.insert("actionTypeId", mockFailingActionTypeId);
    params.insert("thingId", m_mockThingId);
    response = injectAndWait("Integrations.ExecuteAction", params);

    clientSpy.wait();

    logEntryAddedVariants = checkNotifications(clientSpy, "Logging.LogEntryAdded");
    QVERIFY2(!logEntryAddedVariants.isEmpty(), "Did not get Logging.LogEntryAdded notification.");

    found = false;
    foreach (const QVariant &loggEntryAddedVariant, logEntryAddedVariants) {
        QVariantMap logEntry = loggEntryAddedVariant.toMap().value("params").toMap().value("logEntry").toMap();
        if (logEntry.value("source").toString() == "action-" + m_mockThingId.toString() + "-failing") {
            found = true;
            QCOMPARE(logEntry.value("values").toMap().value("status").toString(), enumValueName(Thing::ThingErrorSetupFailed));
            QCOMPARE(logEntry.value("values").toMap().value("triggeredBy").toString(), enumValueName(Action::TriggeredByUser));
            QJsonParseError error;
            QVariantMap actionParams = QJsonDocument::fromJson(logEntry.value("values").toMap().value("params").toByteArray(), &error).toVariant().toMap();
            QCOMPARE(error.error, QJsonParseError::NoError);
            QVERIFY(actionParams.isEmpty());
            break;
        }
    }

    QVERIFY2(found, "Could not find the corresponding Logging.LogEntryAdded notification");

    // get this logentry with filter
    params.clear();
    params.insert("sources", QStringList{"action-" + m_mockThingId.toString() + "-failing"});
    response = injectAndWait("Logging.GetLogEntries", params);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY2(!logEntries.isEmpty(), "No logs received");

    // Get all action logs in one go
    params.clear();
    params.insert("sources", QStringList{
                      "action-" + m_mockThingId.toString() + "-withParams",
                      "action-" + m_mockThingId.toString() + "-withoutParams",
                      "action-" + m_mockThingId.toString() + "-failing"
                  });

    response = injectAndWait("Logging.GetLogEntries", params);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QCOMPARE(logEntries.count(), 3);

    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

void TestLogging::removeThing()
{
    QList<Thing*> things = NymeaCore::instance()->thingManager()->findConfiguredThings(mockThingClassId);
    QVERIFY2(things.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Thing *thing = things.first();

    QString stateName = "int";
    StateTypeId stateTypeId = mockIntStateTypeId;

    qCDebug(dcTests) << "Using mock:" << things.first()->id();


    // Setup connection to mock client
    QNetworkAccessManager nam;

    int port = thing->paramValue(mockThingHttpportParamTypeId).toInt();

    QString logSourceName = "state-" + thing->id().toString() + "-" + stateName;

    // init state in mock device
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(port).arg(stateTypeId.toString()).arg("12")));
    QNetworkReply *reply = nam.get(request);
    {
    QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
    finishedSpy.wait();
    }

    waitForDBSync();

    // enable notifications
    enableNotifications({"Logging"});

    // get this logentry with filter
    QVariant response = injectAndWait("Logging.GetLogEntries", {{"sources", QStringList{logSourceName}}});
    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() > 0);

    // Remove the device
    response = injectAndWait("Integrations.RemoveThing", {{"thingId", m_mockThingId}});

    waitForDBSync();

    // verify that the logs from this device where removed from the db
    response = injectAndWait("Logging.GetLogEntries", {{"sources", QStringList{logSourceName}}});
    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QCOMPARE(logEntries.count(), 0);

    // disable notifications
    QCOMPARE(disableNotifications(), true);
}

#include "testlogging.moc"
QTEST_MAIN(TestLogging)

