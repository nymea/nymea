/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeatestbase.h"
#include "nymeacore.h"
#include "nymeasettings.h"

#include "integrations/thingdiscoveryinfo.h"
#include "integrations/thingsetupinfo.h"

#include "servers/mocktcpserver.h"
#include "jsonrpc/devicehandler.h"

using namespace nymeaserver;

class TestDevices : public NymeaTestBase
{
    Q_OBJECT

private:
    DeviceId m_mockThingAsyncId;

    inline void verifyDeviceError(const QVariant &response, Device::DeviceError error = Device::DeviceErrorNoError) {
        verifyError(response, "deviceError", enumValueName(error));
    }

private slots:

    void initTestCase();

    void getPlugins();

    void getPluginConfig_data();
    void getPluginConfig();

    void setPluginConfig_data();
    void setPluginConfig();

    void getSupportedVendors();

    void getSupportedDevices_data();
    void getSupportedDevices();

    void verifyInterfaces();

    void addConfiguredDevice_data();
    void addConfiguredDevice();

    void deviceAddedRemovedNotifications();

    void deviceChangedNotifications();

    void getConfiguredDevices();

    void getConfiguredDevice_data();
    void getConfiguredDevice();

    void storedDevices();

    void discoverDevices_data();
    void discoverDevices();

    void addPushButtonDevices_data();
    void addPushButtonDevices();

    void addDisplayPinDevices_data();
    void addDisplayPinDevices();

    void parentChildDevices();

    void getActionTypes_data();
    void getActionTypes();

    void getEventTypes_data();
    void getEventTypes();

    void getStateTypes_data();
    void getStateTypes();

    void getStateType_data();
    void getStateType();

    void getStateValue_data();
    void getStateValue();

    void getStateValues_data();
    void getStateValues();

    void editDevices_data();
    void editDevices();

    void testDeviceSettings();

    void reconfigureDevices_data();
    void reconfigureDevices();

    void reconfigureByDiscovery_data();
    void reconfigureByDiscovery();

    void reconfigureByDiscoveryAndPair();
    void reconfigureAutodevice();

    void testBrowsing_data();
    void testBrowsing();

    void testExecuteBrowserItem_data();
    void testExecuteBrowserItem();

    void testExecuteBrowserItemAction_data();
    void testExecuteBrowserItemAction();

    void executeAction_data();
    void executeAction();

    void triggerEvent();
    void triggerStateChangeEvent();

    void params();

    void asyncSetupEmitsSetupStatusUpdate();

    // Keep those at last as they will remove devices
    void removeDevice_data();
    void removeDevice();

    void removeAutoDevice();

    void discoverDeviceParenting();
};

void TestDevices::initTestCase()
{
    NymeaTestBase::initTestCase();
    QLoggingCategory::setFilterRules("*.debug=false\n"
                                     "Tests.debug=true\n"
                                     "MockDevice.debug=true\n"
                                     );

    // Adding an async mock device to be used in tests below
    QVariantMap params;
    params.insert("deviceClassId", mockThingClassId);
    params.insert("name", "Mock Device (Async)");

    QVariantList deviceParams;

    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", mockThingAsyncParamTypeId);
    asyncParam.insert("value", true);
    deviceParams.append(asyncParam);

    QVariantMap httpParam;
    httpParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpParam.insert("value", 8765);
    deviceParams.append(httpParam);

    params.insert("deviceParams", deviceParams);

    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);

    m_mockThingAsyncId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY2(!m_mockThingAsyncId.isNull(), "Creating an async mock device failed");

    qCDebug(dcTests()) << "Created Async mock device with ID" << m_mockThingAsyncId;
}

void TestDevices::getPlugins()
{
    QVariant response = injectAndWait("Devices.GetPlugins");

    QVariantList plugins = response.toMap().value("params").toMap().value("plugins").toList();

    QCOMPARE(plugins.count() > 0, true);
    bool found = false;
    foreach (const QVariant &listEntry, plugins) {
        if (PluginId(listEntry.toMap().value("id").toString()) == mockPluginId) {
            found = true;
        }
    }
    QCOMPARE(found, true);
}

void TestDevices::getPluginConfig_data()
{
    QTest::addColumn<PluginId>("pluginId");
    QTest::addColumn<Device::DeviceError>("error");

    QTest::newRow("valid plugin") << mockPluginId << Device::DeviceErrorNoError;
    QTest::newRow("invalid plugin") << PluginId::createPluginId() << Device::DeviceErrorPluginNotFound;
}

void TestDevices::getPluginConfig()
{
    QFETCH(PluginId, pluginId);
    QFETCH(Device::DeviceError, error);

    QVariantMap params;
    params.insert("pluginId", pluginId);
    QVariant response = injectAndWait("Devices.GetPluginConfiguration", params);
    verifyDeviceError(response, error);
}

void TestDevices::setPluginConfig_data()
{
    QTest::addColumn<PluginId>("pluginId");
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<Device::DeviceError>("error");

    QTest::newRow("valid") << mockPluginId << QVariant(13) << Device::DeviceErrorNoError;
    QTest::newRow("invalid plugin") << PluginId::createPluginId() << QVariant(13) <<  Device::DeviceErrorPluginNotFound;
    QTest::newRow("too big") << mockPluginId << QVariant(130) << Device::DeviceErrorInvalidParameter;
    QTest::newRow("too small") << mockPluginId << QVariant(-13) << Device::DeviceErrorInvalidParameter;
    QTest::newRow("wrong type") << mockPluginId << QVariant("wrontType") << Device::DeviceErrorInvalidParameter;
}

void TestDevices::setPluginConfig()
{
    QFETCH(PluginId, pluginId);
    QFETCH(QVariant, value);
    QFETCH(Device::DeviceError, error);

    QVariantMap params;
    params.insert("pluginId", pluginId);

    QVariantList configuration;
    QVariantMap configParam;
    configParam.insert("paramTypeId", mockPluginConfigParamIntParamTypeId);
    configParam.insert("value", value);
    configuration.append(configParam);
    params.insert("configuration", configuration);
    QVariant response = injectAndWait("Devices.SetPluginConfiguration", params);
    verifyDeviceError(response, error);

    if (error == Device::DeviceErrorNoError) {
        params.clear();
        params.insert("pluginId", pluginId);
        response = injectAndWait("Devices.GetPluginConfiguration", params);
        verifyDeviceError(response);
        qDebug() << value << response.toMap().value("params").toMap().value("configuration").toList().first();
        QVERIFY2(ParamTypeId(response.toMap().value("params").toMap().value("configuration").toList().first().toMap().value("paramTypeId").toString()) == mockPluginConfigParamIntParamTypeId, "Value not set correctly");
        QVERIFY2(response.toMap().value("params").toMap().value("configuration").toList().first().toMap().value("value") == value, "Value not set correctly");
    }
}

void TestDevices::getSupportedVendors()
{
    QVariant supportedVendors = injectAndWait("Devices.GetSupportedVendors");
    qDebug() << "response" << supportedVendors;

    // Make sure there is exactly 1 supported Vendor named "guh"
    QVariantList vendorList = supportedVendors.toMap().value("params").toMap().value("vendors").toList();
    QCOMPARE(vendorList.count() > 0, true);
    bool found = false;
    foreach (const QVariant &listEntry, vendorList) {
        if (VendorId(listEntry.toMap().value("id").toString()) == nymeaVendorId) {
            found = true;
        }
    }
    QCOMPARE(found, true);
}

