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

#include "guhtestbase.h"
#include "guhcore.h"
#include "devicemanager.h"
#include "plugin/deviceplugin.h"

#include <QDebug>
#include <QSignalSpy>

class TestDevices : public GuhTestBase
{
    Q_OBJECT

private slots:
    void getPlugins();

    void getPluginConfig_data();
    void getPluginConfig();

    void setPluginConfig_data();
    void setPluginConfig();

    void getSupportedVendors();

    void getSupportedDevices_data();
    void getSupportedDevices();

    void addConfiguredDevice_data();
    void addConfiguredDevice();

    void getConfiguredDevices();

    void removeDevice_data();
    void removeDevice();

    void storedDevices();

    void discoverDevices_data();
    void discoverDevices();

    void getActionTypes_data();
    void getActionTypes();

    void getEventTypes_data();
    void getEventTypes();

    void getStateTypes_data();
    void getStateTypes();
};

void TestDevices::getPlugins()
{
    QVariant response = injectAndWait("Devices.GetPlugins");

    QVariantList plugins = response.toMap().value("params").toMap().value("plugins").toList();

    QCOMPARE(plugins.count(), 1);
    QCOMPARE(PluginId(plugins.first().toMap().value("id").toString()), mockPluginId);
}

void TestDevices::getPluginConfig_data()
{
    QTest::addColumn<PluginId>("pluginId");
    QTest::addColumn<bool>("success");

    QTest::newRow("valid plugin") << mockPluginId << true;
    QTest::newRow("invalid plugin") << PluginId::createPluginId() << false;
}

void TestDevices::getPluginConfig()
{
    QFETCH(PluginId, pluginId);
    QFETCH(bool, success);

    QVariantMap params;
    params.insert("pluginId", pluginId);
    QVariant response = injectAndWait("Devices.GetPluginConfiguration", params);
    verifySuccess(response, success);
}

void TestDevices::setPluginConfig_data()
{
    QTest::addColumn<PluginId>("pluginId");
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<bool>("success");

    QTest::newRow("valid") << mockPluginId << QVariant(13) << true;
    QTest::newRow("invalid plugin") << PluginId::createPluginId() << QVariant(13) <<  false;
    QTest::newRow("too big") << mockPluginId << QVariant(130) << false;
    QTest::newRow("too small") << mockPluginId << QVariant(-13) << false;
    QTest::newRow("wrong type") << mockPluginId << QVariant("wrontType") << false;
}

void TestDevices::setPluginConfig()
{
    QFETCH(PluginId, pluginId);
    QFETCH(QVariant, value);
    QFETCH(bool, success);

    QVariantMap params;
    params.insert("pluginId", pluginId);

    QVariantList configuration;
    QVariantMap configParam;
    configParam.insert("name", "configParamInt");
    configParam.insert("value", value);
    configuration.append(configParam);
    params.insert("configuration", configuration);
    QVariant response = injectAndWait("Devices.SetPluginConfiguration", params);
    verifySuccess(response, success);

    if (success) {
        params.clear();
        params.insert("pluginId", pluginId);
        response = injectAndWait("Devices.GetPluginConfiguration", params);
        verifySuccess(response);
        qDebug() << "222" << response.toMap().value("params").toMap().value("configuration").toList().first();
        QVERIFY2(response.toMap().value("params").toMap().value("configuration").toList().first().toMap().value("name") == "configParamInt", "Value not set correctly");
        QVERIFY2(response.toMap().value("params").toMap().value("configuration").toList().first().toMap().value("value") == value, "Value not set correctly");
    }
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

    QTest::newRow("vendor guh") << guhVendorId << 6;
    QTest::newRow("no filter") << VendorId() << 6;
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

    // Make sure there are the right amount of supported device classes with the name Mock Device
    QCOMPARE(supportedDevices.toMap().value("params").toMap().value("deviceClasses").toList().count(), resultCount);
    if (resultCount > 0) {
        QString deviceName = supportedDevices.toMap().value("params").toMap().value("deviceClasses").toList().first().toMap().value("name").toString();
        QVERIFY2(deviceName.startsWith(QString("Mock Device")), QString("Got: %1  Expected: %2").arg(deviceName).arg("Mock Device").toLatin1().data());
    }
}

