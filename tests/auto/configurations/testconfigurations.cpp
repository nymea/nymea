// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeatestbase.h"
#include "nymeacore.h"
#include "nymeasettings.h"
#include "servers/mocktcpserver.h"
#include "../plugins/mock/extern-plugininfo.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QScopedPointer>
#include <QWebSocket>
#include <limits>

using namespace nymeaserver;

class TestConfigurations: public NymeaTestBase
{
    Q_OBJECT

private:
    inline void verifyConfigurationError(const QVariant &response, NymeaConfiguration::ConfigurationError error = NymeaConfiguration::ConfigurationErrorNoError) {
        verifyError(response, "configurationError", enumValueName(error));
    }

protected slots:
    void initTestCase();

private slots:
    void getConfigurations();
    void testBackupFiles();
    void testCreateAndDownloadBackup();

    void testServerName();
    void testLanguages();
    void testLocationNotifications();

    void testDebugServerConfiguration();

    void testDisableInsecureInterfacesEnv();

private:
    QVariantMap loadBasicConfiguration();
    QVariantList loadBackupFiles();
    QWebSocket *openSocket();
    QVariant sendAndWait(QWebSocket *socket, int id, const QString &method, const QVariantMap &params = QVariantMap(), QVariantMap *notification = nullptr);

public slots:
    void sslErrors(const QList<QSslError> &)
    {
        QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
        if (socket) {
            socket->ignoreSslErrors();
        }
    }

};

void TestConfigurations::initTestCase()
{
    QDir dir(NymeaSettings::translationsPath());
    dir.mkpath(NymeaSettings::translationsPath());
    QStringList languages = {"de", "en_US"};
    foreach (const QString &language, languages) {
        QFile f(NymeaSettings::translationsPath().append("/nymead-" + language + ".qm"));
        QVERIFY2(f.open(QFile::WriteOnly), "Could not create translation file.");
        f.write(" ");
        f.close();
    }

    NymeaTestBase::initTestCase("*.debug=false\nApplication.debug=true\nTests.debug=true\nServerManager.debug=true");
}

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

