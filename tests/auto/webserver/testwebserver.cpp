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

#include <webserver/httpreply.h>

#include <QXmlReader>
#include <QRegularExpression>

using namespace nymeaserver;

class TestWebserver: public NymeaTestBase
{
    Q_OBJECT

private slots:
    void initTestCase();

    void coverageCalls();

    void httpVersion();

    void multiPackageMessage();

    void checkAllowedMethodCall_data();
    void checkAllowedMethodCall();

    void badRequests_data();
    void badRequests();

    void getFiles_data();
    void getFiles();

    void getServerDescription();

    void getIcons_data();
    void getIcons();

    void getDebugServer_data();
    void getDebugServer();

public slots:
    void onSslErrors(const QList<QSslError> &errors) {
        qCWarning(dcTests()) << "SSL errors:" << errors;
        QSslSocket *socket = static_cast<QSslSocket*>(sender());
        socket->ignoreSslErrors();
    }
};

void TestWebserver::initTestCase()
{
    NymeaTestBase::initTestCase("*.debug=false\nWebServer.debug=true\ntests.debug=true\nServerManager.debug=true");
    qDebug() << "TestWebserver starting";

    foreach (const WebServerConfiguration &config, NymeaCore::instance()->configuration()->webServerConfigurations()) {
        if (config.port == 3333 && (QHostAddress(config.address) == QHostAddress("127.0.0.1") || QHostAddress(config.address) == QHostAddress("0.0.0.0"))) {
            qDebug() << "Already have a webserver listening on 127.0.0.1:3333";
            return;
        }
    }

    qDebug() << "Creating new webserver instance on 127.0.0.1:3333";
    WebServerConfiguration config;
    config.address = "127.0.0.1";
    config.port = 3333;
    config.sslEnabled = true;
    config.restServerEnabled = true;
    NymeaCore::instance()->configuration()->setWebServerConfiguration(config);
}

void TestWebserver::coverageCalls()
{
    HttpReply *reply = new HttpReply(this);
    qDebug() << reply << reply->payload();
    qDebug() << reply->rawHeaderList();
    qDebug() << reply->rawHeader() << reply->isEmpty();
    reply->clear();
}

void TestWebserver::httpVersion()
{
    QSslSocket *socket = new QSslSocket(this);
    typedef void (QSslSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    connect(socket, static_cast<sslErrorsSignal>(&QSslSocket::sslErrors), this, &TestWebserver::onSslErrors);
    socket->connectToHostEncrypted("127.0.0.1", 3333);
    QSignalSpy encryptedSpy(socket, &QSslSocket::encrypted);
    bool encrypted = encryptedSpy.wait();
    QVERIFY2(encrypted, "could not created encrypted webserver connection.");

    QSignalSpy clientSpy(socket, &QSslSocket::readyRead);

    QByteArray requestData;
    requestData.append("GET /hello/nymea HTTP/1\r\n");
    requestData.append("User-Agent: nymea webserver test\r\n\r\n");

    quint64 count = socket->write(requestData);
    QVERIFY2(count > 0, "could not write to webserver.");

    clientSpy.wait(500);
    qWarning() << "spy count" << clientSpy.count();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    QByteArray data = socket->readAll();
    QVERIFY2(!data.isEmpty(), "got no response");

    QStringList lines = QString(data).split("\r\n");
    QStringList firstLineTokens = lines.first().split(QRegularExpression("[ \r\n][ \r\n]*"));

    QVERIFY2(firstLineTokens.isEmpty() || firstLineTokens.count() > 2, "could not get tokens of first line");

    bool ok = false;
    int statusCode = firstLineTokens.at(1).toInt(&ok);
    qDebug() << "have" << firstLineTokens;
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, 505);

    socket->close();
    socket->deleteLater();
}

