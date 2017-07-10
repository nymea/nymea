/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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
#include "guhsettings.h"
#include "plugin/deviceplugin.h"

#include <QDebug>
#include <QSignalSpy>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

using namespace guhserver;

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

    void verifyInterfaces();

    void addConfiguredDevice_data();
    void addConfiguredDevice();

    void getConfiguredDevices();

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

    void reconfigureDevices_data();
    void reconfigureDevices();

    void reconfigureByDiscovery_data();
    void reconfigureByDiscovery();

    // Keep this the last one! It'll remove the configured mock device
    void removeDevice_data();
    void removeDevice();

};

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
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("valid plugin") << mockPluginId << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid plugin") << PluginId::createPluginId() << DeviceManager::DeviceErrorPluginNotFound;
}

void TestDevices::getPluginConfig()
{
    QFETCH(PluginId, pluginId);
    QFETCH(DeviceManager::DeviceError, error);

    QVariantMap params;
    params.insert("pluginId", pluginId);
    QVariant response = injectAndWait("Devices.GetPluginConfiguration", params);
    verifyDeviceError(response, error);
}

void TestDevices::setPluginConfig_data()
{
    QTest::addColumn<PluginId>("pluginId");
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("valid") << mockPluginId << QVariant(13) << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid plugin") << PluginId::createPluginId() << QVariant(13) <<  DeviceManager::DeviceErrorPluginNotFound;
    QTest::newRow("too big") << mockPluginId << QVariant(130) << DeviceManager::DeviceErrorInvalidParameter;
    QTest::newRow("too small") << mockPluginId << QVariant(-13) << DeviceManager::DeviceErrorInvalidParameter;
    QTest::newRow("wrong type") << mockPluginId << QVariant("wrontType") << DeviceManager::DeviceErrorInvalidParameter;
}

void TestDevices::setPluginConfig()
{
    QFETCH(PluginId, pluginId);
    QFETCH(QVariant, value);
    QFETCH(DeviceManager::DeviceError, error);

    QVariantMap params;
    params.insert("pluginId", pluginId);

    QVariantList configuration;
    QVariantMap configParam;
    configParam.insert("paramTypeId", configParamIntParamTypeId);
    configParam.insert("value", value);
    configuration.append(configParam);
    params.insert("configuration", configuration);
    QVariant response = injectAndWait("Devices.SetPluginConfiguration", params);
    verifyDeviceError(response, error);

    if (error == DeviceManager::DeviceErrorNoError) {
        params.clear();
        params.insert("pluginId", pluginId);
        response = injectAndWait("Devices.GetPluginConfiguration", params);
        verifyDeviceError(response);
        qDebug() << value << response.toMap().value("params").toMap().value("configuration").toList().first();
        QVERIFY2(ParamTypeId(response.toMap().value("params").toMap().value("configuration").toList().first().toMap().value("paramTypeId").toString()) == configParamIntParamTypeId, "Value not set correctly");
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
        if (VendorId(listEntry.toMap().value("id").toString()) == guhVendorId) {
            found = true;
        }
    }
    QCOMPARE(found, true);
}