void TestConfigurations::testBackupFiles()
{
    enableNotifications({"Configuration"});

    const QString backupDirectoryPath = "/tmp/nymea-tests/backups";
    QDir backupDirectory(backupDirectoryPath);
    if (backupDirectory.exists()) {
        QVERIFY2(backupDirectory.removeRecursively(), "Could not clear backup directory.");
    }
    QVERIFY2(QDir().mkpath(backupDirectoryPath), "Could not create backup directory.");

    QVariantMap params;
    params.insert("destinationDirectory", backupDirectoryPath);
    params.insert("maxCount", 10);
    QVariant response = injectAndWait("Configuration.SetBackupConfiguration", params);
    verifyConfigurationError(response);

    QSignalSpy notificationSpy(m_mockTcpServer, &MockTcpServer::outgoingData);
    notificationSpy.clear();

    response = injectAndWait("Configuration.CreateBackup");
    verifyConfigurationError(response);

    QVERIFY2(notificationSpy.wait(2000), "Timed out waiting for first backup notification.");
    QVariantList notifications = checkNotifications(notificationSpy, "Configuration.BackupFilesChanged");
    QVERIFY2(notifications.count() == 1, "Expected exactly one Configuration.BackupFilesChanged notification for first backup.");

    QVariantList backupFilesNotification = notifications.first().toMap().value("params").toMap().value("backupFiles").toList();
    QCOMPARE(backupFilesNotification.count(), 1);

    QTest::qWait(1100);

    notificationSpy.clear();
    response = injectAndWait("Configuration.CreateBackup");
    verifyConfigurationError(response);

    QVERIFY2(notificationSpy.wait(2000), "Timed out waiting for second backup notification.");
    notifications = checkNotifications(notificationSpy, "Configuration.BackupFilesChanged");
    QVERIFY2(notifications.count() == 1, "Expected exactly one Configuration.BackupFilesChanged notification for second backup.");

    backupFilesNotification = notifications.first().toMap().value("params").toMap().value("backupFiles").toList();
    QCOMPARE(backupFilesNotification.count(), 2);

    QVariantList backupFiles = loadBackupFiles();
    QCOMPARE(backupFiles.count(), 2);
    QCOMPARE(backupFilesNotification, backupFiles);

    qint64 previousTimestamp = std::numeric_limits<qint64>::max();
    for (const QVariant &backupFileVariant: backupFiles) {
        const QVariantMap backupFile = backupFileVariant.toMap();
        QVERIFY2(backupFile.contains("fileName"), "Backup file entry does not contain fileName.");
        QVERIFY2(backupFile.contains("serverVersion"), "Backup file entry does not contain serverVersion.");
        QVERIFY2(backupFile.contains("timestamp"), "Backup file entry does not contain timestamp.");
        QVERIFY2(backupFile.contains("size"), "Backup file entry does not contain size.");

        const QString fileName = backupFile.value("fileName").toString();
        const QString serverVersion = backupFile.value("serverVersion").toString();
        const qint64 timestamp = backupFile.value("timestamp").toLongLong();
        const double size = backupFile.value("size").toDouble();

        QVERIFY2(!serverVersion.isEmpty(), "Backup serverVersion should not be empty.");
        QVERIFY2(timestamp > 0, "Backup timestamp should be greater than 0.");
        QVERIFY2(size > 0, "Backup size should be greater than 0.");
        QVERIFY2(timestamp <= previousTimestamp, "Backup files should be returned sorted by timestamp descending.");
        previousTimestamp = timestamp;

        const QRegularExpression fileNamePattern(QString("^nymea-configuration-%1-(\\d{14})\\.tar\\.gz$")
                                                 .arg(QRegularExpression::escape(serverVersion)));
        const QRegularExpressionMatch fileNameMatch = fileNamePattern.match(fileName);
        QVERIFY2(fileNameMatch.hasMatch(), qPrintable(QString("Unexpected backup file name: %1").arg(fileName)));

        const QString timestampString = fileNameMatch.captured(1);
        const QDateTime fileNameTimestamp(QDate::fromString(timestampString.left(8), "yyyyMMdd"),
                                          QTime::fromString(timestampString.mid(8, 6), "hhmmss"),
                                          Qt::UTC);
        QVERIFY2(fileNameTimestamp.isValid(), qPrintable(QString("Could not parse backup timestamp from file name: %1").arg(fileName)));
        QCOMPARE(fileNameTimestamp.toSecsSinceEpoch(), timestamp);

        QFileInfo fileInfo(backupDirectory.filePath(fileName));
        QVERIFY2(fileInfo.exists(), qPrintable(QString("Backup file does not exist on disk: %1").arg(fileInfo.absoluteFilePath())));
        QCOMPARE(fileInfo.fileName(), fileName);
        QCOMPARE(static_cast<double>(fileInfo.size()), size);
    }

    disableNotifications();
}

