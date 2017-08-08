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

class TestRestPlugins: public GuhTestBase
{
    Q_OBJECT

private slots:
    void getPlugins();
    void invalidMethod();
    void invalidPath();

    void invalidPlugin_data();
    void invalidPlugin();

    void getPluginConfiguration();

//    void setPluginConfiguration_data();
//    void setPluginConfiguration();
};

void TestRestPlugins::getPlugins()
{
    // Get all plugins
    QVariant response = getAndWait(QNetworkRequest(QUrl("https://localhost:3333/api/v1/plugins")));
    QVariantList pluginList = response.toList();
    QVERIFY2(pluginList.count() > 0, "Not enought plugins.");

    // Get each of thouse plugins individualy
    foreach (const QVariant &plugin, pluginList) {
        QVariantMap pluginMap = plugin.toMap();
        if (!VendorId(pluginMap.value("id").toString()).isNull()) {
            QNetworkRequest request(QUrl(QString("https://localhost:3333/api/v1/plugins/%1").arg(pluginMap.value("id").toString())));
            response = getAndWait(request);
            QVERIFY2(!response.isNull(), "Could not get plugin");
        }
    }
}

void TestRestPlugins::invalidMethod()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::sslErrors, [this, nam](QNetworkReply *reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333/api/v1/plugins"));
    QNetworkReply *reply = nam->deleteResource(request);

    clientSpy.wait();
    QVERIFY2(clientSpy.count() != 0, "expected at least 1 response from webserver");

    bool ok = false;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, 400);

    reply->deleteLater();
    nam->deleteLater();
}

void TestRestPlugins::invalidPath()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    connect(nam, &QNetworkAccessManager::sslErrors, [this, nam](QNetworkReply *reply, const QList<QSslError> &) {
        reply->ignoreSslErrors();
    });
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("https://localhost:3333/api/v1/plugins/" + QUuid::createUuid().toString() + "/" + QUuid::createUuid().toString()));
    QNetworkReply *reply = nam->get(request);

    clientSpy.wait();
    QVERIFY2(clientSpy.count() != 0, "expected at least 1 response from webserver");

    bool ok = false;
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    QVERIFY2(ok, "Could not convert statuscode from response to int");
    QCOMPARE(statusCode, 501);

    reply->deleteLater();
    nam->deleteLater();
}

void TestRestPlugins::invalidPlugin_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("expectedStatusCode");

    QTest::newRow("invalid PluginId") << QUuid::createUuid().toString() << 404;
    QTest::newRow("invalid PluginId format") << "uuid" << 400;
}

void TestRestPlugins::invalidPlugin()
{
    QFETCH(QString, path);
    QFETCH(int, expectedStatusCode);

    QNetworkRequest request(QUrl("https://localhost:3333/api/v1/vendors/" + path));
    QVariant response = getAndWait(request, expectedStatusCode);
    QCOMPARE(JsonTypes::deviceErrorToString(DeviceManager::DeviceErrorVendorNotFound), response.toMap().value("error").toString());
}

void TestRestPlugins::getPluginConfiguration()
{
    // Get plugin config
    QNetworkRequest request(QUrl(QString("https://localhost:3333/api/v1/plugins/%1/configuration").arg(mockPluginId.toString())));
    QVariant response = getAndWait(request);

//    QVariantList configurations = response.toList();
//    QVERIFY2(configurations.count() == 2, "there should be 2 configurations");
}

//void TestRestPlugins::setPluginConfiguration_data()
//{
//    QTest::addColumn<PluginId>("pluginId");
//    QTest::addColumn<QVariantList>("newConfigurations");
//    QTest::addColumn<int>("expectedStatusCode");

//    QVariantMap validIntParam;
//    validIntParam.insert("name","configParamInt");
//    validIntParam.insert("value", 5);
//    QVariantMap validBoolParam;
//    validBoolParam.insert("name","configParamBool");
//    validBoolParam.insert("value", false);
//    QVariantMap invalidIntParam;
//    invalidIntParam.insert("name","configParamInt");
//    invalidIntParam.insert("value", 69);
//    QVariantMap invalidIntParam2;
//    invalidIntParam2.insert("name","configParamInt");
//    invalidIntParam2.insert("value", -1);