void TestDevices::getSupportedDevices_data()
{
    QTest::addColumn<VendorId>("vendorId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("vendor guh") << nymeaVendorId << 1;
    QTest::newRow("no filter") << VendorId() << 1;
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
    QVariant result = injectAndWait("Devices.GetSupportedDevices", params);

    QVariantList supportedDevices = result.toMap().value("params").toMap().value("deviceClasses").toList();
    // Make sure there are the right amount of supported device classes with the name Mock Device
    QCOMPARE(supportedDevices.count() >= resultCount, true);
}

void TestDevices::verifyInterfaces()
{
    QVariantMap params;
    params.insert("vendorId", nymeaVendorId);
    QVariant result = injectAndWait("Devices.GetSupportedDevices", params);
    QVariantList supportedDevices = result.toMap().value("params").toMap().value("deviceClasses").toList();

    QVariantMap mockDevice;
    foreach (const QVariant &deviceClass, supportedDevices) {
        if (deviceClass.toMap().value("id").toUuid() == mockThingClassId) {
            mockDevice = deviceClass.toMap();
        }
    }
    QVERIFY(!mockDevice.isEmpty());

    QVariantList interfaces = mockDevice.value("interfaces").toList();
    // Must contain system, power, light and battery, but must not contain gateway as the device manager should filter
    // that away because it doesn't implement all the required states.
    QCOMPARE(interfaces.count(), 4);
    QVERIFY(interfaces.contains("system"));
    QVERIFY(interfaces.contains("battery"));
    QVERIFY(interfaces.contains("power"));
    QVERIFY(interfaces.contains("light"));
    QVERIFY(!interfaces.contains("gateway"));
}

void TestDevices::addConfiguredDevice_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<QVariantList>("deviceParams");
    QTest::addColumn<bool>("jsonValidation");
    QTest::addColumn<Device::DeviceError>("deviceError");

    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId.toString());
    httpportParam.insert("value", m_mockThing1Port - 1);
    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", mockThingAsyncParamTypeId);
    asyncParam.insert("value", true);
    QVariantMap brokenParam;
    brokenParam.insert("paramTypeId", mockThingBrokenParamTypeId);
    brokenParam.insert("value", true);

    QVariantList deviceParams;

    deviceParams.clear(); deviceParams << httpportParam;
    QTest::newRow("User, JustAdd") << mockThingClassId << deviceParams << true << Device::DeviceErrorNoError;
    deviceParams.clear(); deviceParams << httpportParam << asyncParam;
    QTest::newRow("User, JustAdd, Async") << mockThingClassId << deviceParams << true << Device::DeviceErrorNoError;
    QTest::newRow("Invalid ThingClassId") << ThingClassId::createThingClassId() << deviceParams << true << Device::DeviceErrorDeviceClassNotFound;
    deviceParams.clear(); deviceParams << httpportParam << brokenParam;
    QTest::newRow("Setup failure") << mockThingClassId << deviceParams << true << Device::DeviceErrorSetupFailed;
    deviceParams.clear(); deviceParams << httpportParam << asyncParam << brokenParam;
    QTest::newRow("Setup failure, Async") << mockThingClassId << deviceParams << true << Device::DeviceErrorSetupFailed;

    QVariantList invalidDeviceParams;
    QTest::newRow("User, JustAdd, missing params") << mockThingClassId << invalidDeviceParams << true << Device::DeviceErrorMissingParameter;

    QVariantMap fakeparam;
    fakeparam.insert("paramTypeId", ParamTypeId::createParamTypeId());
    invalidDeviceParams.append(fakeparam);
    QTest::newRow("User, JustAdd, invalid param") << mockThingClassId << invalidDeviceParams << false << Device::DeviceErrorMissingParameter;

    QVariantMap fakeparam2;
    fakeparam2.insert("paramTypeId", mockThingHttpportParamTypeId.toString());
    fakeparam2.insert("value", "blabla");
    invalidDeviceParams.clear();
    invalidDeviceParams.append(fakeparam2);
    QTest::newRow("User, JustAdd, wrong param") << mockThingClassId << invalidDeviceParams << true << Device::DeviceErrorInvalidParameter;

    deviceParams.clear(); deviceParams << httpportParam << fakeparam;
    QTest::newRow("User, JustAdd, additional invalid param") << mockThingClassId << deviceParams << false << Device::DeviceErrorInvalidParameter;

    deviceParams.clear(); deviceParams << httpportParam << fakeparam2;
    QTest::newRow("User, JustAdd, duplicate param") << mockThingClassId << deviceParams << true << Device::DeviceErrorInvalidParameter;

}

void TestDevices::addConfiguredDevice()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(QVariantList, deviceParams);
    QFETCH(bool, jsonValidation);
    QFETCH(Device::DeviceError, deviceError);

    QVariantMap params;
    params.insert("deviceClassId", thingClassId);
    params.insert("name", "Test Add Device");
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);

    if (!jsonValidation) {
        QCOMPARE(response.toMap().value("status").toString(), QString("error"));
        return;
    }
    verifyDeviceError(response, deviceError);

    if (deviceError == Device::DeviceErrorNoError) {
        QUuid deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
        params.clear();
        params.insert("deviceId", deviceId.toString());
        response = injectAndWait("Devices.RemoveConfiguredDevice", params);
        verifyDeviceError(response);
    }
}

void TestDevices::deviceAddedRemovedNotifications()
{
    enableNotifications({"Devices"});

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // add device and wait for notification
    QVariantList deviceParams;
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParam.insert("value", 5678);
    deviceParams.append(httpportParam);

    QVariantMap params; clientSpy.clear();
    params.insert("deviceClassId", mockThingClassId);
    params.insert("name", "Mock device");
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    if (clientSpy.count() == 0) clientSpy.wait();
    verifyDeviceError(response);
    QVariantMap notificationDeviceMap = checkNotification(clientSpy, "Devices.DeviceAdded").toMap().value("params").toMap().value("device").toMap();

    ThingId deviceId = ThingId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!deviceId.isNull());

    // check the DeviceAdded notification
    QCOMPARE(notificationDeviceMap.value("deviceClassId").toUuid(), QUuid(mockThingClassId));
    QCOMPARE(notificationDeviceMap.value("id").toUuid(), QUuid(deviceId));
    foreach (const QVariant &param, notificationDeviceMap.value("params").toList()) {
        if (param.toMap().value("name").toString() == "httpport") {
            QCOMPARE(param.toMap().value("value").toInt(), httpportParam.value("value").toInt());
        }
    }

    // now remove the device and check the device removed notification
    params.clear(); response.clear(); clientSpy.clear();
    params.insert("deviceId", deviceId);
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    if (clientSpy.count() == 0) clientSpy.wait();
    verifyDeviceError(response);
    checkNotification(clientSpy, "Devices.DeviceRemoved");

    QCOMPARE(disableNotifications(), true);
}

void TestDevices::deviceChangedNotifications()
{
    enableNotifications({"Devices"});

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // ADD
    // add device and wait for notification
    QVariantList deviceParams;
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParam.insert("value", 23234);
    deviceParams.append(httpportParam);

    clientSpy.clear();
    QVariantMap params;
    params.insert("deviceClassId", mockThingClassId);
    params.insert("name", "Mock");
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    ThingId deviceId = ThingId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!deviceId.isNull());
    if (clientSpy.count() == 0) clientSpy.wait();
    verifyDeviceError(response);
    QVariantMap notificationDeviceMap = checkNotification(clientSpy, "Devices.DeviceAdded").toMap().value("params").toMap().value("device").toMap();

    QCOMPARE(notificationDeviceMap.value("deviceClassId").toUuid(), QUuid(mockThingClassId));
    QCOMPARE(notificationDeviceMap.value("id").toUuid(), QUuid(deviceId));
    foreach (const QVariant &param, notificationDeviceMap.value("params").toList()) {
        if (param.toMap().value("name").toString() == "httpport") {
            QCOMPARE(param.toMap().value("value").toInt(), httpportParam.value("value").toInt());
        }
    }

    // RECONFIGURE
    // now reconfigure the device and check the deviceChanged notification
    QVariantList newDeviceParams;
    QVariantMap newHttpportParam;
    newHttpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    newHttpportParam.insert("value", 45473);
    newDeviceParams.append(newHttpportParam);

    params.clear(); response.clear(); clientSpy.clear();
    params.insert("deviceId", deviceId);
    params.insert("deviceParams", newDeviceParams);
    response = injectAndWait("Devices.ReconfigureDevice", params);
    if (clientSpy.count() == 0) clientSpy.wait();
    verifyDeviceError(response);
    QVariantMap reconfigureDeviceNotificationMap = checkNotification(clientSpy, "Devices.DeviceChanged").toMap().value("params").toMap().value("device").toMap();
    QCOMPARE(reconfigureDeviceNotificationMap.value("deviceClassId").toUuid(), QUuid(mockThingClassId));
    QCOMPARE(reconfigureDeviceNotificationMap.value("id").toUuid(), QUuid(deviceId));
    foreach (const QVariant &param, reconfigureDeviceNotificationMap.value("params").toList()) {
        if (param.toMap().value("name").toString() == "httpport") {
            QCOMPARE(param.toMap().value("value").toInt(), newHttpportParam.value("value").toInt());
        }
    }

    // EDIT device name
    QString deviceName = "Test device 1234";
    params.clear(); response.clear(); clientSpy.clear();
    params.insert("deviceId", deviceId);
    params.insert("name", deviceName);
    response = injectAndWait("Devices.EditDevice", params);
    if (clientSpy.count() == 0) clientSpy.wait();
    verifyDeviceError(response);
    QVariantMap editDeviceNotificationMap = checkNotification(clientSpy, "Devices.DeviceChanged").toMap().value("params").toMap().value("device").toMap();
    QCOMPARE(editDeviceNotificationMap.value("deviceClassId").toUuid(), QUuid(mockThingClassId));
    QCOMPARE(editDeviceNotificationMap.value("id").toUuid(), QUuid(deviceId));
    QCOMPARE(editDeviceNotificationMap.value("name").toString(), deviceName);

    // REMOVE
    // now remove the device and check the device removed notification
    params.clear(); response.clear(); clientSpy.clear();
    params.insert("deviceId", deviceId);
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    if (clientSpy.count() == 0) clientSpy.wait();
    verifyDeviceError(response);
    checkNotification(clientSpy, "Devices.DeviceRemoved");
    checkNotification(clientSpy, "Logging.LogDatabaseUpdated");
}

void TestDevices::getConfiguredDevices()
{
    QVariant response = injectAndWait("Devices.GetConfiguredDevices");

    QVariantList devices = response.toMap().value("params").toMap().value("devices").toList();
    QCOMPARE(devices.count(), 3); // There should be: one auto created mock device, one created in NymeaTestBase::initTestcase() and one created in TestDevices::initTestCase()
}

void TestDevices::getConfiguredDevice_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<Device::DeviceError>("expectedError");

    QTest::newRow("valid deviceId") << DeviceId(m_mockThingId) << Device::DeviceErrorNoError;
    QTest::newRow("invalid deviceId") << DeviceId::createDeviceId() << Device::DeviceErrorDeviceNotFound;
}

void TestDevices::getConfiguredDevice()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(Device::DeviceError, expectedError);

    QVariantMap params;
    params.insert("deviceId", deviceId);
    QVariant response = injectAndWait("Devices.GetConfiguredDevices", params);