void TestConfigurations::testCreateAndDownloadBackup()
{
    const QString backupDirectoryPath = "/tmp/nymea-tests/backups-download";
    QDir backupDirectory(backupDirectoryPath);
    if (backupDirectory.exists()) {
        QVERIFY2(backupDirectory.removeRecursively(), "Could not clear download backup directory.");
    }
    QVERIFY2(QDir().mkpath(backupDirectoryPath), "Could not create download backup directory.");

    QVariantMap params;
    params.insert("destinationDirectory", backupDirectoryPath);
    params.insert("maxCount", 10);
    QVariant response = injectAndWait("Configuration.SetBackupConfiguration", params);
    verifyConfigurationError(response);

    response = injectAndWait("Configuration.CreateAndDownloadBackup");
    verifyConfigurationError(response);

    const QVariantMap createDownloadResponse = response.toMap().value("params").toMap();
    const QString downloadId = createDownloadResponse.value("downloadId").toString();
    const QString fileName = createDownloadResponse.value("fileName").toString();
    const qint64 size = createDownloadResponse.value("size").toLongLong();

    QVERIFY2(!downloadId.isEmpty(), "CreateAndDownloadBackup did not return a downloadId.");
    QVERIFY2(!fileName.isEmpty(), "CreateAndDownloadBackup did not return a fileName.");
    QVERIFY2(size > 0, "CreateAndDownloadBackup did not return a valid size.");

    const QStringList backupFiles = backupDirectory.entryList(QStringList() << "nymea-configuration-*.tar.gz", QDir::Files, QDir::Time);
    QCOMPARE(backupFiles.count(), 1);
    QCOMPARE(fileName, backupFiles.first());

    QFile backupFile(backupDirectory.filePath(fileName));
    QVERIFY2(backupFile.open(QIODevice::ReadOnly), "Could not open created backup file.");
    const QByteArray expectedPayload = backupFile.readAll();
    QCOMPARE(expectedPayload.size(), size);

    {
        QScopedPointer<QWebSocket> apiSocket(openSocket());
        QVERIFY(!apiSocket.isNull());

        response = sendAndWait(apiSocket.data(), 1, "JSONRPC.Hello");
        QCOMPARE(response.toMap().value("status").toString(), QString("success"));

        params.clear();
        params.insert("downloadId", downloadId);
        response = sendAndWait(apiSocket.data(), 2, "Transfers.StartDownload", params);
        QCOMPARE(response.toMap().value("status").toString(), QString("success"));

        const QVariantMap downloadInfo = response.toMap().value("params").toMap();
        const QString downloadTransferId = downloadInfo.value("transferId").toString();
        const QString downloadTransferToken = downloadInfo.value("transferToken").toString();
        QCOMPARE(downloadInfo.value("fileName").toString(), fileName);
        QCOMPARE(downloadInfo.value("size").toLongLong(), size);
        QVERIFY(!downloadTransferId.isEmpty());
        QVERIFY(!downloadTransferToken.isEmpty());

        QScopedPointer<QWebSocket> downloadSocket(openSocket());
        QVERIFY(!downloadSocket.isNull());

        params.clear();
        params.insert("transferId", downloadTransferId);
        params.insert("transferToken", downloadTransferToken);
        response = sendAndWait(downloadSocket.data(), 10, "Transfer.Connect", params);
        QCOMPARE(response.toMap().value("status").toString(), QString("success"));
        QCOMPARE(response.toMap().value("params").toMap().value("direction").toString(), QString("download"));

        QByteArray downloadedPayload;
        bool finished = false;
        int downloadCommandId = 11;
        while (!finished) {
            params.clear();
            params.insert("maxBytes", 4096);
            response = sendAndWait(downloadSocket.data(), downloadCommandId++, "Transfer.RequestChunk", params);
            QCOMPARE(response.toMap().value("status").toString(), QString("success"));

            const QVariantMap chunkParams = response.toMap().value("params").toMap();
            downloadedPayload.append(QByteArray::fromBase64(chunkParams.value("data").toByteArray()));
            finished = chunkParams.value("finished").toBool();
        }

        QCOMPARE(downloadedPayload, expectedPayload);

        QSignalSpy downloadDisconnectedSpy(downloadSocket.data(), &QWebSocket::disconnected);
        downloadSocket->close();
        downloadDisconnectedSpy.wait(1000);

        params.clear();
        QSignalSpy apiDisconnectedSpy(apiSocket.data(), &QWebSocket::disconnected);
        apiSocket->close();
        apiDisconnectedSpy.wait(1000);
    }

    qApp->processEvents();
}

