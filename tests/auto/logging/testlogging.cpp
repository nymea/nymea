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

    void eventLogs();
    void actionLog();

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
}

void TestLogging::eventLogs()
{
    QList<Device*> devices = GuhCore::instance()->findConfiguredDevices(mockDeviceClassId);
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
    clientSpy.wait(200);
    QVariant notification = checkNotification(clientSpy, "Logging.LogEntryAdded");
    QVERIFY(!notification.isNull());
    reply->deleteLater();

    QVariantMap logEntry = notification.toMap().value("params").toMap().value("logEntry").toMap();

    // Make sure the notification contains all the stuff we expect
    QCOMPARE(logEntry.value("typeId").toString(), mockEvent1Id.toString());
    QCOMPARE(logEntry.value("deviceId").toString(), device->id().toString());
    QCOMPARE(logEntry.value("eventType").toString(), JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    QCOMPARE(logEntry.value("source").toString(), JsonTypes::loggingSourceToString(Logging::LoggingSourceEvents));
    QCOMPARE(logEntry.value("loggingLevel").toString(), JsonTypes::loggingLevelToString(Logging::LoggingLevelInfo));

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

    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // EXECUTE with params
    QVariant response = injectAndWait("Actions.ExecuteAction", params);
    verifyDeviceError(response);

    // Lets wait for the notification
    clientSpy.wait(200);
    QVariant notification = checkNotification(clientSpy, "Logging.LogEntryAdded");
    QVERIFY(!notification.isNull());

    QVariantMap logEntry = notification.toMap().value("params").toMap().value("logEntry").toMap();

    qDebug() << logEntry;

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

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() == 1);

    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("typeIds", QVariantList() << mockActionIdNoParams);

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() == 1);

    params.clear();
    params.insert("deviceIds", QVariantList() << m_mockDeviceId);
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceActions));
    params.insert("eventTypes", QVariantList() << JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    params.insert("typeIds", QVariantList() << mockActionIdNoParams << mockActionIdWithParams);

    response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() == 2);

}

#include "testlogging.moc"
QTEST_MAIN(TestLogging)