void TestDevices::getSupportedDevices_data()
{
    QTest::addColumn<VendorId>("vendorId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("vendor guh") << guhVendorId << 1;
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
    params.insert("vendorId", guhVendorId);
    QVariant result = injectAndWait("Devices.GetSupportedDevices", params);
    QVariantList supportedDevices = result.toMap().value("params").toMap().value("deviceClasses").toList();

    QVariantMap mockDevice;
    foreach (const QVariant &deviceClass, supportedDevices) {
        if (deviceClass.toMap().value("id") == mockDeviceClassId) {
            mockDevice = deviceClass.toMap();
        }
    }
    QVERIFY(!mockDevice.isEmpty());

    QVariantList interfaces = mockDevice.value("interfaces").toList();
    // Must contain gateway, but must not contain anything else as device manager should filter it away
    QCOMPARE(interfaces.count() == 1, true);
    QVERIFY(interfaces.contains("gateway"));
}

void TestDevices::addConfiguredDevice_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<QVariantList>("deviceParams");
    QTest::addColumn<DeviceManager::DeviceError>("deviceError");

    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", httpportParamTypeId.toString());
    httpportParam.insert("value", m_mockDevice1Port - 1);
    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", asyncParamTypeId);
    asyncParam.insert("value", true);
    QVariantMap brokenParam;
    brokenParam.insert("paramTypeId", brokenParamTypeId);
    brokenParam.insert("value", true);

    QVariantList deviceParams;

    deviceParams.clear(); deviceParams << httpportParam;
    QTest::newRow("User, JustAdd") << mockDeviceClassId << deviceParams << DeviceManager::DeviceErrorNoError;
    deviceParams.clear(); deviceParams << httpportParam << asyncParam;
    QTest::newRow("User, JustAdd, Async") << mockDeviceClassId << deviceParams << DeviceManager::DeviceErrorNoError;
    QTest::newRow("Invalid DeviceClassId") << DeviceClassId::createDeviceClassId() << deviceParams << DeviceManager::DeviceErrorDeviceClassNotFound;
    deviceParams.clear(); deviceParams << httpportParam << brokenParam;
    QTest::newRow("Setup failure") << mockDeviceClassId << deviceParams << DeviceManager::DeviceErrorSetupFailed;
    deviceParams.clear(); deviceParams << httpportParam << asyncParam << brokenParam;
    QTest::newRow("Setup failure, Async") << mockDeviceClassId << deviceParams << DeviceManager::DeviceErrorSetupFailed;

    QVariantList invalidDeviceParams;
    QTest::newRow("User, JustAdd, missing params") << mockDeviceClassId << invalidDeviceParams << DeviceManager::DeviceErrorMissingParameter;

    QVariantMap fakeparam;
    fakeparam.insert("paramTypeId", ParamTypeId::createParamTypeId());
    invalidDeviceParams.append(fakeparam);
    QTest::newRow("User, JustAdd, invalid param") << mockDeviceClassId << invalidDeviceParams << DeviceManager::DeviceErrorInvalidParameter;

    fakeparam.insert("value", "buhuu");
    invalidDeviceParams.clear();
    invalidDeviceParams.append(fakeparam);
    QTest::newRow("User, JustAdd, wrong param") << mockDeviceClassId << invalidDeviceParams << DeviceManager::DeviceErrorInvalidParameter;

}

void TestDevices::addConfiguredDevice()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(QVariantList, deviceParams);
    QFETCH(DeviceManager::DeviceError, deviceError);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("name", "Test Add Device");
    params.insert("deviceParams", deviceParams);
    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    qDebug() << "response is" << response;

    verifyDeviceError(response, deviceError);

    if (deviceError == DeviceManager::DeviceErrorNoError) {
        QUuid deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
        params.clear();
        params.insert("deviceId", deviceId.toString());
        response = injectAndWait("Devices.RemoveConfiguredDevice", params);
        verifyDeviceError(response);
    }
}

void TestDevices::getConfiguredDevices()
{
    QVariant response = injectAndWait("Devices.GetConfiguredDevices");

    QVariantList devices = response.toMap().value("params").toMap().value("devices").toList();
    QCOMPARE(devices.count(), 2); // There should be one auto created mock device and one created in initTestcase()
}

void TestDevices::storedDevices()
{
    QVariantMap params;
    params.insert("deviceClassId", mockDeviceClassId);
    params.insert("name", "Test stored Device");
    QVariantList deviceParams;
    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", asyncParamTypeId);
    asyncParam.insert("value", false);
    deviceParams.append(asyncParam);
    QVariantMap brokenParam;
    brokenParam.insert("paramTypeId", brokenParamTypeId);
    brokenParam.insert("value", false);
    deviceParams.append(brokenParam);
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", httpportParamTypeId);
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
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<DeviceManager::DeviceError>("error");
    QTest::addColumn<QVariantList>("discoveryParams");

    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", resultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QTest::newRow("valid deviceClassId") << mockDeviceClassId << 2 << DeviceManager::DeviceErrorNoError << QVariantList();
    QTest::newRow("valid deviceClassId with params") << mockDeviceClassId << 1 << DeviceManager::DeviceErrorNoError << discoveryParams;
    QTest::newRow("invalid deviceClassId") << DeviceClassId::createDeviceClassId() << 0 << DeviceManager::DeviceErrorDeviceClassNotFound << QVariantList();
}

