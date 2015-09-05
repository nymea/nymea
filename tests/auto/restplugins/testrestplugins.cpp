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
#include <QJsonDocument>
#include <QHttpPart>
#include <QMetaType>

using namespace guhserver;

class TestRestDeviceClasses: public GuhTestBase
{
    Q_OBJECT

private slots:
    void getPlugins();

    void getPluginConfiguration();

    void setPluginConfiguration_data();
    void setPluginConfiguration();
};

void TestRestDeviceClasses::getPlugins()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // Get all plugins
    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:3333/api/v1/plugins"));
    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantList pluginList = jsonDoc.toVariant().toList();
    QVERIFY2(pluginList.count() >= 1, "there should be at least one plugin.");

    // Get each of thouse devices individualy
    foreach (const QVariant &plugin, pluginList) {
        QVariantMap pluginMap = plugin.toMap();
        QNetworkRequest request(QUrl(QString("http://localhost:3333/api/v1/plugins/%1").arg(pluginMap.value("id").toString())));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/json");
        clientSpy.clear();
        QNetworkReply *reply = nam->get(request);
        clientSpy.wait();
        QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");
        jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
        QCOMPARE(error.error, QJsonParseError::NoError);

        reply->deleteLater();
    }
    nam->deleteLater();
}

void TestRestDeviceClasses::getPluginConfiguration()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // Get all plugins
    QNetworkRequest request;
    request.setUrl(QUrl(QString("http://localhost:3333/api/v1/plugins/%1/configuration").arg(mockPluginId.toString())));
    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantList configurations = jsonDoc.toVariant().toList();
    QVERIFY2(configurations.count() == 2, "there should be 2 configurations");
}

void TestRestDeviceClasses::setPluginConfiguration_data()
{

    QTest::addColumn<PluginId>("pluginId");
    QTest::addColumn<QVariantList>("newConfigurations");
    QTest::addColumn<int>("expectedStatusCode");

    QVariantMap validIntParam;
    validIntParam.insert("name","configParamInt");
    validIntParam.insert("value", 5);
    QVariantMap validBoolParam;
    validBoolParam.insert("name","configParamBool");
    validBoolParam.insert("value", false);
    QVariantMap invalidIntParam;
    invalidIntParam.insert("name","configParamInt");
    invalidIntParam.insert("value", 69);
    QVariantMap invalidIntParam2;
    invalidIntParam2.insert("name","configParamInt");
    invalidIntParam2.insert("value", -1);

    QVariantList validConfigurations;
    validConfigurations.append(validIntParam);
    validConfigurations.append(validBoolParam);

    QVariantList invalidConfigurations;
    invalidConfigurations.append(invalidIntParam);

    QVariantList invalidConfigurations2;
    invalidConfigurations2.append(invalidIntParam2);

    QTest::newRow("valid plugin configuration") << mockPluginId << validConfigurations  << 200;
    QTest::newRow("invalid plugin id") << PluginId::createPluginId() << validConfigurations  << 404;
    QTest::newRow("invalid plugin configuration") << mockPluginId << invalidConfigurations  << 400;
    QTest::newRow("invalid plugin configuration 2") << mockPluginId << invalidConfigurations2  << 400;
}

void TestRestDeviceClasses::setPluginConfiguration()
{
    QFETCH(PluginId, pluginId);
    QFETCH(QVariantList, newConfigurations);
    QFETCH(int, expectedStatusCode);

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    // Get plugin configuration
    QNetworkRequest request;
    request.setUrl(QUrl(QString("http://localhost:3333/api/v1/plugins/%1/configuration").arg(pluginId.toString())));
    QNetworkReply *reply = nam->get(request);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater();

    if (expectedStatusCode != 404) {
        QCOMPARE(statusCode, 200);
    } else {
        QCOMPARE(statusCode, expectedStatusCode);
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantList originalConfigurations = jsonDoc.toVariant().toList();
    QVERIFY2(originalConfigurations.count() == 2, "there should be 2 configurations");

    // Set new configuration
    clientSpy.clear();
    request.setUrl(QUrl(QString("http://localhost:3333/api/v1/plugins/%1/configuration").arg(pluginId.toString())));
    reply = nam->put(request, QJsonDocument::fromVariant(newConfigurations).toJson(QJsonDocument::Compact));
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatusCode);
    reply->deleteLater();

    if (expectedStatusCode != 200)
        return;

    // check new configurations
    clientSpy.clear();
    request.setUrl(QUrl(QString("http://localhost:3333/api/v1/plugins/%1/configuration").arg(pluginId.toString())));
    reply = nam->get(request);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, expectedStatusCode);
    data = reply->readAll();
    reply->deleteLater();

    jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantList checkConfigurations = jsonDoc.toVariant().toList();
    QVERIFY2(checkConfigurations.count() == 2, "there should be 2 configurations");

    // verify new configurations
    verifyParams(newConfigurations, checkConfigurations);

    // check new configurations after restart
    clientSpy.clear();
    request.setUrl(QUrl(QString("http://localhost:3333/api/v1/plugins/%1/configuration").arg(pluginId.toString())));
    reply = nam->get(request);
    clientSpy.wait();
    QCOMPARE(clientSpy.count(), 1);
    statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QCOMPARE(statusCode, 200);
    data = reply->readAll();
    reply->deleteLater();

    jsonDoc = QJsonDocument::fromJson(data, &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    checkConfigurations = jsonDoc.toVariant().toList();
    QVERIFY2(checkConfigurations.count() == 2, "there should be 2 configurations");

    // verify new configurations
    verifyParams(newConfigurations, checkConfigurations);

    nam->deleteLater();
}

#include "testrestplugins.moc"
QTEST_MAIN(TestRestDeviceClasses)
