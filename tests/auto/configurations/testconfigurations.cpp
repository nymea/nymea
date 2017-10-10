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
    void testLanguages();

private:
    QVariantMap loadBasicConfiguration();

};

void TestConfigurations::getConfigurations()
{
    QVariant response = injectAndWait("Configuration.GetConfigurations");
    QVariantMap configurations = response.toMap().value("params").toMap();
    qDebug() << QJsonDocument::fromVariant(configurations).toJson();

    QVERIFY(configurations.contains("basicConfiguration"));
    QVERIFY(!configurations.value("basicConfiguration").toMap().value("serverUuid").toUuid().isNull());

    QVERIFY(configurations.contains("tcpServerConfigurations"));
    QVERIFY(configurations.contains("webServerConfigurations"));
    QVERIFY(configurations.contains("webSocketServerConfigurations"));
}

void TestConfigurations::testTimeZones()
{
    enableNotifications();

    QVariantMap params; QVariant response; QVariantMap configurations; QVariantList configurationChangedNotifications;

    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    QVariantList timeZones = injectAndWait("Configuration.GetTimeZones").toMap().value("params").toMap().value("timeZones").toList();
    QVERIFY(timeZones.count() > 0);
    QVERIFY(timeZones.contains("America/Toronto"));
    QVERIFY(timeZones.contains("Europe/Vienna"));
    QVERIFY(timeZones.contains("Africa/Dakar"));

    // Get current configurations
    QVariantMap basicConfigurationMap = loadBasicConfiguration();

    // Set timezone unchainged
    params.clear(); response.clear(); configurations.clear();
    params.insert("timeZone", basicConfigurationMap.value("timeZone").toString());
    response = injectAndWait("Configuration.SetTimeZone", params);
    verifyConfigurationError(response);

    // Check notification not emitted
    notificationSpy.wait(200);
    configurationChangedNotifications = checkNotifications(notificationSpy, "Configuration.BasicConfigurationChanged");
    QVERIFY2(configurationChangedNotifications.count() == 0, "Got Configuration.BasicConfigurationChanged notification but should have not.");

    // Set new timezone (Africa/Dakar)
    QString newTimeZone("Africa/Dakar");
    params.clear(); response.clear(); configurations.clear(); notificationSpy.clear();
    params.insert("timeZone", newTimeZone);
    response = injectAndWait("Configuration.SetTimeZone", params);
    verifyConfigurationError(response);

    notificationSpy.wait(200);
    configurationChangedNotifications = checkNotifications(notificationSpy, "Configuration.BasicConfigurationChanged");
    QVERIFY2(configurationChangedNotifications.count() == 1, "Should get only one Configuration.BasicConfigurationChanged notification");
    QVariantMap notificationContent = configurationChangedNotifications.first().toMap().value("params").toMap();
    QVERIFY2(notificationContent.contains("basicConfiguration"), "Notification does not contain basicConfiguration");
    QVariantMap basicConfigurationNotificationMap = notificationContent.value("basicConfiguration").toMap();
    QVERIFY2(basicConfigurationNotificationMap.contains("language"), "Notification does not contain key language");
    QVERIFY2(basicConfigurationNotificationMap.contains("serverName"), "Notification does not contain key serverName");
    QVERIFY2(basicConfigurationNotificationMap.contains("serverTime"), "Notification does not contain key serverTime");
    QVERIFY2(basicConfigurationNotificationMap.contains("serverUuid"), "Notification does not contain key serverUuid");
    QVERIFY2(basicConfigurationNotificationMap.contains("timeZone"), "Notification does not contain key timeZone");
    QVERIFY2(basicConfigurationNotificationMap.value("timeZone").toString() == newTimeZone, "Notification does not contain the new timeZone");

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
    newTimeZone = configurations.value("basicConfiguration").toMap().value("timeZone").toString();
    int newTime = configurations.value("basicConfiguration").toMap().value("serverTime").toInt();
    qDebug() << newTimeZone << QDateTime::fromTime_t(newTime);
    QVERIFY(currentTimeZone != newTimeZone);

    restartServer();

    // Check loaded timezone
    configurations = injectAndWait("Configuration.GetConfigurations").toMap().value("params").toMap();
    QString reloadedTimeZone = configurations.value("basicConfiguration").toMap().value("timeZone").toString();
    QVERIFY(newTimeZone == reloadedTimeZone);

    // Reset the timezone
    params.clear(); response.clear();
    params.insert("timeZone", "Europe/Vienna");
    response = injectAndWait("Configuration.SetTimeZone", params);
    verifyConfigurationError(response);

    disableNotifications();
}