void TestWebserver::multiPackageMessage()
{
    QSslSocket *socket = new QSslSocket(this);
    typedef void (QSslSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    connect(socket, static_cast<sslErrorsSignal>(&QSslSocket::sslErrors), this, &TestWebserver::onSslErrors);
    socket->connectToHostEncrypted("127.0.0.1", 3333);
    QSignalSpy encryptedSpy(socket, &QSslSocket::encrypted);
    bool encrypted = encryptedSpy.wait();
    QVERIFY2(encrypted, "could not created encrypte webserver connection.");

    QSignalSpy clientSpy(socket, &QSslSocket::readyRead);

    QByteArray requestData;
    requestData.append("PUT / HTTP/1.1\r\n");
    requestData.append("User-Agent: webserver test\r\n");
    requestData.append("Content-Length: 42\r\n");
    requestData.append("\r\n");
    requestData.append("This message");

    quint64 count = socket->write(requestData);
    QVERIFY2(count > 0, "could not write to webserver.");

    count = socket->write(QByteArray(" was sent"));
    QVERIFY2(count > 0, "could not write to webserver.");

    count = socket->write(QByteArray(" in four TCP"));
    QVERIFY2(count > 0, "could not write to webserver.");

    count = socket->write(QByteArray("packages. "));
    QVERIFY2(count > 0, "could not write to webserver.");

    clientSpy.wait(500);
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    QByteArray data = socket->readAll();
    QVERIFY2(!data.isEmpty(), "got no response");

    QStringList lines = QString(data).split("\r\n");
    QStringList firstLineTokens = lines.first().split(QRegularExpression("[ \r\n][ \r\n]*"));

    QVERIFY2(firstLineTokens.isEmpty() || firstLineTokens.count() > 2, "could not get tokens of first line");

    bool ok = false;
    int statusCode = firstLineTokens.at(1).toInt(&ok);
    qDebug() << "have" << firstLineTokens;
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, 501);

    socket->close();
    socket->deleteLater();
}

void TestWebserver::checkAllowedMethodCall_data()
{
    QTest::addColumn<QString>("method");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("GET") << "GET" << 200;
    QTest::newRow("PUT") <<  "PUT" << 200;
    QTest::newRow("POST") << "POST" << 200;
    QTest::newRow("DELETE") << "DELETE" << 200;
    QTest::newRow("OPTIONS") << "OPTIONS" << 200;
    QTest::newRow("HEAD") << "HEAD" << 405;
    QTest::newRow("CONNECT") << "CONNECT" << 405;
    QTest::newRow("TRACE") << "TRACE" << 405;
}

void TestWebserver::checkAllowedMethodCall()
{
    QFETCH(QString, method);
    QFETCH(int, expectedStatusCode);

    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [](QNetworkReply* reply, const QList<QSslError> &errors) {
        qCWarning(dcTests) << "SSL errors:" << errors;
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, &QNetworkAccessManager::finished);

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333"));
    QNetworkReply *reply = 0;

    clientSpy.clear();

    if (method == "GET") {
        reply = nam.get(request);
    } else if(method == "PUT") {
        reply = nam.put(request, QByteArray("Hello nymea!"));
    } else if(method == "POST") {
        reply = nam.post(request, QByteArray("Hello nymea!"));
    } else if(method == "DELETE") {
        reply = nam.deleteResource(request);
    } else if(method == "HEAD") {
        reply = nam.head(request);
    } else if(method == "CONNECT") {
        reply = nam.sendCustomRequest(request, "CONNECT");
    } else if(method == "OPTIONS") {
        QNetworkRequest req(QUrl("https://localhost:3333/api/v1/devices"));
        reply = nam.sendCustomRequest(req, "OPTIONS");
    } else if(method == "TRACE") {
        reply = nam.sendCustomRequest(request, "TRACE");
    } else {
        // just to make sure there will be a reply to delete
        reply = nam.get(request);
    }

    clientSpy.wait();

    QVERIFY2(clientSpy.count() > 0, "expected response");

    if (expectedStatusCode == 405){
        QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), expectedStatusCode);
        QVERIFY2(reply->hasRawHeader("Allow"), "405 should contain the allowed methods header");
    }

    reply->deleteLater();
}

