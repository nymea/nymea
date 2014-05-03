/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#include "testdevices.h"
#include "guhcore.h"
#include "devicemanager.h"

#include <QDebug>
#include <QSignalSpy>

TestDevices::TestDevices(QObject *parent) :
    GuhTestBase(parent)
{
}

void TestDevices::getSupportedVendors()
{
    QVariant supportedVendors = injectAndWait("Devices.GetSupportedVendors");
    qDebug() << "response" << supportedVendors;

    // Make sure there is exactly 1 supported Vendor named "guh"
    QVariantList vendorList = supportedVendors.toMap().value("params").toMap().value("vendors").toList();
    QCOMPARE(vendorList.count(), 1);
    VendorId vendorId = VendorId(vendorList.first().toMap().value("id").toString());
    QCOMPARE(vendorId, guhVendorId);
}

void TestDevices::getSupportedDevices_data()
{
    QTest::addColumn<VendorId>("vendorId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("vendor guh") << guhVendorId << 2;
    QTest::newRow("no filter") << VendorId() << 2;
    QTest::newRow("invalid vendor") << VendorId("93e7d361-8025-4354-b17e-b68406c800bc") << 0;
}

void TestDevices::getSupportedDevices()
{
    QFETCH(VendorId, vendorId);
    QFETCH(int, resultCount);

    QVariantMap params;
    if (!vendorId.isNull()) {
        params.insert("vendorId", vendorId);
    }
    QVariant supportedDevices = injectAndWait("Devices.GetSupportedDevices", params);

    // Make sure there is exactly 1 supported device class with the name Mock Wifi Device
    QCOMPARE(supportedDevices.toMap().value("params").toMap().value("deviceClasses").toList().count(), resultCount);
    if (resultCount > 0) {
        QString deviceName = supportedDevices.toMap().value("params").toMap().value("deviceClasses").toList().first().toMap().value("name").toString();
        QVERIFY(deviceName.startsWith(QString("Mock Device")));
    }
}

void TestDevices::addConfiguredDevice_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<QVariantMap>("deviceParams");
    QTest::addColumn<bool>("success");

    QVariantMap deviceParams;
    deviceParams.insert("httpport", m_mockDevice1Port - 1);
    QTest::newRow("User, JustAdd") << mockDeviceClassId << deviceParams << true;
    QTest::newRow("Auto, JustAdd") << mockDeviceAutoClassId << deviceParams << false;
}

void TestDevices::addConfiguredDevice()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(QVariantMap, deviceParams);
    QFETCH(bool, success);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    qDebug() << "response is" << response;

    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), success);

    if (success) {
        QUuid deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
        params.clear();
        params.insert("deviceId", deviceId.toString());
        injectAndWait("Devices.RemoveConfiguredDevice", params);
        QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);
    }
}

void TestDevices::getConfiguredDevices()
{
    QVariant response = injectAndWait("Devices.GetConfiguredDevices");

    QVariantList devices = response.toMap().value("params").toMap().value("devices").toList();
    qDebug() << "got devices" << devices;
    QCOMPARE(devices.count(), 2); // There should be one auto created mock device and the one created in initTestcase()
}

void TestDevices::removeDevice()
{
    QVERIFY(!m_mockDeviceId.isNull());
    QSettings settings;
    settings.beginGroup(m_mockDeviceId.toString());
    // Make sure we have some config values for this device
    QVERIFY(settings.allKeys().count() > 0);

    QVariantMap params;
    params.insert("deviceId", m_mockDeviceId);

    QVariant response = injectAndWait("Devices.RemoveConfiguredDevice", params);

    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);

    // Make sure the device is gone from settings too
    QCOMPARE(settings.allKeys().count(), 0);
}

void TestDevices::storedDevices()
{
    QVariantMap params;
    params.insert("deviceClassId", mockDeviceClassId);
    QVariantMap deviceParams;
    deviceParams.insert("httpport", 8888);
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    QCOMPARE(response.toMap().value("params").toMap().value("success").toBool(), true);
    DeviceId addedDeviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!addedDeviceId.isNull());

    // Destroy and recreate the core instance to check if settings are loaded at startup
    GuhCore::instance()->destroy();
    QSignalSpy spy(GuhCore::instance()->deviceManager(), SIGNAL(loaded()));
    spy.wait();
    m_mockTcpServer = MockTcpServer::servers().first();


    response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

    foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
        qDebug() << "found stored device" << device;
        if (DeviceId(device.toMap().value("id").toString()) == addedDeviceId) {
//            foreach (const QVariant &paramVariant, device.toMap().value("params").toList()) {
//                if ()
//            }

            qDebug() << "found added device" << device.toMap().value("params").toList().first().toMap();
            qDebug() << "expected deviceParams:" << deviceParams;
            QCOMPARE(device.toMap().value("params").toList().first().toMap(), deviceParams);
        }
    }

    params.clear();
    params.insert("deviceId", addedDeviceId);
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
}

