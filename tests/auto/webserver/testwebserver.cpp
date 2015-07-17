/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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
#include "webserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QMetaType>

using namespace guhserver;

class TestWebserver: public GuhTestBase
{
    Q_OBJECT

private slots:
    void pingServer();
    void httpVersion();

    void checkAllowedMethodCall_data();
    void checkAllowedMethodCall();

    void getFiles_data();
    void getFiles();

private:
    // for debugging
    void printResponse(QNetworkReply *reply);

};

void TestWebserver::pingServer()
{
    // TODO: when QWebsocket will be used
}

void TestWebserver::httpVersion()
{
    QTcpSocket *socket = new QTcpSocket(this);
    socket->connectToHost(QHostAddress("127.0.0.1"), 3000);
    bool connected = socket->waitForConnected(1000);
    QVERIFY2(connected, "could not connect to webserver.");

    QSignalSpy clientSpy(socket, SIGNAL(readyRead()));

    socket->write("Confusing, non HTTP protocol stuff which should not be accepted.");
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
    QCOMPARE(statusCode, 505);

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
    QTest::newRow("HEAD") << "HEAD" << 405;
    QTest::newRow("CONNECT") << "CONNECT" << 405;
    QTest::newRow("OPTIONS") << "OPTIONS" << 405;
    QTest::newRow("TRACE") << "TRACE" << 405;
}

void TestWebserver::checkAllowedMethodCall()
{
    QFETCH(QString, method);
    QFETCH(int, expectedStatusCode);

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:3000"));
    QNetworkReply *reply;

    if (method == "GET") {
        reply = nam->get(request);
    } else if(method == "PUT") {
        reply = nam->put(request, QByteArray("Hello guh!"));
    } else if(method == "POST") {
        reply = nam->post(request, QByteArray("Hello guh!"));
    } else if(method == "DELETE") {
        reply = nam->deleteResource(request);
    } else if(method == "HEAD") {
        reply = nam->head(request);
    } else if(method == "CONNECT") {
        reply = nam->sendCustomRequest(request, "CONNECT");
    } else if(method == "OPTIONS") {
        reply = nam->sendCustomRequest(request, "OPTIONS");
    } else if(method == "TRACE") {
        reply = nam->sendCustomRequest(request, "TRACE");
    }

    clientSpy.wait(200);
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
    if (expectedStatusCode == 405){
        QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), expectedStatusCode);
        QVERIFY2(reply->hasRawHeader("Allow"), "405 should contain the allowed methods header");
    }

    reply->deleteLater();
}

void TestWebserver::getFiles_data()
{
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("get /etc/passwd") << "/etc/passwd" << 404;
    QTest::newRow("get /etc/guh/guhd.conf") << "/etc/guh/guhd.conf" << 404;
    QTest::newRow("get /etc/sudoers") <<  "/etc/sudoers" << 404;
    QTest::newRow("get /root/.ssh/id_rsa.pub") <<  "/root/.ssh/id_rsa.pub" << 404;


}

void TestWebserver::getFiles()
{
    QFETCH(QString, query);
    QFETCH(int, expectedStatusCode);

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:3000" + query));
    QNetworkReply *reply = nam->get(request);

    clientSpy.wait(200);
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");

    printResponse(reply);

    bool ok = false;
    qDebug() << reply->readAll();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, expectedStatusCode);

    reply->deleteLater();
}

void TestWebserver::printResponse(QNetworkReply *reply)
{
    qDebug() << "-------------------------------";
    qDebug() << "Response header:";
    qDebug() << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    foreach (const  QNetworkReply::RawHeaderPair &headerPair, reply->rawHeaderPairs()) {
        qDebug() << headerPair.first << ":" << headerPair.second;
    }
    qDebug() << "-------------------------------";
    qDebug() << "Response payload";
    qDebug() << reply->readAll();
    qDebug() << "-------------------------------";
}

#include "testwebserver.moc"
QTEST_MAIN(TestWebserver)