void TestWebserver::badRequests_data()
{
    QTest::addColumn<QByteArray>("request");
    QTest::addColumn<int>("expectedStatusCode");

    QByteArray wrongContentLength;
    wrongContentLength.append("PUT / HTTP/1.1\r\n");
    wrongContentLength.append("User-Agent: webserver test\r\n");
    wrongContentLength.append("Content-Length: 1\r\n");
    wrongContentLength.append("\r\n");
    wrongContentLength.append("longer content than told in the header");

    QByteArray wrongHeaderFormatting;
    wrongHeaderFormatting.append("PUT / HTTP/1.1\r\n");
    wrongHeaderFormatting.append("User-Agent webserver test\r\n");
    wrongHeaderFormatting.append("Content-Length: 1\r\n");
    wrongHeaderFormatting.append("\r\n");

    QByteArray userAgentMissing;
    userAgentMissing.append("GET /index.html HTTP/1.1\r\n");
    userAgentMissing.append("\r\n");

    QTest::newRow("wrong content length") << wrongContentLength << 400;
    QTest::newRow("invalid header formatting") << wrongHeaderFormatting << 400;
    QTest::newRow("user agent missing") << userAgentMissing << 404;

}

void TestWebserver::badRequests()
{
    QFETCH(QByteArray, request);
    QFETCH(int, expectedStatusCode);

    QSslSocket *socket = new QSslSocket(this);
    typedef void (QSslSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    connect(socket, static_cast<sslErrorsSignal>(&QSslSocket::sslErrors), this, &TestWebserver::onSslErrors);
    socket->connectToHostEncrypted("127.0.0.1", 3333);
    QSignalSpy encryptedSpy(socket, &QSslSocket::encrypted);
    bool encrypted = encryptedSpy.wait();
    QVERIFY2(encrypted, "could not created encrypte webserver connection.");

    QSignalSpy clientSpy(socket, &QSslSocket::readyRead);

    socket->write(request);
    bool filesWritten = socket->waitForBytesWritten(500);
    QVERIFY2(filesWritten, "could not write to webserver.");

    clientSpy.wait(500);
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    QByteArray data = socket->readAll();
    QVERIFY2(!data.isEmpty(), "got no response");

    QStringList lines = QString(data).split("\r\n");
    QStringList firstLineTokens = lines.first().split(QRegularExpression("[ \r\n][ \r\n]*"));

    QVERIFY2(firstLineTokens.isEmpty() || firstLineTokens.count() > 2, "could not get tokens of first line");

    bool ok = false;
    int statusCode = firstLineTokens.at(1).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, expectedStatusCode);

    socket->close();
    socket->deleteLater();
}

void TestWebserver::getFiles_data()
{
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("get /etc/passwd") << "/etc/passwd" << 404;
    QTest::newRow("get /blub/blub/blabla") << "/etc/passwd" << 404;
    QTest::newRow("get /../../etc/passwd") << "/../../etc/passwd" << 404;
    QTest::newRow("get /../../") << "/../../" << 404;
    QTest::newRow("get /../") << "/../" << 404;
    QTest::newRow("get /etc/nymea/nymead.conf") << "/etc/nymea/nymead.conf" << 404;
    QTest::newRow("get /etc/sudoers") <<  "/etc/sudoers" << 404;
    QTest::newRow("get /root/.ssh/id_rsa.pub") <<  "/root/.ssh/id_rsa.pub" << 404;
}

void TestWebserver::getFiles()
{
    QFETCH(QString, query);
    QFETCH(int, expectedStatusCode);

    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, this, [](QNetworkReply* reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, &QNetworkAccessManager::finished);

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333" + query));
    QNetworkReply *reply = nam.get(request);

    clientSpy.wait();
    QVERIFY2(clientSpy.count() > 0, "expected response from webserver");

    bool ok = false;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, expectedStatusCode);

    reply->deleteLater();
}

void TestWebserver::getServerDescription()
{
    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, this, [](QNetworkReply* reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, &QNetworkAccessManager::finished);

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333/server.xml"));
    QNetworkReply *reply = nam.get(request);

    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");

    QXmlSimpleReader xmlReader; QXmlInputSource xmlSource;
    xmlSource.setData(reply->readAll());
    QVERIFY(xmlReader.parse(xmlSource));

    reply->deleteLater();
}

