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
#include "webserver.h"

#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QHttpPart>
#include <QMetaType>

using namespace guhserver;

class TestRestVendors: public GuhTestBase
{
    Q_OBJECT

private slots:
    void getVendors();
    void invalidMethod();
    void invalidPath();

    void invalidVendor_data();
    void invalidVendor();
};

void TestRestVendors::getVendors()
{
    // Get all vendors
    QVariant response = getAndWait(QNetworkRequest(QUrl("https://localhost:3333/api/v1/vendors")));
    QVariantList vendorList = response.toList();
    QVERIFY2(vendorList.count() > 0, "Not enought vendors.");

    // Get each of thouse vendors individualy
    foreach (const QVariant &vendor, vendorList) {
        QVariantMap vendorMap = vendor.toMap();
        if (!VendorId(vendorMap.value("id").toString()).isNull()) {
            QNetworkRequest request(QUrl(QString("https://localhost:3333/api/v1/vendors/%1").arg(vendorMap.value("id").toString())));
            response = getAndWait(request);
            QVERIFY2(!response.isNull(), "Could not get vendor");
        }
    }
}

void TestRestVendors::invalidMethod()
{
    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply *reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333/api/v1/vendors"));
    QNetworkReply *reply = nam.post(request, QByteArray());

    clientSpy.wait();
    QVERIFY2(clientSpy.count() != 0, "expected at least 1 response from webserver");

    bool ok = false;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, 400);

    reply->deleteLater();
}

void TestRestVendors::invalidPath()
{
    QNetworkAccessManager nam;
    connect(&nam, &QNetworkAccessManager::sslErrors, [this, &nam](QNetworkReply *reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333/api/v1/vendors/" + QUuid::createUuid().toString() + "/" + QUuid::createUuid().toString()));
    QNetworkReply *reply = nam.get(request);

    clientSpy.wait();
    QVERIFY2(clientSpy.count() != 0, "expected at least 1 response from webserver");

    bool ok = false;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, 501);

    reply->deleteLater();
}

void TestRestVendors::invalidVendor_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("invalid VendorId") << QUuid::createUuid().toString() << 404;
    QTest::newRow("invalid VendorId format") << "uuid" << 400;
}

void TestRestVendors::invalidVendor()
{
    QFETCH(QString, path);
    QFETCH(int, expectedStatusCode);

    QNetworkRequest request(QUrl("https://localhost:3333/api/v1/vendors/" + path));
    QVariant response = getAndWait(request, expectedStatusCode);
    QCOMPARE(JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorVendorNotFound), response.toMap().value("error").toString());
}

#include "testrestvendors.moc"
QTEST_MAIN(TestRestVendors)
