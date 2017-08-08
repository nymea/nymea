/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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
#include "rest/restresource.h"
#include "webserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QMetaType>
#include <QByteArray>
#include <QXmlReader>

using namespace guhserver;

class TestWebserver: public GuhTestBase
{
    Q_OBJECT

private slots:
    void coverageCalls();

    void httpVersion();

    void multiPackageMessage();

    void checkAllowedMethodCall_data();
    void checkAllowedMethodCall();

    void badRequests_data();
    void badRequests();

    void getOptions_data();
    void getOptions();

    void getFiles_data();
    void getFiles();

    void getServerDescription();

    void getIcons_data();
    void getIcons();

public slots:
    void onSslErrors(const QList<QSslError> &) {
        qWarning() << "SSL error";
        QSslSocket *socket = static_cast<QSslSocket*>(sender());
        socket->ignoreSslErrors();
    }
};

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
    QSignalSpy encryptedSpy(socket, SIGNAL(encrypted()));
    bool encrypted = encryptedSpy.wait();
    QVERIFY2(encrypted, "could not created encrypte webserver connection.");

    QSignalSpy clientSpy(socket, SIGNAL(readyRead()));

    QByteArray requestData;
    requestData.append("GET /hello/guh HTTP/1\r\n");
    requestData.append("User-Agent: guh webserver test\r\n\r\n");

    quint64 count = socket->write(requestData);
    QVERIFY2(count > 0, "could not write to webserver.");

    clientSpy.wait(500);
    qWarning() << "spy count" << clientSpy.count();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    QByteArray data = socket->readAll();
    QVERIFY2(!data.isEmpty(), "got no response");

    QStringList lines = QString(data).split("\r\n");
    QStringList firstLineTokens = lines.first().split(QRegExp("[ \r\n][ \r\n]*"));

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
    QSignalSpy encryptedSpy(socket, SIGNAL(encrypted()));
    bool encrypted = encryptedSpy.wait();
    QVERIFY2(encrypted, "could not created encrypte webserver connection.");

    QSignalSpy clientSpy(socket, SIGNAL(readyRead()));

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
    QStringList firstLineTokens = lines.first().split(QRegExp("[ \r\n][ \r\n]*"));

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
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply* reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333"));
    QNetworkReply *reply = 0;

    clientSpy.clear();

    if (method == "GET") {
        reply = nam.get(request);
    } else if(method == "PUT") {
        reply = nam.put(request, QByteArray("Hello guh!"));
    } else if(method == "POST") {
        reply = nam.post(request, QByteArray("Hello guh!"));
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
        // just to make shore there will be a reply to delete
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
    userAgentMissing.append("GET /abc HTTP/1.1\r\n");
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
    QSignalSpy encryptedSpy(socket, SIGNAL(encrypted()));
    bool encrypted = encryptedSpy.wait();
    QVERIFY2(encrypted, "could not created encrypte webserver connection.");

    QSignalSpy clientSpy(socket, SIGNAL(readyRead()));

    socket->write(request);
    bool filesWritten = socket->waitForBytesWritten(500);
    QVERIFY2(filesWritten, "could not write to webserver.");

    clientSpy.wait(500);
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    QByteArray data = socket->readAll();
    QVERIFY2(!data.isEmpty(), "got no response");

    QStringList lines = QString(data).split("\r\n");
    QStringList firstLineTokens = lines.first().split(QRegExp("[ \r\n][ \r\n]*"));

    QVERIFY2(firstLineTokens.isEmpty() || firstLineTokens.count() > 2, "could not get tokens of first line");

    bool ok = false;
    int statusCode = firstLineTokens.at(1).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, expectedStatusCode);

    socket->close();
    socket->deleteLater();
}

void TestWebserver::getOptions_data()
{
    QTest::addColumn<QString>("path");

    QTest::newRow("get OPTIONS /api/v1/devices") << "/api/v1/devices";
    QTest::newRow("get OPTIONS /api/v1/devices/pair") << "/api/v1/devices/pair";
    QTest::newRow("get OPTIONS /api/v1/devices/confirmpairing") << "/api/v1/devices/confirmpairing";
    QTest::newRow("get OPTIONS /api/v1/rules") << "/api/v1/rules";
    QTest::newRow("get OPTIONS /api/v1/plugins") << "/api/v1/plugins";
    QTest::newRow("get OPTIONS /api/v1/logs") << "/api/v1/logs";
    QTest::newRow("get OPTIONS /api/v1/deviceclasses") << "/api/v1/deviceclasses";
    QTest::newRow("get OPTIONS /api/v1/vendors") << "/api/v1/vendors";
}

void TestWebserver::getOptions()
{
    QFETCH(QString, path);

    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply* reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333" + path));
    QNetworkReply *reply = nam.sendCustomRequest(request, "OPTIONS");

    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);

    bool ok = false;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, 200);

    reply->deleteLater();
}

void TestWebserver::getFiles_data()
{
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("get /etc/passwd") << "/etc/passwd" << 404;
    QTest::newRow("get /blub/blub/blabla") << "/etc/passwd" << 404;
    QTest::newRow("get /../../etc/passwd") << "/../../etc/passwd" << 404;
    QTest::newRow("get /../../") << "/../../" << 403;
    QTest::newRow("get /../") << "/../" << 403;
    QTest::newRow("get /etc/guh/guhd.conf") << "/etc/guh/guhd.conf" << 404;
    QTest::newRow("get /etc/sudoers") <<  "/etc/sudoers" << 404;
    QTest::newRow("get /root/.ssh/id_rsa.pub") <<  "/root/.ssh/id_rsa.pub" << 404;
}

void TestWebserver::getFiles()
{
    QFETCH(QString, query);
    QFETCH(int, expectedStatusCode);

    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply* reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

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
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply* reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

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

    QTest::newRow("get /icons/guh-logo-8x8.png") << "/icons/guh-logo-8x8.png" << 228;
    QTest::newRow("get /icons/guh-logo-16x16.png") << "/icons/guh-logo-16x16.png" << 392;
    QTest::newRow("get /icons/guh-logo-22x22.png") << "/icons/guh-logo-22x22.png" << 512;
    QTest::newRow("get /icons/guh-logo-32x32.png") << "/icons/guh-logo-32x32.png" << 747;
    QTest::newRow("get /icons/guh-logo-48x48.png") << "/icons/guh-logo-48x48.png" << 1282;
    QTest::newRow("get /icons/guh-logo-64x64.png") << "/icons/guh-logo-64x64.png" << 1825;
    QTest::newRow("get /icons/guh-logo-120x120.png") << "/icons/guh-logo-120x120.png" << 4090;
    QTest::newRow("get /icons/guh-logo-128x128.png") << "/icons/guh-logo-128x128.png" << 4453;
    QTest::newRow("get /icons/guh-logo-256x256.png") << "/icons/guh-logo-256x256.png" << 10763;
    QTest::newRow("get /icons/guh-logo-512x512.png") << "/icons/guh-logo-512x512.png" << 24287;
}

void TestWebserver::getIcons()
{
    QFETCH(QString, query);
    QFETCH(int, iconSize);

    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply* reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

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

#include "testwebserver.moc"
QTEST_MAIN(TestWebserver)