void TestDevices::discoverDevices()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);
    QFETCH(DeviceManager::DeviceError, error);
    QFETCH(QVariantList, discoveryParams);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response, error);
    if (error == DeviceManager::DeviceErrorNoError) {
        QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), resultCount);
    }

    // If we found something, lets try to add it
    if (DeviceManager::DeviceErrorNoError) {
        DeviceDescriptorId descriptorId = DeviceDescriptorId(response.toMap().value("params").toMap().value("deviceDescriptors").toList().first().toMap().value("id").toString());

        params.clear();
        params.insert("deviceClassId", deviceClassId);
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
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<DeviceManager::DeviceError>("error");
    QTest::addColumn<bool>("waitForButtonPressed");

    QTest::newRow("Valid: Add PushButton device") << mockPushButtonDeviceClassId << DeviceManager::DeviceErrorNoError << true;
    QTest::newRow("Invalid: Add PushButton device (press to early)") << mockPushButtonDeviceClassId << DeviceManager::DeviceErrorSetupFailed << false;
}

void TestDevices::addPushButtonDevices()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(DeviceManager::DeviceError, error);
    QFETCH(bool, waitForButtonPressed);

    // Discover device
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", resultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response, DeviceManager::DeviceErrorNoError);
    QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), 1);


    // Pair device
    DeviceDescriptorId descriptorId = DeviceDescriptorId(response.toMap().value("params").toMap().value("deviceDescriptors").toList().first().toMap().value("id").toString());
    params.clear();
    params.insert("deviceClassId", deviceClassId);
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

    if (error == DeviceManager::DeviceErrorNoError) {
        DeviceId deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
        params.clear();
        params.insert("deviceId", deviceId.toString());
        response = injectAndWait("Devices.RemoveConfiguredDevice", params);
        verifyDeviceError(response);
    }
}

void TestDevices::addDisplayPinDevices_data()
{
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<DeviceManager::DeviceError>("error");
    QTest::addColumn<QString>("secret");

    QTest::newRow("Valid: Add DisplayPin device") << mockDisplayPinDeviceClassId << DeviceManager::DeviceErrorNoError << "243681";
    QTest::newRow("Invalid: Add DisplayPin device (wrong pin)") << mockDisplayPinDeviceClassId << DeviceManager::DeviceErrorSetupFailed << "243682";
}

