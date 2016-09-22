/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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
#include "mocktcpserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>

using namespace guhserver;

class TestConfigurations: public GuhTestBase
{
    Q_OBJECT

private slots:
    void getConfigurations();
    void testTimeZones();
    void testServerName();


};

void TestConfigurations::getConfigurations()
{
    QVariant response = injectAndWait("Configuration.GetConfigurations");
    QVariantMap configurations = response.toMap().value("params").toMap();
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(configurations).toJson());

    QVERIFY(configurations.contains("basicConfiguration"));
    QVERIFY(!configurations.value("basicConfiguration").toMap().value("serverUuid").toUuid().isNull());

    QVERIFY(configurations.contains("sslConfiguration"));
    QVERIFY(configurations.contains("tcpServerConfiguration"));
    QVERIFY(configurations.contains("webServerConfiguration"));
    QVERIFY(configurations.contains("webSocketServerConfiguration"));
}

void TestConfigurations::testTimeZones()
{
    QVariantMap params; QVariant response; QVariantMap configurations;

    QVariantList timeZones = injectAndWait("Configuration.GetTimeZones").toMap().value("params").toMap().value("timeZones").toList();
    QVERIFY(timeZones.count() > 0);
    QVERIFY(timeZones.contains("America/Toronto"));
    QVERIFY(timeZones.contains("Europe/Vienna"));

    // Set current timezone (Europe/Vienna)
    params.clear(); response.clear(); configurations.clear();
    params.insert("timeZone", "Europe/Vienna");
    response = injectAndWait("Configuration.SetTimeZone", params);
    verifyConfigurationError(response);

    // Get current timezone and time
    params.clear(); response.clear(); configurations.clear();
    configurations = injectAndWait("Configuration.GetConfigurations").toMap().value("params").toMap();
    QString currentTimeZone = configurations.value("basicConfiguration").toMap().value("timeZone").toString();
    int currentTime = configurations.value("basicConfiguration").toMap().value("serverTime").toInt();
    qDebug() << currentTimeZone << QDateTime::fromTime_t(currentTime);

    // Set new timezone
    params.clear(); response.clear(); configurations.clear();
    params.insert("timeZone", "Moon/Darkside");
    response = injectAndWait("Configuration.SetTimeZone", params);
    verifyConfigurationError(response, GuhConfiguration::ConfigurationErrorInvalidTimeZone);

    // Set new timezone
    params.clear(); response.clear(); configurations.clear();
    params.insert("timeZone", "America/Toronto");
    response = injectAndWait("Configuration.SetTimeZone", params);
    verifyConfigurationError(response);

    // Check new timezone
    params.clear(); response.clear(); configurations.clear();
    configurations = injectAndWait("Configuration.GetConfigurations").toMap().value("params").toMap();
    QString newTimeZone = configurations.value("basicConfiguration").toMap().value("timeZone").toString();
    int newTime = configurations.value("basicConfiguration").toMap().value("serverTime").toInt();
    qDebug() << newTimeZone << QDateTime::fromTime_t(newTime);
    QVERIFY(currentTimeZone != newTimeZone);

    restartServer();

    // Check loaded timezone
    configurations = injectAndWait("Configuration.GetConfigurations").toMap().value("params").toMap();
    QString reloadedTimeZone = configurations.value("basicConfiguration").toMap().value("timeZone").toString();
    QVERIFY(newTimeZone == reloadedTimeZone);

    params.clear(); response.clear();
    params.insert("timeZone", "Europe/Vienna");
    response = injectAndWait("Configuration.SetTimeZone", params);
    verifyConfigurationError(response);
}

void TestConfigurations::testServerName()
{
    QVariantMap params; QVariant response; QVariantMap configurations;
    configurations = injectAndWait("Configuration.GetConfigurations").toMap().value("params").toMap();
    QString serverName = configurations.value("basicConfiguration").toMap().value("serverName").toString();
    QString serverUuid = configurations.value("basicConfiguration").toMap().value("serverUuid").toString();
    qDebug() << "Server name" << serverName << "(" << serverUuid << ")";

    params.insert("serverName", "Test server");
    response = injectAndWait("Configuration.SetServerName", params);
    verifyConfigurationError(response);

    configurations = injectAndWait("Configuration.GetConfigurations").toMap().value("params").toMap();
    QString newServerName = configurations.value("basicConfiguration").toMap().value("serverName").toString();
    QVERIFY(newServerName == "Test server");

    restartServer();

    configurations = injectAndWait("Configuration.GetConfigurations").toMap().value("params").toMap();
    QString reloadedServerName = configurations.value("basicConfiguration").toMap().value("serverName").toString();
    QVERIFY(newServerName == reloadedServerName);
}

#include "testconfigurations.moc"
QTEST_MAIN(TestConfigurations)
