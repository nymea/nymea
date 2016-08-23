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
    QVariantMap configurations = injectAndWait("Configuration.GetConfigurations").toMap().value("params").toMap();
    QString currentTimeZone = configurations.value("basicConfiguration").toMap().value("timeZone").toString();
    QString currentTime = configurations.value("basicConfiguration").toMap().value("serverTime").toString();
    qDebug() << currentTimeZone << QDateTime::fromTime_t(currentTime.toInt());
    QVariantList timeZones = injectAndWait("Configuration.GetTimeZones").toMap().value("params").toMap().value("timeZones").toList();
    QVERIFY(timeZones.count() > 0);
    QVERIFY(timeZones.contains("America/Toronto"));

}

#include "testconfigurations.moc"
QTEST_MAIN(TestConfigurations)