void TestWebserver::getIcons_data()
{
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("iconSize");

    QTest::newRow("get /icons/nymea-logo-8x8.png") << "/icons/nymea-logo-8x8.png" << 228;
    QTest::newRow("get /icons/nymea-logo-16x16.png") << "/icons/nymea-logo-16x16.png" << 392;
    QTest::newRow("get /icons/nymea-logo-22x22.png") << "/icons/nymea-logo-22x22.png" << 512;
    QTest::newRow("get /icons/nymea-logo-32x32.png") << "/icons/nymea-logo-32x32.png" << 747;
    QTest::newRow("get /icons/nymea-logo-48x48.png") << "/icons/nymea-logo-48x48.png" << 1282;
    QTest::newRow("get /icons/nymea-logo-64x64.png") << "/icons/nymea-logo-64x64.png" << 1825;
    QTest::newRow("get /icons/nymea-logo-120x120.png") << "/icons/nymea-logo-120x120.png" << 4090;
    QTest::newRow("get /icons/nymea-logo-128x128.png") << "/icons/nymea-logo-128x128.png" << 4453;
    QTest::newRow("get /icons/nymea-logo-256x256.png") << "/icons/nymea-logo-256x256.png" << 10763;
    QTest::newRow("get /icons/nymea-logo-512x512.png") << "/icons/nymea-logo-512x512.png" << 24287;
}

void TestWebserver::getIcons()
{
    QFETCH(QString, query);
    QFETCH(int, iconSize);

    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [](QNetworkReply* reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, &QNetworkAccessManager::finished);

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333" + query));
    QNetworkReply *reply = nam.get(request);

    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    QByteArray iconData = reply->readAll();
    QVERIFY(!iconData.isEmpty());
    QCOMPARE(iconData.size(), iconSize);

    reply->deleteLater();
}