void TestDevices::addDisplayPinDevices()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(DeviceManager::DeviceError, error);
    QFETCH(QString, secret);

    // Discover device
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", resultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response, DeviceManager::DeviceErrorNoError);
    QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), 1);

    // Pair device
    DeviceDescriptorId descriptorId = DeviceDescriptorId(response.toMap().value("params").toMap().value("deviceDescriptors").toList().first().toMap().value("id").toString());
    params.clear();
    params.insert("deviceClassId", deviceClassId);
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

    if (error == DeviceManager::DeviceErrorNoError) {
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
    params.insert("deviceClassId", mockParentDeviceClassId);
    params.insert("name", "Parent device");

    QVariant response = injectAndWait("Devices.AddConfiguredDevice", params);
    verifyDeviceError(response);

    DeviceId parentDeviceId = DeviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!parentDeviceId.isNull());

    // find child device
    response = injectAndWait("Devices.GetConfiguredDevices");

    QVariantList devices = response.toMap().value("params").toMap().value("devices").toList();

    DeviceId childDeviceId;
    foreach (const QVariant deviceVariant, devices) {
        QVariantMap deviceMap = deviceVariant.toMap();

        if (deviceMap.value("deviceClassId").toString() == mockChildDeviceClassId.toString()) {
            if (deviceMap.value("parentId") == parentDeviceId.toString()) {
                //qDebug() << QJsonDocument::fromVariant(deviceVariant).toJson();
                childDeviceId = DeviceId(deviceMap.value("id").toString());
            }
        }
    }
    QVERIFY2(!childDeviceId.isNull(), "Could not find child device");

    // Try to remove the child device
    params.clear();
    params.insert("deviceId", childDeviceId.toString());
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response, DeviceManager::DeviceErrorDeviceIsChild);

    // check if the child device is still there
    response = injectAndWait("Devices.GetConfiguredDevices");
    devices = response.toMap().value("params").toMap().value("devices").toList();
    bool found = false;
    foreach (const QVariant deviceVariant, devices) {
        QVariantMap deviceMap = deviceVariant.toMap();
        if (deviceMap.value("deviceClassId").toString() == mockChildDeviceClassId.toString()) {
            if (deviceMap.value("id") == childDeviceId.toString()) {
                found = true;
                break;
            }
        }
    }
    QVERIFY2(found, "Could not find child device.");

    // remove the parent device
    params.clear();
    params.insert("deviceId", parentDeviceId.toString());
    response = injectAndWait("Devices.RemoveConfiguredDevice", params);
    verifyDeviceError(response);

    // check if the child device is still there
    response = injectAndWait("Devices.GetConfiguredDevices");
    devices = response.toMap().value("params").toMap().value("devices").toList();
    found = false;
    foreach (const QVariant deviceVariant, devices) {
        QVariantMap deviceMap = deviceVariant.toMap();
        if (deviceMap.value("deviceClassId").toString() == mockChildDeviceClassId.toString()) {
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

    qDebug() << response;

    QVariantList eventTypes = response.toMap().value("params").toMap().value("eventTypes").toList();
    QCOMPARE(eventTypes.count(), resultCount);

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

void TestDevices::getStateType_data()
{
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<DeviceManager::DeviceError>("error");

    QTest::newRow("valid int state") << mockIntStateId << DeviceManager::DeviceErrorNoError;
    QTest::newRow("valid bool state") << mockBoolStateId << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid stateTypeId") << StateTypeId::createStateTypeId() << DeviceManager::DeviceErrorStateTypeNotFound;
}

void TestDevices::getStateType()
{
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(DeviceManager::DeviceError, error);

    QVariantMap params;
    params.insert("stateTypeId", stateTypeId);
    QVariant response = injectAndWait("States.GetStateType", params);
    verifyDeviceError(response, error);

    if (error != DeviceManager::DeviceErrorNoError)
        return;

    QVariantMap stateType = response.toMap().value("params").toMap().value("stateType").toMap();

    qDebug() << QJsonDocument::fromVariant(stateType).toJson();

    QVERIFY2(!stateType.isEmpty(), "Got no stateType");
    QCOMPARE(stateType.value("id").toString(), stateTypeId.toString());
}

void TestDevices::getStateValue_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<DeviceManager::DeviceError>("statusCode");

    QTest::newRow("valid deviceId") << m_mockDeviceId << mockIntStateId << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid deviceId") << DeviceId("094f8024-5caa-48c1-ab6a-de486a92088f") << mockIntStateId << DeviceManager::DeviceErrorDeviceNotFound;
    QTest::newRow("invalid statetypeId") << m_mockDeviceId << StateTypeId("120514f1-343e-4621-9bff-dac616169df9") << DeviceManager::DeviceErrorStateTypeNotFound;
}

void TestDevices::getStateValue()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(DeviceManager::DeviceError, statusCode);

    QVariantMap params;
    params.insert("deviceId", deviceId);
    params.insert("stateTypeId", stateTypeId);
    QVariant response = injectAndWait("Devices.GetStateValue", params);

    QCOMPARE(response.toMap().value("params").toMap().value("deviceError").toString(), JsonTypes::deviceErrorToString(statusCode));
    if (statusCode == DeviceManager::DeviceErrorNoError) {
        QVariant value = response.toMap().value("params").toMap().value("value");
        QCOMPARE(value.toInt(), 10); // Mock device has value 10 by default...
    }
}