void TestConfigurations::testServerName()
{
    enableNotifications();

    // Get current configurations
    QVariantMap basicConfigurationMap = loadBasicConfiguration();

    QString serverName = basicConfigurationMap.value("serverName").toString();
    QString serverUuid = basicConfigurationMap.value("serverUuid").toString();
    qDebug() << "Server name" << serverName << "(" << serverUuid << ")";

    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Set name unchanged
    QVariantMap params; QVariant response; QVariantList configurationChangedNotifications;
    params.insert("serverName", serverName);
    response = injectAndWait("Configuration.SetServerName", params);
    verifyConfigurationError(response);

    // Check notification not emitted
    notificationSpy.wait(500);
    configurationChangedNotifications = checkNotifications(notificationSpy, "Configuration.BasicConfigurationChanged");
    QVERIFY2(configurationChangedNotifications.count() == 0, "Got Configuration.BasicConfigurationChanged notification but should have not.");

    // Set new server name
    QString newServerName = QString("Test server %1").arg(QUuid::createUuid().toString());
    params.clear(); response.clear(); configurationChangedNotifications.clear();
    params.insert("serverName", newServerName);

    notificationSpy.clear();
    response = injectAndWait("Configuration.SetServerName", params);
    verifyConfigurationError(response);

    // Check notification not emitted
    notificationSpy.wait(500);
    configurationChangedNotifications = checkNotifications(notificationSpy, "Configuration.BasicConfigurationChanged");
    QVariantMap notificationContent = configurationChangedNotifications.first().toMap().value("params").toMap();
    QVERIFY2(notificationContent.contains("basicConfiguration"), "Notification does not contain basicConfiguration");
    QVERIFY2(configurationChangedNotifications.count() == 1, "Should get only one Configuration.BasicConfigurationChanged notification");
    QVariantMap basicConfigurationNotificationMap = notificationContent.value("basicConfiguration").toMap();
    QVERIFY2(basicConfigurationNotificationMap.contains("language"), "Notification does not contain key language");
    QVERIFY2(basicConfigurationNotificationMap.contains("serverTime"), "Notification does not contain key serverTime");
    QVERIFY2(basicConfigurationNotificationMap.contains("serverUuid"), "Notification does not contain key serverUuid");
    QVERIFY2(basicConfigurationNotificationMap.contains("timeZone"), "Notification does not contain key timeZone");
    QVERIFY2(basicConfigurationNotificationMap.contains("serverName"), "Notification does not contain key serverName");
    QVERIFY2(basicConfigurationNotificationMap.value("serverName").toString() == newServerName, "Notification does not contain the new serverName");

    basicConfigurationMap = loadBasicConfiguration();
    QString loadedServerName = basicConfigurationMap.value("serverName").toString();
    QVERIFY2(loadedServerName == newServerName, "Server name not set correctly");

    restartServer();

    basicConfigurationMap = loadBasicConfiguration();
    loadedServerName = basicConfigurationMap.value("serverName").toString();
    QVERIFY2(newServerName == loadedServerName, "Server name not loaded correctly after restart");

    disableNotifications();
}

void TestConfigurations::testLanguages()
{
    enableNotifications();

    // Get current configurations
    QVariantMap basicConfigurationMap = loadBasicConfiguration();

    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Set language unchanged
    QVariant response; QVariantMap params;
    params.insert("language", basicConfigurationMap.value("language"));
    response = injectAndWait("Configuration.SetLanguage", params);
    verifyConfigurationError(response);

    // Check notification not emitted
    notificationSpy.wait(500);
    QVariantList languageChangedNotifications = checkNotifications(notificationSpy, "Configuration.LanguageChanged");
    QVERIFY2(languageChangedNotifications.count() == 0, "Got Configuration.LanguageChanged notification but should have not.");

    // Get available languages
    response = injectAndWait("Configuration.GetAvailableLanguages");
    QVERIFY2(response.toMap().value("params").toMap().contains("languages"), "Did not get list of languages");
    QVariantMap responseMap = response.toMap().value("params").toMap();
    QVERIFY2(responseMap.value("languages").toList().count() >= 2, "Avaliable languages list to short: " +  responseMap.value("languages").toList().count());

    QVariantList languageVariantList = responseMap.value("languages").toList();
    foreach (const QVariant &languageVariant, languageVariantList) {
        // Get current configurations
        basicConfigurationMap = loadBasicConfiguration();

        // Set language
        params.clear(); response.clear(); notificationSpy.clear();
        params.insert("language", languageVariant);
        QVariant response = injectAndWait("Configuration.SetLanguage", params);
        verifyConfigurationError(response);

        // Check notification
        notificationSpy.wait(500);
        QVariantList languageChangedNotifications = checkNotifications(notificationSpy, "Configuration.LanguageChanged");

        // If the language did not change no notification should be emited
        if (basicConfigurationMap.value("language").toString() == languageVariant.toString()) {
            QVERIFY2(languageChangedNotifications.count() == 0, "Got Configuration.LanguageChanged notification but should have not.");
        } else {
            QVERIFY2(languageChangedNotifications.count() == 1, "Should get only one Configuration.LanguageChanged notification");
            QVariantMap notificationMap = languageChangedNotifications.first().toMap().value("params").toMap();
            QVERIFY2(notificationMap.contains("language"), "Notification does not contain language");
            QVERIFY2(notificationMap.value("language").toString() == languageVariant.toString(), "Notification does not contain the new language");

            // Restart the server and check if the language will be loaded correctly
            restartServer();

            // Get configuration
            basicConfigurationMap = loadBasicConfiguration();

            QCOMPARE(basicConfigurationMap.value("language").toString(), languageVariant.toString());
        }
    }

    // Reset the language to en_US
    params.clear(); response.clear();
    params.insert("language", "en_US");

    // Set language
    response = injectAndWait("Configuration.SetLanguage", params);
    verifyConfigurationError(response);

    disableNotifications();
}

QVariantMap TestConfigurations::loadBasicConfiguration()
{
    QVariant response = injectAndWait("Configuration.GetConfigurations");
    QVariantMap configurationMap = response.toMap().value("params").toMap();
    return configurationMap.value("basicConfiguration").toMap();
}

#include "testconfigurations.moc"
QTEST_MAIN(TestConfigurations)