//    qCDebug(dcTests()) << qUtf8Printable(QJsonDocument::fromVariant(response).toJson());

    if (expectedError == Device::DeviceErrorNoError) {
        QVariantList devices = response.toMap().value("params").toMap().value("devices").toList();
        QCOMPARE(devices.count(), 1);
    }
}

void TestDevices::storedDevices()
{
    QVariantMap params;
    params.insert("deviceClassId", mockThingClassId);
    params.insert("name", "Test stored Device");
    QVariantList deviceParams;
    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", mockThingAsyncParamTypeId);
    asyncParam.insert("value", false);
    deviceParams.append(asyncParam);
    QVariantMap brokenParam;
    brokenParam.insert("paramTypeId", mockThingBrokenParamTypeId);
    brokenParam.insert("value", false);
    deviceParams.append(brokenParam);
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParam.insert("value", 8889);
    deviceParams.append(httpportParam);
    params.insert("deviceParams", deviceParams);

    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    verifyDeviceError(response);
    DeviceId addedDeviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!addedDeviceId.isNull());

    // Restart the core instance to check if settings are loaded at startup
    restartServer();

    response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

    bool found = false;
    foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
        if (DeviceId(device.toMap().value("id").toString()) == addedDeviceId) {
            qDebug() << "found added device" << device.toMap().value("params");
            qDebug() << "expected deviceParams:" << deviceParams;
            verifyParams(deviceParams, device.toMap().value("params").toList());
            found = true;
            break;
        }
    }
    QVERIFY2(found, "Device missing in config!");

    params.clear();
    params.insert("deviceId", addedDeviceId);
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response);
}

void TestDevices::discoverDevices_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<Device::DeviceError>("error");
    QTest::addColumn<QVariantList>("discoveryParams");

    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", mockDiscoveryResultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QTest::newRow("valid ThingClassId") << mockThingClassId << 2 << Device::DeviceErrorNoError << QVariantList();
    QTest::newRow("valid ThingClassId with params") << mockThingClassId << 1 << Device::DeviceErrorNoError << discoveryParams;
    QTest::newRow("invalid ThingClassId") << ThingClassId::createThingClassId() << 0 << Device::DeviceErrorDeviceClassNotFound << QVariantList();
}

void TestDevices::discoverDevices()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(int, resultCount);
    QFETCH(Device::DeviceError, error);
    QFETCH(QVariantList, discoveryParams);

    QVariantMap params;
    params.insert("deviceClassId", thingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response, error);
    if (error == Device::DeviceErrorNoError) {
        QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), resultCount);
    }

    // If we found something, lets try to add it
    if (error == Device::DeviceErrorNoError) {
        ThingDescriptorId descriptorId = ThingDescriptorId(response.toMap().value("params").toMap().value("deviceDescriptors").toList().first().toMap().value("id").toString());

        params.clear();
        params.insert("deviceClassId", thingClassId);
        params.insert("name", "Discoverd mock device");
        params.insert("deviceDescriptorId", descriptorId.toString());
        response = injectAndWait("Devices.AddConfiguredDevice", params);

        verifyDeviceError(response);

        DeviceId deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
        params.clear();
        params.insert("deviceId", deviceId.toString());
        response = injectAndWait("Devices.RemoveConfiguredDevice", params);
        verifyDeviceError(response);
    }
}

void TestDevices::addPushButtonDevices_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<Device::DeviceError>("error");
    QTest::addColumn<bool>("waitForButtonPressed");

    QTest::newRow("Valid: Add PushButton device") << pushButtonMockThingClassId << Device::DeviceErrorNoError << true;
    QTest::newRow("Invalid: Add PushButton device (press to early)") << pushButtonMockThingClassId << Device::DeviceErrorAuthenticationFailure << false;
}

void TestDevices::addPushButtonDevices()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(Device::DeviceError, error);
    QFETCH(bool, waitForButtonPressed);

    // Discover device
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", pushButtonMockDiscoveryResultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QVariantMap params;
    params.insert("deviceClassId", thingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response, Device::DeviceErrorNoError);
    QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), 1);


    // Pair device
    ThingDescriptorId descriptorId = ThingDescriptorId(response.toMap().value("params").toMap().value("deviceDescriptors").toList().first().toMap().value("id").toString());
    params.clear();
    params.insert("deviceClassId", thingClassId);
    params.insert("name", "Pushbutton device");
    params.insert("deviceDescriptorId", descriptorId.toString());
    response = injectAndWait("Devices.PairDevice", params);

    verifyDeviceError(response);

    PairingTransactionId pairingTransactionId(response.toMap().value("params").toMap().value("pairingTransactionId").toString());
    QString displayMessage = response.toMap().value("params").toMap().value("displayMessage").toString();

    qDebug() << "displayMessage" << displayMessage;

    if (waitForButtonPressed)
        QTest::qWait(3500);

    // Confirm pairing
    params.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());
    response = injectAndWait("Devices.ConfirmPairing", params);

    verifyDeviceError(response, error);

    if (error == Device::DeviceErrorNoError) {
        DeviceId deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
        params.clear();
        params.insert("deviceId", deviceId.toString());
        response = injectAndWait("Devices.RemoveConfiguredDevice", params);
        verifyDeviceError(response);
    }
}

void TestDevices::addDisplayPinDevices_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<Device::DeviceError>("error");
    QTest::addColumn<QString>("secret");

    QTest::newRow("Valid: Add DisplayPin device") << displayPinMockThingClassId << Device::DeviceErrorNoError << "243681";
    QTest::newRow("Invalid: Add DisplayPin device (wrong pin)") << displayPinMockThingClassId << Device::DeviceErrorAuthenticationFailure << "243682";
}

void TestDevices::addDisplayPinDevices()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(Device::DeviceError, error);
    QFETCH(QString, secret);

    // Discover device
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", displayPinMockDiscoveryResultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QVariantMap params;
    params.insert("deviceClassId", thingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response, Device::DeviceErrorNoError);
    QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), 1);

    // Pair device
    ThingDescriptorId descriptorId = ThingDescriptorId(response.toMap().value("params").toMap().value("deviceDescriptors").toList().first().toMap().value("id").toString());
    params.clear();
    params.insert("deviceClassId", thingClassId);
    params.insert("name", "Display pin mock device");
    params.insert("deviceDescriptorId", descriptorId.toString());
    response = injectAndWait("Devices.PairDevice", params);

    verifyDeviceError(response);

    PairingTransactionId pairingTransactionId(response.toMap().value("params").toMap().value("pairingTransactionId").toString());
    QString displayMessage = response.toMap().value("params").toMap().value("displayMessage").toString();

    qDebug() << "displayMessage" << displayMessage;

    params.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());
    params.insert("secret", secret);
    response = injectAndWait("Devices.ConfirmPairing", params);

    verifyDeviceError(response, error);

    if (error == Device::DeviceErrorNoError) {
        DeviceId deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
        params.clear();
        params.insert("deviceId", deviceId.toString());
        response = injectAndWait("Devices.RemoveConfiguredDevice", params);
        verifyDeviceError(response);
    }


}

void TestDevices::parentChildDevices()
{
    // add parent device
    QVariantMap params;
    params.insert("deviceClassId", parentMockThingClassId);
    params.insert("name", "Parent device");

    QSignalSpy deviceAddedSpy(NymeaCore::instance()->thingManager(), &ThingManager::thingAdded);

    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    verifyDeviceError(response);

    DeviceId parentId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!parentId.isNull());

    deviceAddedSpy.wait();
    QCOMPARE(deviceAddedSpy.count(), 2);

    // find child device
    response = injectAndWait("Devices.GetConfiguredDevices");

    QVariantList devices = response.toMap().value("params").toMap().value("devices").toList();

    DeviceId childDeviceId;
    foreach (const QVariant deviceVariant, devices) {
        QVariantMap deviceMap = deviceVariant.toMap();

        if (deviceMap.value("deviceClassId").toUuid() == childMockThingClassId) {
            if (deviceMap.value("parentId").toUuid() == parentId) {
                childDeviceId = DeviceId(deviceMap.value("id").toString());
                break;
            }
        }
    }
    QVERIFY2(!childDeviceId.isNull(), QString("Could not find child device:\nParent ID:%1\nResponse:%2")
             .arg(parentId.toString())
             .arg(qUtf8Printable(QJsonDocument::fromVariant(response).toJson()))
             .toUtf8());

    // Try to remove the child device
    params.clear();
    params.insert("deviceId", childDeviceId.toString());
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response, Device::DeviceErrorDeviceIsChild);

    // check if the child device is still there
    response = injectAndWait("Devices.GetConfiguredDevices");
    devices = response.toMap().value("params").toMap().value("devices").toList();
    bool found = false;
    foreach (const QVariant deviceVariant, devices) {
        QVariantMap deviceMap = deviceVariant.toMap();
        if (deviceMap.value("deviceClassId").toUuid() == childMockThingClassId) {
            if (deviceMap.value("id").toUuid() == childDeviceId) {
                found = true;
                break;
            }
        }
    }
    QVERIFY2(found, "Could not find child device.");

    // remove the parent device
    params.clear();
    params.insert("deviceId", parentId.toString());
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response);

    // check if the child device is still there
    response = injectAndWait("Devices.GetConfiguredDevices");
    devices = response.toMap().value("params").toMap().value("devices").toList();
    found = false;
    foreach (const QVariant deviceVariant, devices) {
        QVariantMap deviceMap = deviceVariant.toMap();
        if (deviceMap.value("deviceClassId").toString() == childMockThingClassId.toString()) {
            if (deviceMap.value("id") == childDeviceId.toString()) {
                found = true;
                break;
            }
        }
    }
    QVERIFY2(!found, "Could not find child device.");
}