void TestDevices::getStateValues_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<DeviceManager::DeviceError>("statusCode");

    QTest::newRow("valid deviceId") << m_mockDeviceId << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid deviceId") << DeviceId("094f8024-5caa-48c1-ab6a-de486a92088f") << DeviceManager::DeviceErrorDeviceNotFound;
}

void TestDevices::getStateValues()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(DeviceManager::DeviceError, statusCode);

    QVariantMap params;
    params.insert("deviceId", deviceId);
    QVariant response = injectAndWait("Devices.GetStateValues", params);

    QCOMPARE(response.toMap().value("params").toMap().value("deviceError").toString(), JsonTypes::deviceErrorToString(statusCode));
    if (statusCode == DeviceManager::DeviceErrorNoError) {
        QVariantList values = response.toMap().value("params").toMap().value("values").toList();
        QCOMPARE(values.count(), 2); // Mock device has two states...
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
    httpportParam.insert("paramTypeId", httpportParamTypeId);
    httpportParam.insert("value", 8889);
    deviceParams.append(httpportParam);

    QVariantMap params;
    params.insert("deviceClassId", mockDeviceClassId);
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

void TestDevices::reconfigureDevices_data()
{
    QVariantList asyncChangeDeviceParams;
    QVariantMap asyncParamDifferent;
    asyncParamDifferent.insert("paramTypeId", asyncParamTypeId);
    asyncParamDifferent.insert("value", true);
    asyncChangeDeviceParams.append(asyncParamDifferent);

    QVariantList httpportChangeDeviceParams;
    QVariantMap httpportParamDifferent;
    httpportParamDifferent.insert("paramTypeId", httpportParamTypeId);
    httpportParamDifferent.insert("value", 8893); // if change -> change also newPort in reconfigureDevices()
    httpportChangeDeviceParams.append(httpportParamDifferent);

    QVariantList brokenChangedDeviceParams;
    QVariantMap brokenParamDifferent;
    brokenParamDifferent.insert("paramTypeId", brokenParamTypeId);
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
    QTest::addColumn<DeviceManager::DeviceError>("deviceError");

    QTest::newRow("valid - change async param") << false << asyncChangeDeviceParams << DeviceManager::DeviceErrorParameterNotWritable;
    QTest::newRow("valid - change httpport param") << false <<  httpportChangeDeviceParams << DeviceManager::DeviceErrorNoError;
    QTest::newRow("invalid - change httpport and async param") << false << asyncAndPortChangeDeviceParams << DeviceManager::DeviceErrorParameterNotWritable;
    QTest::newRow("invalid - change all params (except broken)") << false << changeAllWritableDeviceParams << DeviceManager::DeviceErrorParameterNotWritable;
}

void TestDevices::reconfigureDevices()
{
    QFETCH(bool, broken);
    QFETCH(QVariantList, newDeviceParams);
    QFETCH(DeviceManager::DeviceError, deviceError);

    // add device
    QVariantMap params;
    params.insert("deviceClassId", mockDeviceClassId);
    params.insert("name", "Device to edit");
    QVariantList deviceParams;
    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", asyncParamTypeId);
    asyncParam.insert("value", false);
    deviceParams.append(asyncParam);
    QVariantMap brokenParam;
    brokenParam.insert("paramTypeId", brokenParamTypeId);
    brokenParam.insert("value", broken);
    deviceParams.append(brokenParam);
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", httpportParamTypeId);
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

    // if the edit should have been successfull
    if (deviceError == DeviceManager::DeviceErrorNoError) {
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
    QTest::addColumn<DeviceClassId>("deviceClassId");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<DeviceManager::DeviceError>("error");
    QTest::addColumn<QVariantList>("discoveryParams");

    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", resultCountParamTypeId);
    resultCountParam.insert("value", 2);
    discoveryParams.append(resultCountParam);

    QTest::newRow("discover 2 devices with params") << mockDeviceClassId << 2 << DeviceManager::DeviceErrorNoError << discoveryParams;
}

void TestDevices::reconfigureByDiscovery()
{
    QFETCH(DeviceClassId, deviceClassId);
    QFETCH(int, resultCount);
    QFETCH(DeviceManager::DeviceError, error);
    QFETCH(QVariantList, discoveryParams);

    QVariantMap params;
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response);
    if (error == DeviceManager::DeviceErrorNoError) {
        QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), resultCount);
    }

    // add Discovered Device 1 port 55555
    QVariantList deviceDescriptors = response.toMap().value("params").toMap().value("deviceDescriptors").toList();

    DeviceDescriptorId descriptorId1;
    foreach (const QVariant &descriptor, deviceDescriptors) {
        // find the device with port 55555
        if (descriptor.toMap().value("description").toString() == "55555") {
            descriptorId1 = DeviceDescriptorId(descriptor.toMap().value("id").toString());
            qDebug() << descriptorId1.toString();
            break;
        }
    }

    qDebug() << "adding descriptorId 1" << descriptorId1;

    QVERIFY(!descriptorId1.isNull());

    params.clear();
    response.clear();
    params.insert("deviceClassId", deviceClassId);
    params.insert("name", "Discoverd mock device");
    params.insert("deviceDescriptorId", descriptorId1);
    response = injectAndWait("Devices.AddConfiguredDevice", params);

    DeviceId deviceId(response.toMap().value("params").toMap().value("deviceId").toString());
    QVERIFY(!deviceId.isNull());

    // and now rediscover, and edit the first device with the second
    params.clear();
    response.clear();
    params.insert("deviceClassId", deviceClassId);
    params.insert("discoveryParams", discoveryParams);
    response = injectAndWait("Devices.GetDiscoveredDevices", params);

    verifyDeviceError(response, error);
    if (error == DeviceManager::DeviceErrorNoError) {
        QCOMPARE(response.toMap().value("params").toMap().value("deviceDescriptors").toList().count(), resultCount);
    }

    // get the second device
    DeviceDescriptorId descriptorId2;
    foreach (const QVariant &descriptor, deviceDescriptors) {
        // find the device with port 55556
        if (descriptor.toMap().value("description").toString() == "55556") {
            descriptorId2 = DeviceDescriptorId(descriptor.toMap().value("id").toString());
            break;
        }
    }
    QVERIFY(!descriptorId2.isNull());

    qDebug() << "edit device 1 (55555) with descriptor 2 (55556) " << descriptorId2;

    // EDIT
    response.clear();
    params.clear();
    params.insert("deviceId", deviceId.toString());
    params.insert("deviceDescriptorId", descriptorId2);
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
    QCOMPARE(deviceMap.value("id").toString(), deviceId.toString());
    if (deviceMap.contains("setupComplete"))
        QVERIFY2(deviceMap.value("setupComplete").toBool(), "Setup not completed after edit");

    // Note: this shows that by discovery a not editable param (name) can be changed!
    foreach (QVariant param, deviceMap.value("params").toList()) {
        if (param.toMap().value("paramTypeId") == httpportParamTypeId) {
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

    // check if the daemon is realy running on the new port
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


void TestDevices::removeDevice_data()
{
    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<DeviceManager::DeviceError>("deviceError");

    QTest::newRow("Existing Device") << m_mockDeviceId << DeviceManager::DeviceErrorNoError;
    QTest::newRow("Not existing Device") << DeviceId::createDeviceId() << DeviceManager::DeviceErrorDeviceNotFound;
}

void TestDevices::removeDevice()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(DeviceManager::DeviceError, deviceError);

    GuhSettings settings(GuhSettings::SettingsRoleDevices);
    settings.beginGroup("DeviceConfig");
    if (deviceError == DeviceManager::DeviceErrorNoError) {
        settings.beginGroup(m_mockDeviceId.toString());
        // Make sure we have some config values for this device
        QVERIFY(settings.allKeys().count() > 0);
    }

    QVariantMap params;
    params.insert("deviceId", deviceId);

    QVariant response = injectAndWait("Devices.RemoveConfiguredDevice", params);

    verifyDeviceError(response, deviceError);

    if (DeviceManager::DeviceErrorNoError) {
        // Make sure the device is gone from settings too
        QCOMPARE(settings.allKeys().count(), 0);
    }
}

#include "testdevices.moc"
QTEST_MAIN(TestDevices)