void TestConfigurations::testServerName()
{
    enableNotifications({"Configuration"});

    // Get current configurations
    QVariantMap basicConfigurationMap = loadBasicConfiguration();

    QString serverName = basicConfigurationMap.value("serverName").toString();
    QString serverUuid = basicConfigurationMap.value("serverUuid").toString();
    qDebug() << "Server name" << serverName << "(" << serverUuid << ")";

    QSignalSpy notificationSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

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
    notificationSpy.wait();
    configurationChangedNotifications = checkNotifications(notificationSpy, "Configuration.BasicConfigurationChanged");
    QVariantMap notificationContent = configurationChangedNotifications.first().toMap().value("params").toMap();
    QVERIFY2(notificationContent.contains("basicConfiguration"), "Notification does not contain basicConfiguration");
    QVERIFY2(configurationChangedNotifications.count() == 1, "Should get only one Configuration.BasicConfigurationChanged notification");
    QVariantMap basicConfigurationNotificationMap = notificationContent.value("basicConfiguration").toMap();
    QVERIFY2(basicConfigurationNotificationMap.contains("language"), "Notification does not contain key language");
    QVERIFY2(basicConfigurationNotificationMap.contains("serverTime"), "Notification does not contain key serverTime");
    QVERIFY2(basicConfigurationNotificationMap.contains("serverUuid"), "Notification does not contain key serverUuid");
    QVERIFY2(basicConfigurationNotificationMap.contains("timeZone"), "Notification does not contain key timeZone");
    QVERIFY2(basicConfigurationNotificationMap.contains("debugServerEnabled"), "Notification does not contain key debugServerEnabled");
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
    enableNotifications({"Configuration"});

    // Get current configurations
    QVariantMap basicConfigurationMap = loadBasicConfiguration();

    QSignalSpy notificationSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

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
    QVERIFY2(responseMap.value("languages").toList().size() >= 2, qPrintable(QString("Available languages list to short: %1").arg(responseMap.value("languages").toList().size())));

    QVariantList languageVariantList = responseMap.value("languages").toList();
    foreach (const QVariant &languageVariant, languageVariantList) {
        // create a new spy for each run as we restart the server and kill the old one in this loop
         QSignalSpy notificationSpy2 (m_mockTcpServer, &MockTcpServer::outgoingData);

        // Get current configurations
        basicConfigurationMap = loadBasicConfiguration();

        // Set language
        params.clear(); response.clear();
        params.insert("language", languageVariant);
        QVariant response = injectAndWait("Configuration.SetLanguage", params);
        verifyConfigurationError(response);


        // Check notification
        notificationSpy2.wait(500);
        QVariantList configurationChangedNotifications = checkNotifications(notificationSpy2, "Configuration.BasicConfigurationChanged");

        // If the language did not change no notification should be emitted
        if (basicConfigurationMap.value("language").toString() == languageVariant.toString()) {
            QVERIFY2(configurationChangedNotifications.count() == 0, "Got Configuration.BasicConfigurationChanged notification but should have not.");
        } else {
            QVariantMap notificationContent = configurationChangedNotifications.first().toMap().value("params").toMap();
            QVERIFY2(notificationContent.contains("basicConfiguration"), "Notification does not contain basicConfiguration");
            QVERIFY2(configurationChangedNotifications.count() == 1, "Should get only one Configuration.BasicConfigurationChanged notification");
            QVariantMap basicConfigurationNotificationMap = notificationContent.value("basicConfiguration").toMap();
            QVERIFY2(basicConfigurationNotificationMap.contains("language"), "Notification does not contain key language");
            QVERIFY2(basicConfigurationNotificationMap.value("language").toString() == languageVariant.toString(), "Notification does not contain the new language");

            // Restart the server and check if the language will be loaded correctly
            restartServer();
            enableNotifications({"Configuration"});


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

void TestConfigurations::testLocationNotifications()
{
    enableNotifications({"Configuration"});

    const QVariantMap currentBasicConfiguration = loadBasicConfiguration();
    const QVariantMap currentLocation = currentBasicConfiguration.value("location").toMap();

    const double newLatitude = currentLocation.value("latitude").toDouble() + 1.234567;
    const double newLongitude = currentLocation.value("longitude").toDouble() - 2.345678;
    const QString newName = QString("Test location %1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));

    QSignalSpy notificationSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    QVariantMap params;
    params.insert("location", QVariantMap{
                      {"latitude", newLatitude},
                      {"longitude", newLongitude},
                      {"name", newName},
                  });
    const QVariant response = injectAndWait("Configuration.SetLocation", params);
    verifyConfigurationError(response);

    QVERIFY2(notificationSpy.wait(500), "Timed out waiting for Configuration.BasicConfigurationChanged notification.");
    const QVariantList configurationChangedNotifications = checkNotifications(notificationSpy, "Configuration.BasicConfigurationChanged");
    QCOMPARE(configurationChangedNotifications.count(), 1);

    const QVariantMap notificationContent = configurationChangedNotifications.first().toMap().value("params").toMap();
    const QVariantMap basicConfiguration = notificationContent.value("basicConfiguration").toMap();
    const QVariantMap notifiedLocation = basicConfiguration.value("location").toMap();
    QCOMPARE(notifiedLocation.value("latitude").toDouble(), newLatitude);
    QCOMPARE(notifiedLocation.value("longitude").toDouble(), newLongitude);
    QCOMPARE(notifiedLocation.value("name").toString(), newName);

    const QVariantMap storedLocation = loadBasicConfiguration().value("location").toMap();
    QCOMPARE(storedLocation.value("latitude").toDouble(), newLatitude);
    QCOMPARE(storedLocation.value("longitude").toDouble(), newLongitude);
    QCOMPARE(storedLocation.value("name").toString(), newName);

    disableNotifications();
}

void TestConfigurations::testDebugServerConfiguration()
{
    enableNotifications({"Configuration"});

    // Get current configurations
    QVariantMap basicConfigurationMap = loadBasicConfiguration();

    bool debugServerEnabled = basicConfigurationMap.value("debugServerEnabled").toBool();
    qCDebug(dcTests) << "Debug server enabled" << debugServerEnabled;

    QSignalSpy notificationSpy(m_mockTcpServer, &MockTcpServer::outgoingData);

    // Unchanged debug server
    QVariantMap params; QVariant response; QVariantList configurationChangedNotifications;
    params.insert("enabled", debugServerEnabled);
    response = injectAndWait("Configuration.SetDebugServerEnabled", params);
    verifyConfigurationError(response);

    // Check notification not emitted
    notificationSpy.wait(500);
    configurationChangedNotifications = checkNotifications(notificationSpy, "Configuration.BasicConfigurationChanged");
    QVERIFY2(configurationChangedNotifications.count() == 0, "Got Configuration.BasicConfigurationChanged notification but should have not.");

    // Enable debug server
    bool newValue = true;
    params.clear(); response.clear(); configurationChangedNotifications.clear();
    params.insert("enabled", newValue);

    qCDebug(dcTests()) << "Enabling debug server";

    notificationSpy.clear();
    response = injectAndWait("Configuration.SetDebugServerEnabled", params);
    verifyConfigurationError(response);

    // Check notification not emitted
    notificationSpy.wait();
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
    QVERIFY2(basicConfigurationNotificationMap.contains("debugServerEnabled"), "Notification does not contain key debugServerEnabled");
    QVERIFY2(basicConfigurationNotificationMap.value("debugServerEnabled").toBool() == newValue, "Notification does not contain the new debugServerEnabled");

    qCDebug(dcTests()) << "TestWebserver starting";
    foreach (const WebServerConfiguration &config, NymeaCore::instance()->configuration()->webServerConfigurations()) {
        if (config.port == 3333 && (QHostAddress(config.address) == QHostAddress("127.0.0.1") || QHostAddress(config.address) == QHostAddress("0.0.0.0"))) {
            qDebug() << "Already have a webserver listening on 127.0.0.1:3333";
            return;
        }
    }

    qCDebug(dcTests) << "Creating new webserver instance on 127.0.0.1:3333";
    WebServerConfiguration config;
    config.id = "Testwebserver for debug server interface";
    config.address = "127.0.0.1";
    config.port = 3333;
    config.sslEnabled = true;
    NymeaCore::instance()->configuration()->setWebServerConfiguration(config);

    // Webserver request
    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, this, [](QNetworkReply* reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy namSpy(&nam, &QNetworkAccessManager::finished);

    // Check if debug interface is reachable
    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333/debug/"));
    QNetworkReply *reply = nam.get(request);

    namSpy.wait();
    QVERIFY2(namSpy.count() > 0, "expected response from webserver");

    bool ok = false;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, 200);
    reply->deleteLater();

    // Disable debug server
    params.clear(); response.clear();
    params.insert("enabled", false);
    response = injectAndWait("Configuration.SetDebugServerEnabled", params);
    verifyConfigurationError(response);

    // Check if debug interface is not reachable any more
    namSpy.clear();
    reply = nam.get(request);

    namSpy.wait();
    QVERIFY2(namSpy.count() > 0, "expected response from webserver");

    ok = false;
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert status code from response to int");
    QCOMPARE(statusCode, 404);
    reply->deleteLater();

    NymeaCore::instance()->configuration()->removeWebServerConfiguration(config.id);

    disableNotifications();
}

void TestConfigurations::testDisableInsecureInterfacesEnv()
{
    QString id = "insecure";

    // Create a insecure interface
    QVariantMap insecureTcpConfig;
    insecureTcpConfig.insert("id", id);
    insecureTcpConfig.insert("address", "0.0.0.0");
    insecureTcpConfig.insert("port", 23456);
    insecureTcpConfig.insert("sslEnabled", false);
    insecureTcpConfig.insert("authenticationEnabled", false);

    // Create a insecure interface
    QVariantMap insecureWebSocketConfig;
    insecureWebSocketConfig.insert("id", id);
    insecureWebSocketConfig.insert("address", "0.0.0.0");
    insecureWebSocketConfig.insert("port", 23457);
    insecureWebSocketConfig.insert("sslEnabled", false);
    insecureWebSocketConfig.insert("authenticationEnabled", false);

    // Create a insecure interface
    QVariantMap insecureTunnelProxyConfig;
    insecureTunnelProxyConfig.insert("id", id);
    insecureTunnelProxyConfig.insert("address", "example.nymea.io");
    insecureTunnelProxyConfig.insert("port", 2213);
    insecureTunnelProxyConfig.insert("sslEnabled", false);
    insecureTunnelProxyConfig.insert("authenticationEnabled", false);
    insecureTunnelProxyConfig.insert("ignoreSslErrors", true);

    QVariantMap params; QVariant response;

    params.insert("configuration", insecureTcpConfig);
    response = injectAndWait("Configuration.SetTcpServerConfiguration", params);
    verifyConfigurationError(response);

    params.insert("configuration", insecureWebSocketConfig);
    response = injectAndWait("Configuration.SetWebSocketServerConfiguration", params);
    verifyConfigurationError(response);

    params.insert("configuration", insecureTunnelProxyConfig);
    response = injectAndWait("Configuration.SetTunnelProxyServerConfiguration", params);
    verifyConfigurationError(response);

    // Restart with disabled insecure interfaces
    qputenv("NYMEA_INSECURE_INTERFACES_DISABLED", "1");
    restartServer();

    // FIXME: make sure the insecure servers are not running

    // Remove the insecure configs and try to add them again and expect them to fail
    params.clear(); response.clear();
    params.insert("id", id);
    response = injectAndWait("Configuration.DeleteTcpServerConfiguration", params);
    verifyConfigurationError(response);

    params.clear(); response.clear();
    params.insert("id", id);
    response = injectAndWait("Configuration.DeleteWebSocketServerConfiguration", params);
    verifyConfigurationError(response);

    params.clear(); response.clear();
    params.insert("id", id);
    response = injectAndWait("Configuration.DeleteTunnelProxyServerConfiguration", params);
    verifyConfigurationError(response);

    // Make sure we cannot add insecure interfaces beside localhost
    params.clear(); response.clear();
    params.insert("configuration", insecureTcpConfig);
    response = injectAndWait("Configuration.SetTcpServerConfiguration", params);
    verifyConfigurationError(response, NymeaConfiguration::ConfigurationErrorUnsupported);

    params.clear(); response.clear();
    params.insert("configuration", insecureWebSocketConfig);
    response = injectAndWait("Configuration.SetWebSocketServerConfiguration", params);
    verifyConfigurationError(response, NymeaConfiguration::ConfigurationErrorUnsupported);

    params.clear(); response.clear();
    params.insert("configuration", insecureTunnelProxyConfig);
    response = injectAndWait("Configuration.SetTunnelProxyServerConfiguration", params);
    verifyConfigurationError(response, NymeaConfiguration::ConfigurationErrorUnsupported);

    qunsetenv("NYMEA_INSECURE_INTERFACES_DISABLED");
}

QVariantMap TestConfigurations::loadBasicConfiguration()
{
    QVariant response = injectAndWait("Configuration.GetConfigurations");
    QVariantMap configurationMap = response.toMap().value("params").toMap();
    return configurationMap.value("basicConfiguration").toMap();
}

QVariantList TestConfigurations::loadBackupFiles()
{
    QVariant response = injectAndWait("Configuration.GetBackupFiles");
    QVariantMap responseMap = response.toMap().value("params").toMap();
    return responseMap.value("backupFiles").toList();
}

QWebSocket *TestConfigurations::openSocket()
{
    QWebSocket *socket = new QWebSocket("nymea configuration tests", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &TestConfigurations::sslErrors);

    QSignalSpy connectedSpy(socket, &QWebSocket::connected);
    socket->open(QUrl(QStringLiteral("wss://localhost:4444")));
    if (!connectedSpy.wait()) {
        socket->deleteLater();
        return nullptr;
    }

    return socket;
}

QVariant TestConfigurations::sendAndWait(QWebSocket *socket, int id, const QString &method, const QVariantMap &params, QVariantMap *notification)
{
    QVariantMap call;
    call.insert("id", id);
    call.insert("method", method);
    if (method.startsWith("JSONRPC.") || method.startsWith("Transfers.")) {
        call.insert("token", m_apiToken);
    }
    if (!params.isEmpty()) {
        call.insert("params", params);
    }

    const QByteArray payload = QJsonDocument::fromVariant(call).toJson(QJsonDocument::Compact);
    QSignalSpy spy(socket, &QWebSocket::textMessageReceived);
    socket->sendTextMessage(QString::fromUtf8(payload));

    while (spy.count() > 0 || spy.wait()) {
        for (int i = 0; i < spy.count(); ++i) {
            QJsonParseError error;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.at(i).at(0).toByteArray(), &error);
            if (error.error != QJsonParseError::NoError) {
                continue;
            }

            const QVariantMap response = jsonDoc.toVariant().toMap();
            if (response.contains("notification")) {
                if (notification) {
                    *notification = response;
                }
                continue;
            }

            if (response.value("id").toInt() == id) {
                return response;
            }
        }
        spy.clear();
    }

    return QVariant();
}

#include "testconfigurations.moc"
QTEST_MAIN(TestConfigurations)