void TestDevices::getActionTypes_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<QList<ActionTypeId> >("actionTypeTestData");

    QTest::newRow("valid deviceclass") << mockThingClassId
                                       << (QList<ActionTypeId>() << mockAsyncActionTypeId << mockAsyncFailingActionTypeId << mockFailingActionTypeId << mockWithoutParamsActionTypeId << mockPowerActionTypeId << mockWithoutParamsActionTypeId);
    QTest::newRow("invalid deviceclass") << ThingClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << QList<ActionTypeId>();
}

void TestDevices::getActionTypes()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(QList<ActionTypeId>, actionTypeTestData);

    QVariantMap params;
    params.insert("deviceClassId", thingClassId);
    QVariant response = injectAndWait("Devices.GetActionTypes", params);

    QVariantList actionTypes = response.toMap().value("params").toMap().value("actionTypes").toList();
    QCOMPARE(actionTypes.count(), actionTypeTestData.count());
    foreach (const ActionTypeId &testDataId, actionTypeTestData) {
        bool found = false;
        foreach (const QVariant &at, actionTypes) {
            if (testDataId == at.toMap().value("id").toUuid()) {
                found = true;
                break;
            }
        }
        QVERIFY(found);
    }
}

void TestDevices::getEventTypes_data()
{
    QTest::addColumn<ThingClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid deviceclass") << mockThingClassId << 8;
    QTest::newRow("invalid deviceclass") << ThingClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestDevices::getEventTypes()
{
    QFETCH(ThingClassId, deviceClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    QVariant response = injectAndWait("Devices.GetEventTypes", params);

    qDebug() << response;

    QVariantList eventTypes = response.toMap().value("params").toMap().value("eventTypes").toList();
    QCOMPARE(eventTypes.count(), resultCount);

}

void TestDevices::getStateTypes_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid deviceclass") << mockThingClassId << 6;
    QTest::newRow("invalid deviceclass") << ThingClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestDevices::getStateTypes()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("deviceClassId", thingClassId);
    QVariant response = injectAndWait("Devices.GetStateTypes", params);

    QVariantList stateTypes = response.toMap().value("params").toMap().value("stateTypes").toList();
    QCOMPARE(stateTypes.count(), resultCount);
    if (resultCount > 0) {
        QCOMPARE(stateTypes.first().toMap().value("id").toUuid().toString(), mockIntStateTypeId.toString());
    }
}

void TestDevices::getStateType_data()
{
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<Device::DeviceError>("error");

    QTest::newRow("valid int state") << mockIntStateTypeId << Device::DeviceErrorNoError;
    QTest::newRow("valid bool state") << mockBoolStateTypeId << Device::DeviceErrorNoError;
    QTest::newRow("invalid stateTypeId") << StateTypeId::createStateTypeId() << Device::DeviceErrorStateTypeNotFound;
}

void TestDevices::getStateType()
{
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(Device::DeviceError, error);

    QVariantMap params;
    params.insert("stateTypeId", stateTypeId);
    QVariant response = injectAndWait("States.GetStateType", params);
    verifyDeviceError(response, error);

    if (error != Device::DeviceErrorNoError)
        return;

    QVariantMap stateType = response.toMap().value("params").toMap().value("stateType").toMap();

    QVERIFY2(!stateType.isEmpty(), "Got no stateType");
    QCOMPARE(stateType.value("id").toUuid().toString(), stateTypeId.toString());
}

void TestDevices::getStateValue_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<Device::DeviceError>("statusCode");

    QTest::newRow("valid deviceId") << DeviceId(m_mockThingId) << mockIntStateTypeId << Device::DeviceErrorNoError;
    QTest::newRow("invalid deviceId") << DeviceId("094f8024-5caa-48c1-ab6a-de486a92088f") << mockIntStateTypeId << Device::DeviceErrorDeviceNotFound;
    QTest::newRow("invalid statetypeId") << DeviceId(m_mockThingId) << StateTypeId("120514f1-343e-4621-9bff-dac616169df9") << Device::DeviceErrorStateTypeNotFound;
}

void TestDevices::getStateValue()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(Device::DeviceError, statusCode);

    QVariantMap params;
    params.insert("deviceId", deviceId);
    params.insert("stateTypeId", stateTypeId);
    QVariant response = injectAndWait("Devices.GetStateValue", params);

    QCOMPARE(response.toMap().value("params").toMap().value("deviceError").toString(), enumValueName(statusCode));
    if (statusCode == Device::DeviceErrorNoError) {
        QVariant value = response.toMap().value("params").toMap().value("value");
        QCOMPARE(value.toInt(), 10); // Mock device has value 10 by default...
    }
}

void TestDevices::getStateValues_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<Device::DeviceError>("statusCode");

    QTest::newRow("valid deviceId") << DeviceId(m_mockThingId) << Device::DeviceErrorNoError;
    QTest::newRow("invalid deviceId") << DeviceId("094f8024-5caa-48c1-ab6a-de486a92088f") << Device::DeviceErrorDeviceNotFound;
}

void TestDevices::getStateValues()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(Device::DeviceError, statusCode);

    QVariantMap params;
    params.insert("deviceId", deviceId);
    QVariant response = injectAndWait("Devices.GetStateValues", params);

    QCOMPARE(response.toMap().value("params").toMap().value("deviceError").toString(), enumValueName(statusCode));
    if (statusCode == Device::DeviceErrorNoError) {
        QVariantList values = response.toMap().value("params").toMap().value("values").toList();
        QCOMPARE(values.count(), 6); // Mock device has 6 states...
    }
}

void TestDevices::editDevices_data()
{
    QTest::addColumn<QString>("name");

    QTest::newRow("change name") << "New device name";
    QTest::newRow("change name") << "Foo device";
    QTest::newRow("change name") << "Bar device";
}

void TestDevices::editDevices()
{
    QFETCH(QString, name);

    QString originalName = "Test device";

    // add device
    QVariantList deviceParams;
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParam.insert("value", 8889);
    deviceParams.append(httpportParam);

    QVariantMap params;
    params.insert("deviceClassId", mockThingClassId);
    params.insert("name", originalName);
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    verifyDeviceError(response);
    DeviceId deviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());

    // edit device
    params.clear();
    params.insert("deviceId", deviceId);
    params.insert("name", name);

    response = injectAndWait("Devices.EditDevice", params);
    verifyDeviceError(response);

    // verify changed
    QString newName;
    response = injectAndWait("Devices.GetConfiguredDevices");
    QVariantList devices = response.toMap().value("params").toMap().value("devices").toList();

    foreach (const QVariant &deviceVariant, devices) {
        QVariantMap device = deviceVariant.toMap();
        if (DeviceId(device.value("id").toString()) == deviceId) {
            newName = device.value("name").toString();
        }
    }
    QCOMPARE(newName, name);

    restartServer();

    // check if the changed name is still there after loading
    response = injectAndWait("Devices.GetConfiguredDevices");
    devices = response.toMap().value("params").toMap().value("devices").toList();
    foreach (const QVariant &deviceVariant, devices) {
        QVariantMap device = deviceVariant.toMap();
        if (DeviceId(device.value("id").toString()) == deviceId) {
            newName = device.value("name").toString();
            break;
        }
    }
    QCOMPARE(newName, name);

    params.clear();
    params.insert("deviceId", deviceId.toString());
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response);
}

void TestDevices::testDeviceSettings()
{
    // add device
    QVariantList deviceParams;
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParam.insert("value", 8889);
    deviceParams.append(httpportParam);

    QVariantMap params;
    params.insert("deviceClassId", mockThingClassId);
    params.insert("name", "Mock");
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    verifyDeviceError(response);
    DeviceId deviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());

    // check if default settings are loaded
    params.clear();
    params.insert("deviceId", deviceId);
    response = injectAndWait("Devices.GetConfiguredDevices", params);
    QVariantList devices = response.toMap().value("params").toMap().value("devices").toList();
    QVERIFY2(devices.count() == 1, "Error creating device");

    QVariantMap device = devices.first().toMap();
    QVERIFY2(DeviceId(device.value("id").toString()) == deviceId, "DeviceId not matching");

    QVariantList settings = device.value("settings").toList();
    QCOMPARE(settings.count(), 1);

    QCOMPARE(settings.first().toMap().value("paramTypeId").toUuid(), QUuid(mockSettingsSetting1ParamTypeId));
    QVERIFY2(settings.first().toMap().value("value").toInt() == 5, "Setting 1 default value not matching");

    // change a setting
    params.clear();
    params.insert("deviceId", deviceId);
    settings.clear();
    QVariantMap setting;
    setting.insert("paramTypeId", mockSettingsSetting1ParamTypeId);
    setting.insert("value", 7);
    settings.append(setting);
    params.insert("settings", settings);
    response = injectAndWait("Devices.SetDeviceSettings", params);

    // Check if the change happened
    params.clear();
    params.insert("deviceId", deviceId);
    response = injectAndWait("Devices.GetConfiguredDevices", params);
    devices = response.toMap().value("params").toMap().value("devices").toList();
    QVERIFY2(devices.count() == 1, "Error creating device");

    device = devices.first().toMap();
    QVERIFY2(DeviceId(device.value("id").toString()) == deviceId, "DeviceId not matching");

    settings = device.value("settings").toList();
    QCOMPARE(settings.count(), 1);

    QCOMPARE(settings.first().toMap().value("paramTypeId").toUuid(), QUuid(mockSettingsSetting1ParamTypeId));
    QVERIFY2(settings.first().toMap().value("value").toInt() == 7, "Setting 1 changed value not matching");

    restartServer();

    // Check if the change persisted
    params.clear();
    params.insert("deviceId", deviceId);
    response = injectAndWait("Devices.GetConfiguredDevices", params);
    devices = response.toMap().value("params").toMap().value("devices").toList();
    QVERIFY2(devices.count() == 1, "Error creating device");

    device = devices.first().toMap();
    QVERIFY2(DeviceId(device.value("id").toString()) == deviceId, "DeviceId not matching");

    settings = device.value("settings").toList();
    QCOMPARE(settings.count(), 1);

    QCOMPARE(settings.first().toMap().value("paramTypeId").toUuid(), QUuid(mockSettingsSetting1ParamTypeId));
    QVERIFY2(settings.first().toMap().value("value").toInt() == 7, "Setting 1 changed value not persisting restart");

}

