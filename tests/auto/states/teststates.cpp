/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

class TestStates: public GuhTestBase
{
    Q_OBJECT

private slots:
    void getStateTypes();

    void getStateValue_data();
    void getStateValue();
};

void TestStates::getStateTypes()
{
    QVariantMap params;
    params.insert("deviceClassId", mockDeviceClassId);
    QVariant response = injectAndWait("Devices.GetStateTypes", params);
    QVERIFY(!response.isNull());
    //verifyDeviceError(response);
}

void TestStates::getStateValue_data()
{
    QList<Device*> devices = GuhCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Device *device = devices.first();

    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("existing state") << device->id() << mockIntStateId << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid device") << DeviceId::createDeviceId() << mockIntStateId << DeviceManager::DeviceErrorDeviceNotFound;
    QTest::newRow("invalid statetype") << device->id() << StateTypeId::createStateTypeId() << DeviceManager::DeviceErrorStateTypeNotFound;
}

void TestStates::getStateValue()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(DeviceManager::DeviceError, error);

    QVariantMap params;
    params.insert("deviceId", deviceId.toString());
    params.insert("stateTypeId", stateTypeId.toString());

    QVariant response = injectAndWait("Devices.GetStateValue", params);

    verifyDeviceError(response, error);
}

#include "teststates.moc"
QTEST_MAIN(TestStates)