void TestWebserver::getDebugServer_data()
{
    QTest::addColumn<QString>("method");
    QTest::addColumn<QString>("query");
    QTest::addColumn<bool>("serverEnabled");
    QTest::addColumn<int>("expectedStatusCode");

    // Resource files
    QTest::newRow("GET /debug/styles.css | server enabled | 200") << "get" << "/debug/styles.css" << true << 200;
    QTest::newRow("GET /debug/logo.svg | server enabled | 200") << "get" << "/debug/logo.svg" << true << 200;

    // Check redirection
    QTest::newRow("GET /debug/thisresourcedoesnotexist | server enabled | 308-200") << "get" << "/debug/thisresourcedoesnotexist" << true << 308;

    // debug enabled
    QTest::newRow("GET /debug | server enabled | 200") << "get" << "/debug" << true << 200;
    QTest::newRow("OPTIONS /debug | server enabled | 200") << "options" << "/debug" << true << 200;
    QTest::newRow("PUT /debug | server enabled | 405") << "put" << "/debug" << true << 405;
    QTest::newRow("POST /debug | server enabled | 405") << "post" << "/debug" << true << 405;
    QTest::newRow("DELETE /debug | server enabled | 405") << "delete" << "/debug" << true << 405;

    // debug disabled
    QTest::newRow("GET /debug | server disabled | 404") << "get" << "/debug" << false << 404;
    QTest::newRow("OPTIONS /debug | server disabled | 404") << "options" << "/debug" << false << 404;
    QTest::newRow("PUT /debug | server disabled | 404") << "put" << "/debug" << false << 404;
    QTest::newRow("POST /debug | server disabled | 404") << "post" << "/debug" << false << 404;
    QTest::newRow("DELETE /debug | server disabled | 404") << "delete" << "/debug" << false << 404;

    // styles.css enabled
    QTest::newRow("GET /debug/styles.css | server enabled | 200") << "get" << "/debug/styles.css" << true << 200;
    QTest::newRow("OPTIONS /debug/styles.css | server enabled | 200") << "options" << "/debug/styles.css" << true << 200;
    QTest::newRow("PUT /debug/styles.css | server enabled | 405") << "put" << "/debug/styles.css" << true << 405;
    QTest::newRow("POST /debug/styles.css | server enabled | 405") << "post" << "/debug/styles.css" << true << 405;
    QTest::newRow("DELETE /debug/styles.css | server enabled | 405") << "delete" << "/debug/styles.css" << true << 405;

    // styles.css disabled
    QTest::newRow("GET /debug/styles.css | server disabled | 404") << "get" << "/debug/styles.css" << false << 404;
    QTest::newRow("OPTIONS /debug/styles.css | server disabled | 404") << "options" << "/debug/styles.css" << false << 404;
    QTest::newRow("PUT /debug/styles.css | server disabled | 404") << "put" << "/debug/styles.css" << false << 404;
    QTest::newRow("POST /debug/styles.css | server disabled | 404") << "post" << "/debug/styles.css" << false << 404;
    QTest::newRow("DELETE /debug/styles.css | server disabled | 404") << "delete" << "/debug/styles.css" << false << 404;

    // Check if syslog is accessable
    QFileInfo syslogFileInfo("/var/log/syslog");

    if (!syslogFileInfo.exists()) {
        // syslog file doesn't exist
        QTest::newRow("GET /debug/syslog | server enabled | 200") << "get" << "/debug/syslog" << true << 404;
        QTest::newRow("OPTIONS /debug/syslog | server enabled | 200") << "options" << "/debug/syslog" << true << 404;
        QTest::newRow("PUT /debug/syslog | server enabled | 405") << "put" << "/debug/syslog" << true << 405;
        QTest::newRow("POST /debug/syslog | server enabled | 405") << "post" << "/debug/syslog" << true << 405;
        QTest::newRow("DELETE /debug/syslog | server enabled | 405") << "delete" << "/debug/syslog" << true << 405;
    } else if (!syslogFileInfo.isReadable()) {
        // syslog enabled, but not readable
        QTest::newRow("GET /debug/syslog | server enabled | 200") << "get" << "/debug/syslog" << true << 403;
        QTest::newRow("OPTIONS /debug/syslog | server enabled | 200") << "options" << "/debug/syslog" << true << 403;
        QTest::newRow("PUT /debug/syslog | server enabled | 405") << "put" << "/debug/syslog" << true << 405;
        QTest::newRow("POST /debug/syslog | server enabled | 405") << "post" << "/debug/syslog" << true << 405;
        QTest::newRow("DELETE /debug/syslog | server enabled | 405") << "delete" << "/debug/syslog" << true << 405;
    } else {
        // syslog enabled
        QTest::newRow("GET /debug/syslog | server enabled | 200") << "get" << "/debug/syslog" << true << 200;
        QTest::newRow("OPTIONS /debug/syslog | server enabled | 200") << "options" << "/debug/syslog" << true << 200;
        QTest::newRow("PUT /debug/syslog | server enabled | 405") << "put" << "/debug/syslog" << true << 405;
        QTest::newRow("POST /debug/syslog | server enabled | 405") << "post" << "/debug/syslog" << true << 405;
        QTest::newRow("DELETE /debug/syslog | server enabled | 405") << "delete" << "/debug/syslog" << true << 405;
    }

    // syslog disabled
    QTest::newRow("GET /debug/syslog | server enabled | 404") << "get" << "/debug/syslog" << false << 404;
    QTest::newRow("OPTIONS /debug/syslog | server enabled | 404") << "options" << "/debug/syslog" << false << 404;
    QTest::newRow("PUT /debug/syslog | server enabled | 404") << "put" << "/debug/syslog" << false << 404;
    QTest::newRow("POST /debug/syslog | server enabled | 404") << "post" << "/debug/syslog" << false << 404;
    QTest::newRow("DELETE /debug/syslog | server enabled | 404") << "delete" << "/debug/syslog" << false << 404;

    // settings/nymead enabled
    QTest::newRow("GET /debug/settings/nymead | server enabled | 200") << "get" << "/debug/settings/nymead" << true << 200;
    QTest::newRow("OPTIONS /debug/settings/nymead | server enabled | 200") << "options" << "/debug/settings/nymead" << true << 200;
    QTest::newRow("PUT /debug/settings/nymead | server enabled | 405") << "put" << "/debug/settings/nymead" << true << 405;
    QTest::newRow("POST /debug/settings/nymead | server enabled | 405") << "post" << "/debug/settings/nymead" << true << 405;
    QTest::newRow("DELETE /debug/settings/nymead | server enabled | 405") << "delete" << "/debug/settings/nymead" << true << 405;

    // settings/nymead disabled
    QTest::newRow("GET /debug/settings/nymead | server disabled | 404") << "get" << "/debug/settings/nymead" << false << 404;
    QTest::newRow("OPTIONS /debug/settings/nymead | server disabled | 404") << "options" << "/debug/settings/nymead" << false << 404;
    QTest::newRow("PUT /debug/settings/nymead | server disabled | 404") << "put" << "/debug/settings/nymead" << false << 404;
    QTest::newRow("POST /debug/settings/nymead | server disabled | 404") << "post" << "/debug/settings/nymead" << false << 404;
    QTest::newRow("DELETE /debug/settings/nymead | server disabled | 404") << "delete" << "/debug/settings/nymead" << false << 404;

    // settings/devices enabled
    QTest::newRow("GET /debug/settings/devices | server enabled | 200") << "get" << "/debug/settings/things" << true << 200;
    QTest::newRow("OPTIONS /debug/settings/devices | server enabled | 200") << "options" << "/debug/settings/things" << true << 200;
    QTest::newRow("PUT /debug/settings/devices | server enabled | 405") << "put" << "/debug/settings/things" << true << 405;
    QTest::newRow("POST /debug/settings/devices | server enabled | 405") << "post" << "/debug/settings/things" << true << 405;
    QTest::newRow("DELETE /debug/settings/devices | server enabled | 405") << "delete" << "/debug/settings/things" << true << 405;

    // settings/devices disabled
    QTest::newRow("GET /debug/settings/devices | server disabled | 404") << "get" << "/debug/settings/things" << false << 404;
    QTest::newRow("OPTIONS /debug/settings/devices | server disabled | 404") << "options" << "/debug/settings/things" << false << 404;
    QTest::newRow("PUT /debug/settings/devices | server disabled | 404") << "put" << "/debug/settings/things" << false << 404;
    QTest::newRow("POST /debug/settings/devices | server disabled | 404") << "post" << "/debug/settings/things" << false << 404;
    QTest::newRow("DELETE /debug/settings/devices | server disabled | 404") << "delete" << "/debug/settings/things" << false << 404;

    // settings/rules enabled
    QTest::newRow("GET /debug/settings/rules | server enabled | 200") << "get" << "/debug/settings/rules" << true << 200;
    QTest::newRow("OPTIONS /debug/settings/rules | server enabled | 200") << "options" << "/debug/settings/rules" << true << 200;
    QTest::newRow("PUT /debug/settings/rules | server enabled | 405") << "put" << "/debug/settings/rules" << true << 405;
    QTest::newRow("POST /debug/settings/rules | server enabled | 405") << "post" << "/debug/settings/rules" << true << 405;
    QTest::newRow("DELETE /debug/settings/rules | server enabled | 405") << "delete" << "/debug/settings/rules" << true << 405;

    // settings/rules disabled
    QTest::newRow("GET /debug/settings/rules | server disabled | 404") << "get" << "/debug/settings/rules" << false << 404;
    QTest::newRow("OPTIONS /debug/settings/rules | server disabled | 404") << "options" << "/debug/settings/rules" << false << 404;
    QTest::newRow("PUT /debug/settings/rules | server disabled | 404") << "put" << "/debug/settings/rules" << false << 404;
    QTest::newRow("POST /debug/settings/rules | server disabled | 404") << "post" << "/debug/settings/rules" << false << 404;
    QTest::newRow("DELETE /debug/settings/rules | server disabled | 404") << "delete" << "/debug/settings/rules" << false << 404;

    // settings/devicestates enabled
    QTest::newRow("GET /debug/settings/devicestates | server enabled | 200") << "get" << "/debug/settings/thingstates" << true << 200;
    QTest::newRow("OPTIONS /debug/settings/devicestates | server enabled | 200") << "options" << "/debug/settings/thingstates" << true << 200;
    QTest::newRow("PUT /debug/settings/devicestates | server enabled | 405") << "put" << "/debug/settings/thingstates" << true << 405;
    QTest::newRow("POST /debug/settings/devicestates | server enabled | 405") << "post" << "/debug/settings/thingstates" << true << 405;
    QTest::newRow("DELETE /debug/settings/devicestates | server enabled | 405") << "delete" << "/debug/settings/thingstates" << true << 405;

    // settings/devicestates disabled
    QTest::newRow("GET /debug/settings/devicestates | server disabled | 404") << "get" << "/debug/settings/thingstates" << false << 404;
    QTest::newRow("OPTIONS /debug/settings/devicestates | server disabled | 404") << "options" << "/debug/settings/thingstates" << false << 404;
    QTest::newRow("PUT /debug/settings/devicestates | server disabled | 404") << "put" << "/debug/settings/thingstates" << false << 404;
    QTest::newRow("POST /debug/settings/devicestates | server disabled | 404") << "post" << "/debug/settings/thingstates" << false << 404;
    QTest::newRow("DELETE /debug/settings/devicestates | server disabled | 404") << "delete" << "/debug/settings/thingstates" << false << 404;

    // settings/plugins enabled
    QTest::newRow("GET /debug/settings/plugins | server enabled | 200") << "get" << "/debug/settings/plugins" << true << 200;
    QTest::newRow("OPTIONS /debug/settings/plugins | server enabled | 200") << "options" << "/debug/settings/plugins" << true << 200;
    QTest::newRow("PUT /debug/settings/plugins | server enabled | 405") << "put" << "/debug/settings/plugins" << true << 405;
    QTest::newRow("POST /debug/settings/plugins | server enabled | 405") << "post" << "/debug/settings/plugins" << true << 405;
    QTest::newRow("DELETE /debug/settings/plugins | server enabled | 405") << "delete" << "/debug/settings/plugins" << true << 405;

    // settings/devicestates disabled
    QTest::newRow("GET /debug/settings/plugins | server disabled | 404") << "get" << "/debug/settings/plugins" << false << 404;
    QTest::newRow("OPTIONS /debug/settings/plugins | server disabled | 404") << "options" << "/debug/settings/plugins" << false << 404;
    QTest::newRow("PUT /debug/settings/plugins | server disabled | 404") << "put" << "/debug/settings/plugins" << false << 404;
    QTest::newRow("POST /debug/settings/plugins | server disabled | 404") << "post" << "/debug/settings/plugins" << false << 404;
    QTest::newRow("DELETE /debug/settings/plugins | server disabled | 404") << "delete" << "/debug/settings/plugins" << false << 404;


}