void TestDevices::reconfigureDevices_data()
{
    QVariantList asyncChangeDeviceParams;
    QVariantMap asyncParamDifferent;
    asyncParamDifferent.insert("paramTypeId", mockThingAsyncParamTypeId);
    asyncParamDifferent.insert("value", true);
    asyncChangeDeviceParams.append(asyncParamDifferent);

    QVariantList httpportChangeDeviceParams;
    QVariantMap httpportParamDifferent;
    httpportParamDifferent.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParamDifferent.insert("value", 8893); // if change -> change also newPort in reconfigureDevices()
    httpportChangeDeviceParams.append(httpportParamDifferent);

    QVariantList brokenChangedDeviceParams;
    QVariantMap brokenParamDifferent;
    brokenParamDifferent.insert("paramTypeId", mockThingBrokenParamTypeId);
    brokenParamDifferent.insert("value", true);
    brokenChangedDeviceParams.append(brokenParamDifferent);

    QVariantList asyncAndPortChangeDeviceParams;
    asyncAndPortChangeDeviceParams.append(asyncParamDifferent);
    asyncAndPortChangeDeviceParams.append(httpportParamDifferent);


    QVariantList changeAllWritableDeviceParams;
    changeAllWritableDeviceParams.append(asyncParamDifferent);
    changeAllWritableDeviceParams.append(httpportParamDifferent);

    QTest::addColumn<bool>("broken");
    QTest::addColumn<QVariantList>("newDeviceParams");
    QTest::addColumn<Device::DeviceError>("deviceError");

    QTest::newRow("valid - change async param") << false << asyncChangeDeviceParams << Device::DeviceErrorParameterNotWritable;
    QTest::newRow("valid - change httpport param") << false <<  httpportChangeDeviceParams << Device::DeviceErrorNoError;
    QTest::newRow("invalid - change httpport and async param") << false << asyncAndPortChangeDeviceParams << Device::DeviceErrorParameterNotWritable;
    QTest::newRow("invalid - change all params (except broken)") << false << changeAllWritableDeviceParams << Device::DeviceErrorParameterNotWritable;
}

void TestDevices::reconfigureDevices()
{
    QFETCH(bool, broken);
    QFETCH(QVariantList, newDeviceParams);
    QFETCH(Device::DeviceError, deviceError);

    // add device
    QVariantMap params;
    params.insert("deviceClassId", mockThingClassId);
    params.insert("name", "Device to edit");
    QVariantList deviceParams;
    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", mockThingAsyncParamTypeId);
    asyncParam.insert("value", false);
    deviceParams.append(asyncParam);
    QVariantMap brokenParam;
    brokenParam.insert("paramTypeId", mockThingBrokenParamTypeId);
    brokenParam.insert("value", broken);
    deviceParams.append(brokenParam);
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParam.insert("value", 8892);
    deviceParams.append(httpportParam);
    params.insert("deviceParams", deviceParams);

    // add a mockdevice
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    verifyDeviceError(response);

    DeviceId deviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!deviceId.isNull());

    // now EDIT the added device
    response.clear();
    QVariantMap editParams;
    editParams.insert("deviceId", deviceId);
    editParams.insert("deviceParams", newDeviceParams);
    response = injectAndWait("Devices.ReconfigureDevice", editParams);
    verifyDeviceError(response, deviceError);

    // if the edit should have been successful
    if (deviceError == Device::DeviceErrorNoError) {
        response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

        bool found = false;
        foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
            if (DeviceId(device.toMap().value("id").toString()) == deviceId) {
                qDebug() << "found added device" << device.toMap().value("params");
                qDebug() << "expected deviceParams:" << newDeviceParams;
                // check if the edit was ok
                verifyParams(newDeviceParams, device.toMap().value("params").toList(), false);
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Device missing in config!");

        // Restart the core instance to check if settings are loaded at startup
        restartServer();

        response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

        found = false;
        foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
            if (DeviceId(device.toMap().value("id").toString()) == deviceId) {
                qDebug() << "found added device" << device.toMap().value("params");
                qDebug() << "expected deviceParams:" << newDeviceParams;
                // check if the edit was ok
                verifyParams(newDeviceParams, device.toMap().value("params").toList(), false);
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Device missing in config!");

        // delete it
        params.clear();
        params.insert("deviceId", deviceId);
        response.clear();
        response = injectAndWait("Devices.RemoveConfiguredDevice", params);
        verifyDeviceError(response);
        return;
    } else {
        // The edit was not ok, check if the old params are still there
        response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

        bool found = false;
        foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
            if (DeviceId(device.toMap().value("id").toString()) == deviceId) {
                qDebug() << "found added device" << device.toMap().value("params");
                qDebug() << "expected deviceParams:" << newDeviceParams;
                // check if the params are unchanged
                verifyParams(deviceParams, device.toMap().value("params").toList());
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Device missing in config!");

        // Restart the core instance to check if settings are loaded at startup
        restartServer();

        response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

        found = false;
        foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
            if (DeviceId(device.toMap().value("id").toString()) == deviceId) {
                qDebug() << "found added device" << device.toMap().value("params");
                qDebug() << "expected deviceParams:" << newDeviceParams;
                // check if after the reboot the settings are unchanged
                verifyParams(deviceParams, device.toMap().value("params").toList());
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Device missing in config!");
    }

    // delete it
    params.clear();
    params.insert("deviceId", deviceId);
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response);
}


void TestDevices::reconfigureByDiscovery_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<Device::DeviceError>("error");
    QTest::addColumn<QVariantList>("discoveryParams");

    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", mockDiscoveryResultCountParamTypeId);
    resultCountParam.insert("value", 2);
    discoveryParams.append(resultCountParam);

    QTest::newRow("discover 2 devices with params") << mockThingClassId << 2 << Device::DeviceErrorNoError << discoveryParams;
}

void TestDevices::reconfigureByDiscovery()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(int, resultCount);
    QFETCH(Device::DeviceError, error);
    QFETCH(QVariantList, discoveryParams);

    qCDebug(dcTests()) << "Discovering...";
    QVariantMap params;
    params.insert("deviceClassId", thingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response);
    if (error == Device::DeviceErrorNoError) {
        QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), resultCount);
    }

    // add Discovered Device 1 port 55555
    QVariantList deviceDescriptors = response.toMap().value("params").toMap().value("deviceDescriptors").toList();

    ThingDescriptorId descriptorId;
    foreach (const QVariant &descriptor, deviceDescriptors) {
        // find the device with port 55555
        if (descriptor.toMap().value("description").toString() == "55555") {
            descriptorId = ThingDescriptorId(descriptor.toMap().value("id").toString());
            qDebug() << descriptorId.toString();
            break;
        }
    }

    QVERIFY(!descriptorId.isNull());

    qCDebug(dcTests()) << "Adding...";

    params.clear();
    response.clear();
    params.insert("deviceClassId", thingClassId);
    params.insert("name", "Discoverd mock device");
    params.insert("deviceDescriptorId", descriptorId);
    response = injectAndWait("Devices.AddConfiguredDevice", params);

    DeviceId deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!deviceId.isNull());

    // and now rediscover and find the existing device in the discovery results
    qCDebug(dcTests()) << "Re-Discovering...";

    params.clear();
    response.clear();
    params.insert("deviceClassId", thingClassId);
    params.insert("discoveryParams", discoveryParams);
    response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response, error);
    if (error == Device::DeviceErrorNoError) {
        QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), resultCount);
    }

    deviceDescriptors = response.toMap().value("params").toMap().value("deviceDescriptors").toList();

    // find the already added device
    descriptorId = ThingDescriptorId(); // reset it first
    foreach (const QVariant &descriptor, deviceDescriptors) {
        if (descriptor.toMap().value("deviceId").toUuid().toString() == deviceId.toString()) {
            descriptorId = ThingDescriptorId(descriptor.toMap().value("id").toString());
            break;
        }
    }
    QVERIFY2(!descriptorId.isNull(), QString("Device %1 not found in discovery results: %2").arg(deviceId.toString()).arg(qUtf8Printable(QJsonDocument::fromVariant(response).toJson())).toUtf8());

    qCDebug(dcTests()) << "Reconfiguring...";

    response.clear();
    params.clear();
    params.insert("deviceDescriptorId", descriptorId);
    // override port param
    QVariantMap portParam;
    portParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    portParam.insert("value", "55556");
    params.insert("deviceParams", QVariantList() << portParam);
    response = injectAndWait("Devices.ReconfigureDevice", params);
    verifyDeviceError(response, error);

    response.clear();
    response = injectAndWait("Devices.GetConfiguredDevices", QVariantMap());

    QVariantMap deviceMap;
    bool found = false;
    foreach (const QVariant device, response.toMap().value("params").toMap().value("devices").toList()) {
        if (DeviceId(device.toMap().value("id").toString()) == deviceId) {
            qDebug() << "found added device" << device.toMap().value("params");
            found = true;
            deviceMap = device.toMap();
            break;
        }
    }

    QVERIFY2(found, "Device missing in config!");
    QCOMPARE(deviceMap.value("id").toUuid(), QUuid(deviceId));
    if (deviceMap.contains("setupComplete"))
        QVERIFY2(deviceMap.value("setupComplete").toBool(), "Setup not completed after edit");

    // Note: this shows that by discovery a not editable param (name) can be changed!
    foreach (QVariant param, deviceMap.value("params").toList()) {
        if (param.toMap().value("paramTypeId") == mockThingHttpportParamTypeId) {
            QCOMPARE(param.toMap().value("value").toInt(), 55556);
        }
    }

    // check if the daemons are running
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    // check if old daemon is still running (should not)
    QNetworkRequest request(QUrl(QString("http://localhost:%1").arg(55555)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QVERIFY2(reply->error(), "The old daemon is still running");
    reply->deleteLater();

    // check if the daemon is really running on the new port
    request = QNetworkRequest(QUrl(QString("http://localhost:%1").arg(55556)));
    reply = nam.get(request);
    spy.wait();
    QVERIFY2(reply->error() == QNetworkReply::NoError, "The new daemon is not running");
    reply->deleteLater();

    params.clear();
    params.insert("deviceId", deviceId.toString());
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response);
}

