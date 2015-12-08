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

    void actionLog_data();
    void actionLog();


};



void TestLogging::initLogs()
{
    QVariant response = injectAndWait("Logging.GetLogEntries");
    verifyLoggingError(response);

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    qDebug() << "Got" << logEntries.count() << "logs";
    QVERIFY(logEntries.count() > 0);

//    foreach (const QVariant &logEntryVariant, logEntries) {
//        qDebug() << QJsonDocument::fromVariant(logEntryVariant).toJson();
//    }

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
    clientSpy.wait(1000);
    QVariant notification = checkNotification(clientSpy, "Logging.LogEntryAdded");
    QVERIFY(!notification.isNull());
    reply->deleteLater();

    QVariantMap logEntry = notification.toMap().value("params").toMap().value("logEntry").toMap();

    // Make sure the event contains all the stuff we expect
    QCOMPARE(logEntry.value("typeId").toString(), mockEvent1Id.toString());
    QCOMPARE(logEntry.value("deviceId").toString(), device->id().toString());
    QCOMPARE(logEntry.value("eventType").toString(), JsonTypes::loggingEventTypeToString(Logging::LoggingEventTypeTrigger));
    QCOMPARE(logEntry.value("source").toString(), JsonTypes::loggingSourceToString(Logging::LoggingSourceEvents));
    QCOMPARE(logEntry.value("loggingLevel").toString(), JsonTypes::loggingLevelToString(Logging::LoggingLevelInfo));

    // get this logentry with filter
    QVariantMap params;
    params.insert("deviceIds", QVariantList() << device->id());
    params.insert("loggingSources", QVariantList() << JsonTypes::loggingSourceToString(Logging::LoggingSourceEvents));

    QVariant response = injectAndWait("Logging.GetLogEntries", params);
    verifyLoggingError(response);

    QVariantList logEntries = response.toMap().value("params").toMap().value("logEntries").toList();
    QVERIFY(logEntries.count() == 1);

}

void TestLogging::actionLog_data()
{

}

void TestLogging::actionLog()
{

}

#include "testlogging.moc"
QTEST_MAIN(TestLogging)