void TestDevices::addConfiguredDevice_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<QVariantList>("deviceParams");
    QTest::addColumn<bool>("success");

    QVariantList deviceParams;
    QVariantMap httpportParam;
    httpportParam.insert("name", "httpport");
    httpportParam.insert("value", m_mockDevice1Port - 1);
    deviceParams.append(httpportParam);

    QTest::newRow("User, JustAdd") << mockDeviceClassId << deviceParams << true;
    QTest::newRow("Auto, JustAdd") << mockDeviceAutoClassId << deviceParams << false;
    QTest::newRow("Discovery, JustAdd") << mockDeviceDiscoveryClassId << deviceParams << false;
    QTest::newRow("User, JustAdd, Async") << mockDeviceAsyncSetupClassId << deviceParams << true;
    QTest::newRow("Invalid DeviceClassId") << DeviceClassId::createDeviceClassId() << deviceParams << false;
    QTest::newRow("Setup failure") << mockDeviceBrokenClassId << deviceParams << false;
    QTest::newRow("Setup failure, Async") << mockDeviceBrokenAsyncSetupClassId << deviceParams << false;

    QVariantList invalidDeviceParams;
    QTest::newRow("User, JustAdd, missing params") << mockDeviceClassId << invalidDeviceParams << false;

    QVariantMap fakeparam;
    fakeparam.insert("name", "tropptth");
    invalidDeviceParams.append(fakeparam);
    QTest::newRow("User, JustAdd, invalid param") << mockDeviceClassId << invalidDeviceParams << false;

    fakeparam.insert("value", "buhuu");
    invalidDeviceParams.clear();
    invalidDeviceParams.append(fakeparam);
    QTest::newRow("User, JustAdd, wrong param") << mockDeviceClassId << invalidDeviceParams << false;

}

void TestDevices::addConfiguredDevice()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(QVariantList, deviceParams);
    QFETCH(bool, success);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    qDebug() << "response is" << response;

    verifySuccess(response, success);

    if (success) {
        QUuid deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
        params.clear();
        params.insert("deviceId", deviceId.toString());
        response = injectAndWait("Devices.RemoveConfiguredDevice", params);
        verifySuccess(response);
    }
}

void TestDevices::getConfiguredDevices()
{
    QVariant response = injectAndWait("Devices.GetConfiguredDevices");

    QVariantList devices = response.toMap().value("params").toMap().value("devices").toList();
    QCOMPARE(devices.count(), 2); // There should be one auto created mock device and the one created in initTestcase()
}

void TestDevices::removeDevice_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<bool>("success");

    QTest::newRow("Existing Device") << m_mockDeviceId << true;
    QTest::newRow("Not existing Device") << DeviceId::createDeviceId() << false;
}

void TestDevices::removeDevice()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(bool, success);

    QSettings settings(m_deviceSettings);
    settings.beginGroup("DeviceConfig");
    if (success) {
        settings.beginGroup(m_mockDeviceId.toString());
        // Make sure we have some config values for this device
        QVERIFY(settings.allKeys().count() > 0);
    }

    QVariantMap params;
    params.insert("deviceId", deviceId);

    QVariant response = injectAndWait("Devices.RemoveConfiguredDevice", params);

    verifySuccess(response, success);

    if (success) {
        // Make sure the device is gone from settings too
        QCOMPARE(settings.allKeys().count(), 0);
    }
}