void TestDevices::reconfigureByDiscoveryAndPair()
{
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", displayPinMockDiscoveryResultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    qCDebug(dcTests()) << "Discovering devices...";

    QVariantMap params;
    params.insert("deviceClassId", displayPinMockThingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response);
    QVariantList deviceDescriptors = response.toMap().value("params").toMap().value("deviceDescriptors").toList();

    qCDebug(dcTests()) << "Discovery result:" << qUtf8Printable(QJsonDocument::fromVariant(deviceDescriptors).toJson(QJsonDocument::Indented));
    QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), 1);

    // add Discovered Device 1 port 55555

    QVariant descriptor = deviceDescriptors.first();
    ThingDescriptorId descriptorId = ThingDescriptorId(descriptor.toMap().value("id").toString());
    QVERIFY2(!descriptorId.isNull(), "DeviceDescriptorId is Null");

    qCDebug(dcTests()) << "Pairing descriptorId:" << descriptorId;

    params.clear();
    response.clear();
    params.insert("deviceClassId", displayPinMockThingClassId);
    params.insert("name", "Discoverd mock device");
    params.insert("deviceDescriptorId", descriptorId);
    response = injectAndWait("Devices.PairDevice", params);
    verifyDeviceError(response);

    PairingTransactionId pairingTransactionId = PairingTransactionId(response.toMap().value("params").toMap().value("pairingTransactionId").toString());
    qCDebug(dcTests()) << "PairDevice result:" << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    qCDebug(dcTests()) << "Confirming pairing for transaction ID" << pairingTransactionId;
    params.clear();
    response.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());
    params.insert("secret", "243681");
    response = injectAndWait("Devices.ConfirmPairing", params);
    verifyDeviceError(response);

    DeviceId deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!deviceId.isNull());

    qCDebug(dcTests()) << "Discovering again...";

    // and now rediscover, and edit the first device with the second
    params.clear();
    response.clear();
    params.insert("deviceClassId", displayPinMockThingClassId);
    params.insert("discoveryParams", discoveryParams);
    response = injectAndWait("Devices.GetDiscoveredDevices", params);

    deviceDescriptors = response.toMap().value("params").toMap().value("deviceDescriptors").toList();
    qCDebug(dcTests()) << "Discovery result:" << qUtf8Printable(QJsonDocument::fromVariant(deviceDescriptors).toJson(QJsonDocument::Indented));

    verifyDeviceError(response, Device::DeviceErrorNoError);
    QCOMPARE(deviceDescriptors.count(), 1);

    descriptor = deviceDescriptors.first();
    QVERIFY2(DeviceId(descriptor.toMap().value("deviceId").toString()) == deviceId, "DeviceID not set in descriptor");

    // get the descriptor again
    descriptorId = ThingDescriptorId(descriptor.toMap().value("id").toString());

    QVERIFY(!descriptorId.isNull());

    qDebug() << "Reconfiguring device by pairing again" << descriptorId;

    params.clear();
    response.clear();
    params.insert("deviceClassId", displayPinMockThingClassId);
    params.insert("name", "Discoverd mock device");
    params.insert("deviceDescriptorId", descriptorId);
    response = injectAndWait("Devices.PairDevice", params);
    verifyDeviceError(response);

    pairingTransactionId = PairingTransactionId(response.toMap().value("params").toMap().value("pairingTransactionId").toString());
    qCDebug(dcTests()) << "PairDevice result:" << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));


    qCDebug(dcTests()) << "Confirming pairing for transaction ID" << pairingTransactionId;
    params.clear();
    response.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());
    params.insert("secret", "243681");
    response = injectAndWait("Devices.ConfirmPairing", params);
    verifyDeviceError(response);

    deviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!deviceId.isNull());

}

void TestDevices::reconfigureAutodevice()
{
    qCDebug(dcTests()) << "Reconfigure auto device";

    // Get the autodevice
    QList<Thing*> devices  = NymeaCore::instance()->thingManager()->findConfiguredThings(autoMockThingClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one auto-created Mock Device for this test");

    // Get current auto device infos
    Thing *currentDevice = devices.first();
    DeviceId deviceId = currentDevice->id();
    int currentPort = currentDevice->paramValue(autoMockThingHttpportParamTypeId).toInt();

    // Trigger reconfigure signal in mock device
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy spy(nam, &QNetworkAccessManager::finished);
    QNetworkReply *reply = nam->get(QNetworkRequest(QUrl(QString("http://localhost:%1/reconfigureautodevice").arg(currentPort))));
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    Thing *device = NymeaCore::instance()->thingManager()->findConfiguredThing(deviceId);
    QVERIFY(device);
    int newPort = device->paramValue(autoMockThingHttpportParamTypeId).toInt();
    // Note: reconfigure autodevice increases the http port by 1
    QCOMPARE(newPort, currentPort + 1);
}


void TestDevices::removeDevice_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<Device::DeviceError>("deviceError");

    QTest::newRow("Existing Device") << DeviceId(m_mockThingId) << Device::DeviceErrorNoError;
    QTest::newRow("Not existing Device") << DeviceId::createDeviceId() << Device::DeviceErrorDeviceNotFound;
//    QTest::newRow("Auto device") << m_mockDeviceAutoId << Device::DeviceErrorCreationMethodNotSupported;
}

void TestDevices::removeDevice()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(Device::DeviceError, deviceError);

    NymeaSettings settings(NymeaSettings::SettingsRoleThings);
    settings.beginGroup("ThingConfig");
    if (deviceError == Device::DeviceErrorNoError) {
        settings.beginGroup(m_mockThingId.toString());
        // Make sure we have some config values for this device
        QVERIFY(settings.allKeys().count() > 0);
    }

    QVariantMap params;
    params.insert("deviceId", deviceId);

    QVariant response = injectAndWait("Devices.RemoveConfiguredDevice", params);

    verifyDeviceError(response, deviceError);

    if (Device::DeviceErrorNoError) {
        // Make sure the device is gone from settings too
        QCOMPARE(settings.allKeys().count(), 0);
    }
}

