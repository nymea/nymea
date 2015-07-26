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
#include <QMetaType>

using namespace guhserver;

class TestRestDevices: public GuhTestBase
{
    Q_OBJECT

private slots:
    void getConfiguredDevices();


private:
    // for debugging
    void printResponse(QNetworkReply *reply);

};

void TestRestDevices::getConfiguredDevices()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy clientSpy(nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:3000/api/v1/devices"));
    QNetworkReply *reply;

    reply = nam->get(request);
    clientSpy.wait(200);
    QVERIFY2(clientSpy.count() == 1, "expected exactly 1 response from webserver");

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
    QCOMPARE(error.error, QJsonParseError::NoError);
    QVariantList deviceList = jsonDoc.toVariant().toList();
    QCOMPARE(deviceList.count(), 3);
    reply->deleteLater();
}

void TestRestDevices::printResponse(QNetworkReply *reply)
{
    qDebug() << "-------------------------------";
    qDebug() << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    foreach (const  QNetworkReply::RawHeaderPair &headerPair, reply->rawHeaderPairs()) {
        qDebug() << headerPair.first << ":" << headerPair.second;
    }
    qDebug() << "-------------------------------";
    qDebug() << reply->readAll();
    qDebug() << "-------------------------------";
}

#include "testrestdevices.moc"
QTEST_MAIN(TestRestDevices)