void TestDevices::storedDevices()
{
    QVariantMap params;
    params.insert("deviceClassId", mockDeviceClassId);
    QVariantList deviceParams;
    QVariantMap httpportParam;
    httpportParam.insert("name", "httpport");
    httpportParam.insert("value", 8888);
    deviceParams.append(httpportParam);
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    verifySuccess(response);
    DeviceId addedDeviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!addedDeviceId.isNull());

    // Restart the core instance to check if settings are loaded at startup
    restartServer();

    response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

    bool found = false;
    foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
        qDebug() << "found stored device" << device;
        if (DeviceId(device.toMap().value("id").toString()) == addedDeviceId) {
//            foreach (const QVariant &paramVariant, device.toMap().value("params").toList()) {
//                if ()
//            }

            qDebug() << "found added device" << device.toMap().value("params").toList().first().toMap();
            qDebug() << "expected deviceParams:" << deviceParams;
            QCOMPARE(device.toMap().value("params").toList(), deviceParams);
            found = true;
            break;
        }
    }
    QVERIFY2(found, "Device missing in config!");

    params.clear();
    params.insert("deviceId", addedDeviceId);
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifySuccess(response);
}

void TestDevices::discoverDevices_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<bool>("success");
    QTest::addColumn<QVariantList>("discoveryParams");

    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("name", "resultCount");
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QTest::newRow("valid deviceClassId") << mockDeviceDiscoveryClassId << 2 << true << QVariantList();
    QTest::newRow("valid deviceClassId with params") << mockDeviceDiscoveryClassId << 1 << true << discoveryParams;
    QTest::newRow("invalid deviceClassId") << DeviceClassId::createDeviceClassId() << 0 << false << QVariantList();
    QTest::newRow("CreateMethodUser deviceClassId") << mockDeviceClassId << 0 << false << QVariantList();
}

void TestDevices::discoverDevices()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);
    QFETCH(bool, success);
    QFETCH(QVariantList, discoveryParams);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifySuccess(response, success);
    if (success) {
        QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), resultCount);
    }

    // If we found something, lets try to add it
    if (success) {
        DeviceDescriptorId descriptorId = DeviceDescriptorId(response.toMap().value("params").toMap().value("deviceDescriptors").toList().first().toMap().value("id").toString());

        params.clear();
        params.insert("deviceClassId", deviceClassId);
        params.insert("deviceDescriptorId", descriptorId.toString());
        response = injectAndWait("Devices.AddConfiguredDevice", params);

        verifySuccess(response);

        DeviceId deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
        params.clear();
        params.insert("deviceId", deviceId.toString());
        response = injectAndWait("Devices.RemoveConfiguredDevice", params);
        verifySuccess(response);
    }
}

void TestDevices::getActionTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid deviceclass") << mockDeviceClassId << 5;
    QTest::newRow("invalid deviceclass") << DeviceClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestDevices::getActionTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    QVariant response = injectAndWait("Devices.GetActionTypes", params);

    QVariantList actionTypes = response.toMap().value("params").toMap().value("actionTypes").toList();
    QCOMPARE(actionTypes.count(), resultCount);
    if (resultCount > 0) {
        QCOMPARE(actionTypes.first().toMap().value("id").toString(), mockActionIdWithParams.toString());
    }
}

void TestDevices::getEventTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid deviceclass") << mockDeviceClassId << 4;
    QTest::newRow("invalid deviceclass") << DeviceClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestDevices::getEventTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    QVariant response = injectAndWait("Devices.GetEventTypes", params);

    QVariantList eventTypes = response.toMap().value("params").toMap().value("eventTypes").toList();
    QCOMPARE(eventTypes.count(), resultCount);
    if (resultCount > 0) {
        QCOMPARE(eventTypes.first().toMap().value("id").toString(), mockEvent1Id.toString());
    }
}

void TestDevices::getStateTypes_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid deviceclass") << mockDeviceClassId << 2;
    QTest::newRow("invalid deviceclass") << DeviceClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestDevices::getStateTypes()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    QVariant response = injectAndWait("Devices.GetStateTypes", params);

    QVariantList stateTypes = response.toMap().value("params").toMap().value("stateTypes").toList();
    QCOMPARE(stateTypes.count(), resultCount);
    if (resultCount > 0) {
        QCOMPARE(stateTypes.first().toMap().value("id").toString(), mockIntStateId.toString());
    }
}

#include "testdevices.moc"

QTEST_MAIN(TestDevices)