void TestDevices::removeAutoDevice()
{
    // Setup connection to mock client
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy spy(nam, SIGNAL(finished(QNetworkReply*)));

    // First try to make a manually created device disappear. It must not go away

    QList<Thing*> devices = NymeaCore::instance()->thingManager()->findConfiguredThings(mockThingClassId);
    int oldCount = devices.count();
    QVERIFY2(oldCount > 0, "There needs to be at least one configured Mock Device for this test");
    Thing *device = devices.first();

    // trigger disappear signal in mock device
    int port = device->paramValue(autoMockThingHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/disappear").arg(port)));
    QNetworkReply *reply = nam->get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
    QVERIFY2(NymeaCore::instance()->thingManager()->findConfiguredThings(mockThingClassId).count() == oldCount, "Mock device has disappeared even though it shouldn't");

    // Ok, now do the same with an autocreated one. It should go away

    devices = NymeaCore::instance()->thingManager()->findConfiguredThings(autoMockThingClassId);
    oldCount = devices.count();
    QVERIFY2(oldCount > 0, "There needs to be at least one auto-created Mock Device for this test");
    device = devices.first();

    // trigger disappear signal in mock device
    spy.clear();
    port = device->paramValue(autoMockThingHttpportParamTypeId).toInt();
    request.setUrl(QUrl(QString("http://localhost:%1/disappear").arg(port)));
    reply = nam->get(request);

    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    // Make sure one mock device has disappeared
    QCOMPARE(NymeaCore::instance()->thingManager()->findConfiguredThings(autoMockThingClassId).count(), oldCount - 1);
}

void TestDevices::testBrowsing_data()
{
    QTest::addColumn<DeviceId>("deviceId");

    QTest::newRow("regular mock device") << DeviceId(m_mockThingId);
    QTest::newRow("async mock device") << DeviceId(m_mockThingAsyncId);
}

void TestDevices::testBrowsing()
{
    QFETCH(DeviceId, deviceId);

    // Check if mockdevice is browsable
    QVariant response = injectAndWait("Devices.GetSupportedDevices");

    QVariantMap mockDeviceClass;
    foreach (const QVariant &deviceClassVariant, response.toMap().value("params").toMap().value("deviceClasses").toList()) {
        if (ThingClassId(deviceClassVariant.toMap().value("id").toString()) == mockThingClassId) {
            mockDeviceClass = deviceClassVariant.toMap();
        }
    }

    QVERIFY2(ThingClassId(mockDeviceClass.value("id").toString()) == mockThingClassId, "Could not find mock device");
    QCOMPARE(mockDeviceClass.value("browsable").toBool(), true);


    // Browse it
    QVariantMap params;
    params.insert("deviceId", deviceId);
    response = injectAndWait("Devices.BrowseDevice", params);
    QCOMPARE(response.toMap().value("params").toMap().value("deviceError").toString(), QString("DeviceErrorNoError"));
    QVariantList browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QVERIFY2(browserEntries.count() > 0, "BrowseDevice did not return any items.");

    // Browse item 001, it should be a folder with 2 items
    params.insert("itemId", "001");
    response = injectAndWait("Devices.BrowseDevice", params);
    QCOMPARE(response.toMap().value("params").toMap().value("deviceError").toString(), QString("DeviceErrorNoError"));
    browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QVERIFY2(browserEntries.count() == 2, "BrowseDevice did not return 2 items as childs in folder with id 001.");

    // Browse a non-existent item
    params["itemId"] = "this-does-not-exist";
    response = injectAndWait("Devices.BrowseDevice", params);
    browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QCOMPARE(response.toMap().value("params").toMap().value("deviceError").toString(), QString("DeviceErrorItemNotFound"));
    QCOMPARE(browserEntries.count(), 0);


}

void TestDevices::discoverDeviceParenting()
{
    // Try to discover a mock child device. We don't have a mockParent yet, so it should fail
    ThingDiscoveryInfo *discoveryInfo = NymeaCore::instance()->thingManager()->discoverThings(childMockThingClassId, ParamList());
    {
        QSignalSpy spy(discoveryInfo, &ThingDiscoveryInfo::finished);
        spy.wait();
    }
    QVERIFY(discoveryInfo->thingDescriptors().count() == 0);


    // Now create a mock parent by discovering...
    discoveryInfo = NymeaCore::instance()->thingManager()->discoverThings(parentMockThingClassId, ParamList());
    {
        QSignalSpy spy(discoveryInfo, &ThingDiscoveryInfo::finished);
        spy.wait();
    }
    QVERIFY(discoveryInfo->thingDescriptors().count() == 1);
    ThingDescriptorId descriptorId = discoveryInfo->thingDescriptors().first().id();

    QSignalSpy addSpy(NymeaCore::instance()->thingManager(), &ThingManager::thingAdded);
    ThingSetupInfo *setupInfo = NymeaCore::instance()->thingManager()->addConfiguredThing(descriptorId, ParamList(), "Mock Parent (Discovered)");
    {
        QSignalSpy spy(setupInfo, &ThingSetupInfo::finished);
        spy.wait();
    }
    QCOMPARE(setupInfo->status(), Thing::ThingErrorNoError);

    addSpy.wait();
    QCOMPARE(addSpy.count(), 2); // Mock device parent will also auto-create a child instantly

    Thing *parentDevice = addSpy.at(0).first().value<Thing*>();
    qCDebug(dcTests()) << "Added device:" << parentDevice->name();
    QVERIFY(parentDevice->thingClassId() == parentMockThingClassId);


    // Ok we have our parent device, let's discover for childs again
    discoveryInfo = NymeaCore::instance()->thingManager()->discoverThings(childMockThingClassId, ParamList());
    {
        QSignalSpy spy(discoveryInfo, &ThingDiscoveryInfo::finished);
        spy.wait();
    }
    QVERIFY(discoveryInfo->thingDescriptors().count() == 1);
    descriptorId = discoveryInfo->thingDescriptors().first().id();

    // Found one! Adding it...
    addSpy.clear();
    setupInfo = NymeaCore::instance()->thingManager()->addConfiguredThing(descriptorId, ParamList(), "Mock Child (Discovered)");
    {
        QSignalSpy spy(setupInfo, &ThingSetupInfo::finished);
        spy.wait();
    }
    QCOMPARE(setupInfo->status(), Thing::ThingErrorNoError);

    QCOMPARE(addSpy.count(), 1);

    Thing *childDevice = addSpy.at(0).first().value<Thing*>();
    qCDebug(dcTests()) << "Added device:" << childDevice->name();
    QVERIFY(childDevice->thingClassId() == childMockThingClassId);

    // Now delete the parent and make sure the child will be deleted too
    QSignalSpy removeSpy(NymeaCore::instance(), &NymeaCore::thingRemoved);
    QPair<Thing::ThingError, QList<RuleId> > ret = NymeaCore::instance()->removeConfiguredThing(parentDevice->id(), QHash<RuleId, RuleEngine::RemovePolicy>());
    QCOMPARE(ret.first, Thing::ThingErrorNoError);
    QCOMPARE(removeSpy.count(), 3); // The parent, the auto-mock and the discovered mock

}

void TestDevices::testExecuteBrowserItem_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<QString>("itemId");
    QTest::addColumn<QString>("deviceError");

    QTest::newRow("regular mock device") << DeviceId(m_mockThingId) << "002" << "DeviceErrorNoError";
    QTest::newRow("regular mock device") << DeviceId(m_mockThingId) << "001" << "DeviceErrorItemNotExecutable";
    QTest::newRow("async mock device") << DeviceId(m_mockThingAsyncId) << "002" << "DeviceErrorNoError";
}

void TestDevices::testExecuteBrowserItem()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(QString, itemId);
    QFETCH(QString, deviceError);

    QVariantMap params;
    params.insert("deviceId", deviceId);
    params.insert("itemId", itemId);
    QVariant response = injectAndWait("Actions.ExecuteBrowserItem", params);
    qCDebug(dcTests()) << "resp" << response;

    QCOMPARE(response.toMap().value("params").toMap().value("deviceError").toString(), deviceError);
}

void TestDevices::testExecuteBrowserItemAction_data()
{
    QTest::addColumn<DeviceId>("deviceId");

    QTest::newRow("regular mock device") << DeviceId(m_mockThingId);
    QTest::newRow("async mock device") << DeviceId(m_mockThingAsyncId);
}