void TestWebserver::getDebugServer()
{
    QFETCH(QString, method);
    QFETCH(QString, query);
    QFETCH(bool, serverEnabled);
    QFETCH(int, expectedStatusCode);

    // Enable/disable debug server
    QVariantMap params; QVariant response;
    params.insert("enabled", serverEnabled);
    response = injectAndWait("Configuration.SetDebugServerEnabled", params);
    verifyError(response, "configurationError", "ConfigurationErrorNoError");

    QNetworkAccessManager nam;
    bool ok = false;
    int statusCode = 0;

    QSignalSpy clientSpy(&nam, &QNetworkAccessManager::finished);
    connect(&nam, &QNetworkAccessManager::sslErrors, this, [](QNetworkReply* reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });

    QNetworkReply *reply = nullptr;
    QNetworkRequest request;
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    // Note: in qt6 the request follows by default the redirect
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
#endif
    request.setUrl(QUrl("https://localhost:3333" + query));
    clientSpy.clear();

    if (method == "get") {
        reply = nam.get(request);
    } else if (method == "options") {
        reply = nam.sendCustomRequest(request, "OPTIONS");
    } else if (method == "post") {
        reply = nam.post(request, "");
    } else if (method == "delete") {
        reply = nam.deleteResource(request);
    } else if (method == "put") {
        reply = nam.put(request, "");
    }

    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");

    ok = false;
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, expectedStatusCode);
}

#include "testwebserver.moc"
QTEST_MAIN(TestWebserver)