//    QVariantList validConfigurations;
//    validConfigurations.append(validIntParam);
//    validConfigurations.append(validBoolParam);

//    QVariantList invalidConfigurations;
//    invalidConfigurations.append(invalidIntParam);

//    QVariantList invalidConfigurations2;
//    invalidConfigurations2.append(invalidIntParam2);

////    QTest::newRow("valid plugin configuration") << mockPluginId << validConfigurations  << 200;
////    QTest::newRow("invalid plugin id") << PluginId::createPluginId() << validConfigurations  << 404;
////    QTest::newRow("invalid plugin configuration") << mockPluginId << invalidConfigurations  << 400;
////    QTest::newRow("invalid plugin configuration 2") << mockPluginId << invalidConfigurations2  << 400;
//}

//void TestRestPlugins::setPluginConfiguration()
//{
//    QFETCH(PluginId, pluginId);
//    QFETCH(QVariantList, newConfigurations);
//    QFETCH(int, expectedStatusCode);

//    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
//    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

//    // Get plugin configuration
//    QNetworkRequest request;
//    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/plugins/%1/configuration").arg(pluginId.toString())));
//    QNetworkReply *reply = nam->get(request);
//    clientSpy.wait();
//    QCOMPARE(clientSpy.count(), 1);
//    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//    reply->deleteLater();
//    if (expectedStatusCode == 404)
//        return;

//    QByteArray data = reply->readAll();
//    reply->deleteLater();
//    QJsonParseError error;
//    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
//    QCOMPARE(error.error, QJsonParseError::NoError);
//    QVariantList originalConfigurations = jsonDoc.toVariant().toList();
//    QVERIFY2(originalConfigurations.count() == 2, "there should be 2 configurations");

//    // Set new configuration
//    clientSpy.clear();
//    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/plugins/%1/configuration").arg(pluginId.toString())));
//    reply = nam->put(request, QJsonDocument::fromVariant(newConfigurations).toJson(QJsonDocument::Compact));
//    clientSpy.wait();
//    QCOMPARE(clientSpy.count(), 1);
//    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//    QCOMPARE(statusCode, expectedStatusCode);
//    reply->deleteLater();

//    if (expectedStatusCode != 200)
//        return;

//    // check new configurations
//    clientSpy.clear();
//    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/plugins/%1/configuration").arg(pluginId.toString())));
//    reply = nam->get(request);
//    clientSpy.wait();
//    QCOMPARE(clientSpy.count(), 1);
//    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//    QCOMPARE(statusCode, expectedStatusCode);
//    data = reply->readAll();
//    reply->deleteLater();

//    jsonDoc = QJsonDocument::fromJson(data, &error);
//    QCOMPARE(error.error, QJsonParseError::NoError);
//    QVariantList checkConfigurations = jsonDoc.toVariant().toList();
//    //QVERIFY2(checkConfigurations.count() == 2, "there should be 2 configurations");

//    // verify new configurations
//    verifyParams(newConfigurations, checkConfigurations);

//    // check new configurations after restart
//    clientSpy.clear();
//    request.setUrl(QUrl(QString("https://localhost:3333/api/v1/plugins/%1/configuration").arg(pluginId.toString())));
//    reply = nam->get(request);
//    clientSpy.wait();
//    QCOMPARE(clientSpy.count(), 1);
//    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//    QCOMPARE(statusCode, 200);
//    data = reply->readAll();
//    reply->deleteLater();

//    jsonDoc = QJsonDocument::fromJson(data, &error);
//    QCOMPARE(error.error, QJsonParseError::NoError);
//    checkConfigurations = jsonDoc.toVariant().toList();
//    //QVERIFY2(checkConfigurations.count() == 2, "there should be 2 configurations");

//    // verify new configurations
//    verifyParams(newConfigurations, checkConfigurations);

//    nam->deleteLater();
//}

#include "testrestplugins.moc"
QTEST_MAIN(TestRestPlugins)