void TestDevices::testExecuteBrowserItemAction()
{
    QFETCH(DeviceId, deviceId);

    QVariantMap getItemsParams;
    getItemsParams.insert("deviceId", deviceId);
    QVariant response = injectAndWait("Devices.BrowseDevice", getItemsParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    QVariantList browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QVERIFY(browserEntries.count() > 2);

    QVariantMap item002; // Find the item we need for this test
    foreach (const QVariant &item, browserEntries) {
        if (item.toMap().value("id").toString() == "002") {
            item002 = item.toMap();
            break;
        }
    }
    QVERIFY2(item002.value("id").toString() == QString("002"), "Item with context actions not found");
    QVERIFY2(item002.value("actionTypeIds").toList().count() > 0, "Item doesn't have actionTypeIds");
    QVERIFY2(ActionTypeId(item002.value("actionTypeIds").toList().first().toString()) == mockAddToFavoritesBrowserItemActionTypeId, "AddToFavorites action type id not found in item");


    // Browse favorites
    // ID is "favorites" in mockDevice
    // It should be ampty at this point
    getItemsParams.insert("itemId", "favorites");
    response = injectAndWait("Devices.BrowseDevice", getItemsParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QVERIFY2(browserEntries.count() == 0, "Favorites should be empty at this point");

    // Now add an item to the favorites
    QVariantMap actionParams;
    actionParams.insert("deviceId", deviceId);
    actionParams.insert("itemId", "002");
    actionParams.insert("actionTypeId", mockAddToFavoritesBrowserItemActionTypeId);
    response = injectAndWait("Actions.ExecuteBrowserItemAction", actionParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("deviceError").toString(), QString("DeviceErrorNoError"));

    // Fetch the list again
    response = injectAndWait("Devices.BrowseDevice", getItemsParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QCOMPARE(browserEntries.count(), 1);

    QString favoriteItemId = browserEntries.first().toMap().value("id").toString();
    QVERIFY2(!favoriteItemId.isEmpty(), "ItemId is empty in favorites list");

    // Now remove the again from favorites
    actionParams.clear();
    actionParams.insert("deviceId", deviceId);
    actionParams.insert("itemId", favoriteItemId);
    actionParams.insert("actionTypeId", mockRemoveFromFavoritesBrowserItemActionTypeId);
    response = injectAndWait("Actions.ExecuteBrowserItemAction", actionParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("deviceError").toString(), QString("DeviceErrorNoError"));

    // Fetch the list again
    response = injectAndWait("Devices.BrowseDevice", getItemsParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QCOMPARE(browserEntries.count(), 0);

}

void TestDevices::executeAction_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<ActionTypeId>("actionTypeId");
    QTest::addColumn<QVariantList>("actionParams");
    QTest::addColumn<Device::DeviceError>("error");

    QVariantList params;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 5);
    params.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    params.append(param2);

    QTest::newRow("valid action") << DeviceId(m_mockThingId) << mockWithParamsActionTypeId << params << Device::DeviceErrorNoError;
    QTest::newRow("invalid deviceId") << DeviceId::createDeviceId() << mockWithParamsActionTypeId << params << Device::DeviceErrorDeviceNotFound;
    QTest::newRow("invalid actionTypeId") << DeviceId(m_mockThingId) << ActionTypeId::createActionTypeId() << params << Device::DeviceErrorActionTypeNotFound;
    QTest::newRow("missing params") << DeviceId(m_mockThingId) << mockWithParamsActionTypeId << QVariantList() << Device::DeviceErrorMissingParameter;
    QTest::newRow("async action") << DeviceId(m_mockThingId) << mockAsyncActionTypeId << QVariantList() << Device::DeviceErrorNoError;
    QTest::newRow("broken action") << DeviceId(m_mockThingId) << mockFailingActionTypeId << QVariantList() << Device::DeviceErrorSetupFailed;
    QTest::newRow("async broken action") << DeviceId(m_mockThingId) << mockAsyncFailingActionTypeId << QVariantList() << Device::DeviceErrorSetupFailed;
}

void TestDevices::executeAction()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(ActionTypeId, actionTypeId);
    QFETCH(QVariantList, actionParams);
    QFETCH(Device::DeviceError, error);

    QVariantMap params;
    params.insert("actionTypeId", actionTypeId);
    params.insert("deviceId", deviceId);
    params.insert("params", actionParams);
    QVariant response = injectAndWait("Devices.ExecuteAction", params);
    qDebug() << "executeActionresponse" << response;
    verifyError(response, "deviceError", enumValueName(error));

    // Fetch action execution history from mock device
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(m_mockThing1Port)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
    QByteArray data = reply->readAll();

    if (error == Device::DeviceErrorNoError) {
        QVERIFY2(actionTypeId == ActionTypeId(data), QString("ActionTypeId mismatch. Got %1, Expected: %2")
                 .arg(ActionTypeId(data).toString()).arg(actionTypeId.toString()).toLatin1().data());
    } else {
        QVERIFY2(data.length() == 0, QString("Data is %1, should be empty.").arg(QString(data)).toLatin1().data());
    }

    // cleanup for the next run
    spy.clear();
    request.setUrl(QUrl(QString("http://localhost:%1/clearactionhistory").arg(m_mockThing1Port)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    spy.clear();
    request.setUrl(QUrl(QString("http://localhost:%1/actionhistory").arg(m_mockThing1Port)));
    reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
    data = reply->readAll();
    qDebug() << "cleared data:" << data;

}

void TestDevices::triggerEvent()
{
    enableNotifications({"Devices"});
    QList<Thing*> devices = NymeaCore::instance()->thingManager()->findConfiguredThings(mockThingClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Thing *device = devices.first();


    QSignalSpy spy(NymeaCore::instance(), SIGNAL(eventTriggered(const Event&)));
    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Setup connection to mock client
    QNetworkAccessManager nam;

    // trigger event in mock device
    int port = device->paramValue(mockThingHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(port).arg(mockEvent1EventTypeId.toString())));
    QNetworkReply *reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);

    // Lets wait for the notification
    spy.wait();
    QVERIFY(spy.count() > 0);
    for (int i = 0; i < spy.count(); i++ ){
        Event event = spy.at(i).at(0).value<Event>();
        if (event.thingId() == device->id()) {
            // Make sure the event contains all the stuff we expect
            QCOMPARE(event.eventTypeId(), mockEvent1EventTypeId);
        }
    }

    // Check for the notification on JSON API
    QVariantList notifications;
    notifications = checkNotifications(notificationSpy, "Devices.EventTriggered");
    QVERIFY2(notifications.count() == 1, "Should get Devices.EventTriggered notification");
    QVariantMap notificationContent = notifications.first().toMap().value("params").toMap();

    QCOMPARE(notificationContent.value("event").toMap().value("deviceId").toUuid().toString(), device->id().toString());
    QCOMPARE(notificationContent.value("event").toMap().value("eventTypeId").toUuid().toString(), mockEvent1EventTypeId.toString());
}

void TestDevices::triggerStateChangeEvent()
{
    enableNotifications({"Devices"});

    QList<Thing*> devices = NymeaCore::instance()->thingManager()->findConfiguredThings(mockThingClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Thing *device = devices.first();

    QSignalSpy spy(NymeaCore::instance(), SIGNAL(eventTriggered(const Event&)));
    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Setup connection to mock client
    QNetworkAccessManager nam;

    // trigger state changed event in mock device
    int port = device->paramValue(mockThingHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(port).arg(mockIntStateTypeId.toString()).arg(11)));
    QNetworkReply *reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);

    // Lets wait for the notification
    spy.wait();
    QVERIFY(spy.count() > 0);
    for (int i = 0; i < spy.count(); i++ ){
        Event event = spy.at(i).at(0).value<Event>();
        if (event.thingId() == device->id()) {
            // Make sure the event contains all the stuff we expect
            QCOMPARE(event.eventTypeId().toString(), mockIntStateTypeId.toString());
            QCOMPARE(event.param(ParamTypeId(mockIntStateTypeId.toString())).value().toInt(), 11);
        }
    }

    // Check for the notification on JSON API
    QVariantList notifications;
    notifications = checkNotifications(notificationSpy, "Devices.EventTriggered");
    QVERIFY2(notifications.count() == 1, "Should get Devices.EventTriggered notification");
    QVariantMap notificationContent = notifications.first().toMap().value("params").toMap();

    QCOMPARE(notificationContent.value("event").toMap().value("deviceId").toUuid().toString(), device->id().toString());
    QCOMPARE(notificationContent.value("event").toMap().value("eventTypeId").toUuid().toString(), mockIntEventTypeId.toString());

}

void TestDevices::params()
{
    Event event;
    ParamList params;
    ParamTypeId id = ParamTypeId::createParamTypeId();
    Param p(id, "foo bar");
    params.append(p);
    event.setParams(params);

    QVERIFY(event.param(id).value().toString() == "foo bar");
    QVERIFY(!event.param(ParamTypeId::createParamTypeId()).value().isValid());
}

void TestDevices::asyncSetupEmitsSetupStatusUpdate()
{
    QVariantMap configuredDevices = injectAndWait("Devices.GetConfiguredDevices").toMap();
    foreach (const QVariant &deviceVariant, configuredDevices.value("params").toMap().value("devices").toList()) {
        QVariantMap device = deviceVariant.toMap();
        qCDebug(dcTests()) << "confdiguredd device" << device.value("setupStatus");
    }

    // Restart the core instance to check if settings are loaded at startup
    restartServer();
    enableNotifications({"Devices"});

    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    configuredDevices = injectAndWait("Devices.GetConfiguredDevices").toMap();
    QList<QUuid> devicesWithSetupInProgress;
    foreach (const QVariant &deviceVariant, configuredDevices.value("params").toMap().value("devices").toList()) {
        QVariantMap device = deviceVariant.toMap();
        qCDebug(dcTests()) << "Configured device" << device.value("name").toString() << "with setup status" << device.value("setupStatus").toString();
        if (device.value("setupStatus").toString() == "ThingSetupStatusInProgress") {
            devicesWithSetupInProgress << device.value("id").toUuid();
        }
    }
    QVERIFY2(devicesWithSetupInProgress.count() > 0, "This test requires at least one device that is still being set up at this point.");

    QDateTime maxTime = QDateTime::currentDateTime().addSecs(10);
    while (QDateTime::currentDateTime() < maxTime && devicesWithSetupInProgress.count() > 0) {
        QList<QList<QVariant>> notifications = notificationSpy;
        while (notifications.count() > 0) {
            QByteArray notificationData = notifications.takeFirst().at(1).toByteArray();
            QVariantMap notification = QJsonDocument::fromJson(notificationData).toVariant().toMap();
            if (notification.value("notification").toString() == "Devices.DeviceChanged") {
                QString setupStatus = notification.value("params").toMap().value("device").toMap().value("setupStatus").toString();
                if (setupStatus == "ThingSetupStatusComplete") {
                    qCDebug(dcTests()) << "Device setup completed for" << notification.value("params").toMap().value("device").toMap().value("name").toString();
                    DeviceId deviceId = notification.value("params").toMap().value("device").toMap().value("id").toUuid();
                    devicesWithSetupInProgress.removeAll(deviceId);
                }
            }
        }
        notificationSpy.clear();
        if (devicesWithSetupInProgress.count() > 0) {
            notificationSpy.wait();
        }
    }

    QVERIFY2(devicesWithSetupInProgress.isEmpty(), "Some devices did not finish the setup!");
}

#include "testdevices.moc"
QTEST_MAIN(TestDevices)

