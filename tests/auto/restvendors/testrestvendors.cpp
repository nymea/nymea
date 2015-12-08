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

class TestRestVendors: public GuhTestBase
{
    Q_OBJECT

private slots:
    void getVendors();
};

void TestRestVendors::getVendors()
{
    // Get all vendors
    QVariant response = getAndWait(QNetworkRequest(QUrl("http://localhost:3333/api/v1/vendors")));
    QVariantList vendorList = response.toList();
    QVERIFY2(vendorList.count() > 0, "Not enought vendors.");

    // Get each of thouse vendors individualy
    foreach (const QVariant &vendor, vendorList) {
        QVariantMap vendorMap = vendor.toMap();
        if (!VendorId(vendorMap.value("id").toString()).isNull()) {
            QNetworkRequest request(QUrl(QString("http://localhost:3333/api/v1/vendors/%1").arg(vendorMap.value("id").toString())));
            response = getAndWait(request);
            QVERIFY2(!response.isNull(), "Could not get vendor");
        }
    }
}


#include "testrestvendors.moc"
QTEST_MAIN(TestRestVendors)
