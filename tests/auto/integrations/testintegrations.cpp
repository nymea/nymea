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
#include "jsonrpc/integrationshandler.h"

using namespace nymeaserver;

class TestIntegrations : public NymeaTestBase
{
    Q_OBJECT

private:
    ThingId m_mockThingAsyncId;

    inline void verifyThingError(const QVariant &response, Thing::ThingError error = Thing::ThingErrorNoError) {
        verifyError(response, "thingError", enumValueName(error));
    }

private slots:

    void initTestCase();

    void getPlugins();

    void getPluginConfig_data();
    void getPluginConfig();

    void setPluginConfig_data();
    void setPluginConfig();

    void getSupportedVendors();

    void getThingClasses_data();
    void getThingClasses();

    void verifyInterfaces();

    void addThing_data();
    void addThing();

    void thingAddedRemovedNotifications();

    void thingChangedNotifications();

    void getThings();

    void getThing_data();
    void getThing();

    void storedThings();

    void discoverThings_data();
    void discoverThings();

    void addPushButtonThings_data();
    void addPushButtonThings();

    void addDisplayPinThings_data();
    void addDisplayPinThings();

    void parentChildThings();

    void getActionTypes_data();
    void getActionTypes();

    void getEventTypes_data();
    void getEventTypes();

    void getStateTypes_data();
    void getStateTypes();

    void getStateValue_data();
    void getStateValue();

    void getStateValues_data();
    void getStateValues();

    void editThings_data();
    void editThings();

    void testThingSettings();

    void reconfigureThings_data();
    void reconfigureThings();

    void reconfigureByDiscovery_data();
    void reconfigureByDiscovery();

    void reconfigureByDiscoveryAndPair();
    void reconfigureAutoThing();

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

    void testTranslations();

    // Keep those at last as they will remove things
    void removeThing_data();
    void removeThing();

    void removeAutoThing();

    void discoverThingsParenting();
};

void TestIntegrations::initTestCase()
{
    NymeaTestBase::initTestCase();
    QLoggingCategory::setFilterRules("*.debug=false\n"
                                     "Tests.debug=true\n"
                                     "Mock.debug=true\n"
                                     "Translations.debug=true\n"
                                     );

    // Adding an async mock to be used in tests below
    QVariantMap params;
    params.insert("thingClassId", mockThingClassId);
    params.insert("name", "Mocked Thing (Async)");

    QVariantList thingParams;

    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", mockThingAsyncParamTypeId);
    asyncParam.insert("value", true);
    thingParams.append(asyncParam);

    QVariantMap httpParam;
    httpParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpParam.insert("value", 8765);
    thingParams.append(httpParam);

    params.insert("thingParams", thingParams);

    QVariant response = injectAndWait("Integrations.AddThing", params);

    m_mockThingAsyncId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY2(!m_mockThingAsyncId.isNull(), "Creating an async mock failed");

    qCDebug(dcTests()) << "Created Async mock with ID" << m_mockThingAsyncId;
}

void TestIntegrations::getPlugins()
{
    QVariant response = injectAndWait("Integrations.GetPlugins");

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

void TestIntegrations::getPluginConfig_data()
{
    QTest::addColumn<PluginId>("pluginId");
    QTest::addColumn<Thing::ThingError>("error");

    QTest::newRow("valid plugin") << mockPluginId << Thing::ThingErrorNoError;
    QTest::newRow("invalid plugin") << PluginId::createPluginId() << Thing::ThingErrorPluginNotFound;
}

void TestIntegrations::getPluginConfig()
{
    QFETCH(PluginId, pluginId);
    QFETCH(Thing::ThingError, error);

    QVariantMap params;
    params.insert("pluginId", pluginId);
    QVariant response = injectAndWait("Integrations.GetPluginConfiguration", params);
    verifyThingError(response, error);
}

void TestIntegrations::setPluginConfig_data()
{
    QTest::addColumn<PluginId>("pluginId");
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<Thing::ThingError>("error");

    QTest::newRow("valid") << mockPluginId << QVariant(13) << Thing::ThingErrorNoError;
    QTest::newRow("invalid plugin") << PluginId::createPluginId() << QVariant(13) <<  Thing::ThingErrorPluginNotFound;
    QTest::newRow("too big") << mockPluginId << QVariant(130) << Thing::ThingErrorInvalidParameter;
    QTest::newRow("too small") << mockPluginId << QVariant(-13) << Thing::ThingErrorInvalidParameter;
    QTest::newRow("wrong type") << mockPluginId << QVariant("wrontType") << Thing::ThingErrorInvalidParameter;
}

void TestIntegrations::setPluginConfig()
{
    QFETCH(PluginId, pluginId);
    QFETCH(QVariant, value);
    QFETCH(Thing::ThingError, error);

    QVariantMap params;
    params.insert("pluginId", pluginId);

    QVariantList configuration;
    QVariantMap configParam;
    configParam.insert("paramTypeId", mockPluginConfigParamIntParamTypeId);
    configParam.insert("value", value);
    configuration.append(configParam);
    params.insert("configuration", configuration);
    QVariant response = injectAndWait("Integrations.SetPluginConfiguration", params);
    verifyThingError(response, error);

    if (error == Thing::ThingErrorNoError) {
        params.clear();
        params.insert("pluginId", pluginId);
        response = injectAndWait("Integrations.GetPluginConfiguration", params);
        verifyThingError(response);
        qDebug() << value << response.toMap().value("params").toMap().value("configuration").toList().first();
        QVERIFY2(ParamTypeId(response.toMap().value("params").toMap().value("configuration").toList().first().toMap().value("paramTypeId").toString()) == mockPluginConfigParamIntParamTypeId, "Value not set correctly");
        QVERIFY2(response.toMap().value("params").toMap().value("configuration").toList().first().toMap().value("value") == value, "Value not set correctly");
    }
}

void TestIntegrations::getSupportedVendors()
{
    QVariant supportedVendors = injectAndWait("Integrations.GetVendors");
    qDebug() << "response" << supportedVendors;

    // Make sure there is exactly 1 Vendor with nymea's id
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

void TestIntegrations::getThingClasses_data()
{
    QTest::addColumn<VendorId>("vendorId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("vendor nymea") << nymeaVendorId << 14;
    QTest::newRow("no filter") << VendorId() << 14;
    QTest::newRow("invalid vendor") << VendorId("93e7d361-8025-4354-b17e-b68406c800bc") << 0;
}

void TestIntegrations::getThingClasses()
{
    QFETCH(VendorId, vendorId);
    QFETCH(int, resultCount);

    QVariantMap params;
    if (!vendorId.isNull()) {
        params.insert("vendorId", vendorId);
    }
    QVariant result = injectAndWait("Integrations.GetThingClasses", params);

    QVariantList thingClasses = result.toMap().value("params").toMap().value("thingClasses").toList();
    // Make sure there are the right amount of thing classes
    QCOMPARE(thingClasses.count(), resultCount);
}

void TestIntegrations::verifyInterfaces()
{
    QVariantMap params;
    params.insert("vendorId", nymeaVendorId);
    QVariant result = injectAndWait("Integrations.GetThingClasses", params);
    QVariantList supportedThings = result.toMap().value("params").toMap().value("thingClasses").toList();

    QVariantMap mock;
    foreach (const QVariant &thingClass, supportedThings) {
        if (thingClass.toMap().value("id").toUuid() == mockThingClassId) {
            mock = thingClass.toMap();
        }
    }
    QVERIFY(!mock.isEmpty());

    QVariantList interfaces = mock.value("interfaces").toList();
    // Must contain system, power, light and battery, but must not contain gateway as the thing manager should filter
    // that away because it doesn't implement all the required states.
    QCOMPARE(interfaces.count(), 4);
    QVERIFY(interfaces.contains("system"));
    QVERIFY(interfaces.contains("battery"));
    QVERIFY(interfaces.contains("power"));
    QVERIFY(interfaces.contains("light"));
    QVERIFY(!interfaces.contains("gateway"));
}

void TestIntegrations::addThing_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<QVariantList>("thingParams");
    QTest::addColumn<bool>("jsonValidation");
    QTest::addColumn<Thing::ThingError>("thingError");

    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId.toString());
    httpportParam.insert("value", m_mockThing1Port - 1);
    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", mockThingAsyncParamTypeId);
    asyncParam.insert("value", true);
    QVariantMap brokenParam;
    brokenParam.insert("paramTypeId", mockThingBrokenParamTypeId);
    brokenParam.insert("value", true);

    QVariantList thingParams;

    thingParams.clear(); thingParams << httpportParam;
    QTest::newRow("User, JustAdd") << mockThingClassId << thingParams << true << Thing::ThingErrorNoError;
    thingParams.clear(); thingParams << httpportParam << asyncParam;
    QTest::newRow("User, JustAdd, Async") << mockThingClassId << thingParams << true << Thing::ThingErrorNoError;
    QTest::newRow("Invalid ThingClassId") << ThingClassId::createThingClassId() << thingParams << true << Thing::ThingErrorThingClassNotFound;
    thingParams.clear(); thingParams << httpportParam << brokenParam;
    QTest::newRow("Setup failure") << mockThingClassId << thingParams << true << Thing::ThingErrorSetupFailed;
    thingParams.clear(); thingParams << httpportParam << asyncParam << brokenParam;
    QTest::newRow("Setup failure, Async") << mockThingClassId << thingParams << true << Thing::ThingErrorSetupFailed;

    QVariantList invalidThingParams;
    QTest::newRow("User, JustAdd, missing params") << mockThingClassId << invalidThingParams << true << Thing::ThingErrorMissingParameter;

    QVariantMap fakeparam;
    fakeparam.insert("paramTypeId", ParamTypeId::createParamTypeId());
    invalidThingParams.append(fakeparam);
    QTest::newRow("User, JustAdd, invalid param") << mockThingClassId << invalidThingParams << false << Thing::ThingErrorMissingParameter;

    QVariantMap fakeparam2;
    fakeparam2.insert("paramTypeId", mockThingHttpportParamTypeId.toString());
    fakeparam2.insert("value", "blabla");
    invalidThingParams.clear();
    invalidThingParams.append(fakeparam2);
    QTest::newRow("User, JustAdd, wrong param") << mockThingClassId << invalidThingParams << true << Thing::ThingErrorInvalidParameter;

    thingParams.clear(); thingParams << httpportParam << fakeparam;
    QTest::newRow("USer, JustAdd, additional invalid param") << mockThingClassId << thingParams << false << Thing::ThingErrorInvalidParameter;

    thingParams.clear(); thingParams << httpportParam << fakeparam2;
    QTest::newRow("USer, JustAdd, duplicate param") << mockThingClassId << thingParams << true << Thing::ThingErrorInvalidParameter;

}

void TestIntegrations::addThing()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(QVariantList, thingParams);
    QFETCH(bool, jsonValidation);
    QFETCH(Thing::ThingError, thingError);

    QVariantMap params;
    params.insert("thingClassId", thingClassId);
    params.insert("name", "Test Add Thing");
    params.insert("thingParams", thingParams);
    QVariant response = injectAndWait("Integrations.AddThing", params);

    if (!jsonValidation) {
        QCOMPARE(response.toMap().value("status").toString(), QString("error"));
        return;
    }
    verifyThingError(response, thingError);

    if (thingError == Thing::ThingErrorNoError) {
        QUuid thingId(response.toMap().value("params").toMap().value("thingId").toString());
        params.clear();
        params.insert("thingId", thingId.toString());
        response = injectAndWait("Integrations.RemoveThing", params);
        verifyThingError(response);
    }
}

void TestIntegrations::thingAddedRemovedNotifications()
{
    enableNotifications({"Integrations"});

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // add thing and wait for notification
    QVariantList thingParams;
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParam.insert("value", 5678);
    thingParams.append(httpportParam);

    QVariantMap params; clientSpy.clear();
    params.insert("thingClassId", mockThingClassId);
    params.insert("name", "Mocked thing");
    params.insert("thingParams", thingParams);
    QVariant response = injectAndWait("Integrations.AddThing", params);
    if (clientSpy.count() == 0) clientSpy.wait();
    verifyThingError(response);
    QVariantMap notificationThingMap = checkNotification(clientSpy, "Integrations.ThingAdded").toMap().value("params").toMap().value("thing").toMap();

    ThingId thingId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY(!thingId.isNull());

    // check the ThingAdded notification
    QCOMPARE(notificationThingMap.value("thingClassId").toUuid(), QUuid(mockThingClassId));
    QCOMPARE(notificationThingMap.value("id").toUuid(), QUuid(thingId));
    foreach (const QVariant &param, notificationThingMap.value("params").toList()) {
        if (param.toMap().value("name").toString() == "httpport") {
            QCOMPARE(param.toMap().value("value").toInt(), httpportParam.value("value").toInt());
        }
    }

    // now remove the thong and check the thing removed notification
    params.clear(); response.clear(); clientSpy.clear();
    params.insert("thingId", thingId);
    response = injectAndWait("Integrations.RemoveThing", params);
    if (clientSpy.count() == 0) clientSpy.wait();
    verifyThingError(response);
    checkNotification(clientSpy, "Integrations.ThingRemoved");

    QCOMPARE(disableNotifications(), true);
}

void TestIntegrations::thingChangedNotifications()
{
    enableNotifications({"Integrations"});

    // Setup connection to mock client
    QSignalSpy clientSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // ADD
    // add thing and wait for notification
    QVariantList thingParams;
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParam.insert("value", 23234);
    thingParams.append(httpportParam);

    clientSpy.clear();
    QVariantMap params;
    params.insert("thingClassId", mockThingClassId);
    params.insert("name", "Mock");
    params.insert("thingParams", thingParams);
    QVariant response = injectAndWait("Integrations.AddThing", params);
    ThingId thingId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY(!thingId.isNull());
    if (clientSpy.count() == 0) clientSpy.wait();
    verifyThingError(response);
    QVariantMap notificationThingMap = checkNotification(clientSpy, "Integrations.ThingAdded").toMap().value("params").toMap().value("thing").toMap();

    QCOMPARE(notificationThingMap.value("thingClassId").toUuid(), QUuid(mockThingClassId));
    QCOMPARE(notificationThingMap.value("id").toUuid(), QUuid(thingId));
    foreach (const QVariant &param, notificationThingMap.value("params").toList()) {
        if (param.toMap().value("name").toString() == "httpport") {
            QCOMPARE(param.toMap().value("value").toInt(), httpportParam.value("value").toInt());
        }
    }

    // RECONFIGURE
    // now reconfigure the thing and check the thing changed notification
    QVariantList newThingParams;
    QVariantMap newHttpportParam;
    newHttpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    newHttpportParam.insert("value", 45473);
    newThingParams.append(newHttpportParam);

    params.clear(); response.clear(); clientSpy.clear();
    params.insert("thingId", thingId);
    params.insert("thingParams", newThingParams);
    response = injectAndWait("Integrations.ReconfigureThing", params);
    if (clientSpy.count() == 0) clientSpy.wait();
    verifyThingError(response);
    QVariantMap reconfigureThingNotificationMap = checkNotification(clientSpy, "Integrations.ThingChanged").toMap().value("params").toMap().value("thing").toMap();
    QCOMPARE(reconfigureThingNotificationMap.value("thingClassId").toUuid(), QUuid(mockThingClassId));
    QCOMPARE(reconfigureThingNotificationMap.value("id").toUuid(), QUuid(thingId));
    foreach (const QVariant &param, reconfigureThingNotificationMap.value("params").toList()) {
        if (param.toMap().value("name").toString() == "httpport") {
            QCOMPARE(param.toMap().value("value").toInt(), newHttpportParam.value("value").toInt());
        }
    }

    // EDIT thing name
    QString thingName = "Test thing 1234";
    params.clear(); response.clear(); clientSpy.clear();
    params.insert("thingId", thingId);
    params.insert("name", thingName);
    response = injectAndWait("Integrations.EditThing", params);
    if (clientSpy.count() == 0) clientSpy.wait();
    verifyThingError(response);
    QVariantMap editThingNotificationMap = checkNotification(clientSpy, "Integrations.ThingChanged").toMap().value("params").toMap().value("thing").toMap();
    QCOMPARE(editThingNotificationMap.value("thingClassId").toUuid(), QUuid(mockThingClassId));
    QCOMPARE(editThingNotificationMap.value("id").toUuid(), QUuid(thingId));
    QCOMPARE(editThingNotificationMap.value("name").toString(), thingName);

    // REMOVE
    // now remove the thing and check the thing removed notification
    params.clear(); response.clear(); clientSpy.clear();
    params.insert("thingId", thingId);
    response = injectAndWait("Integrations.RemoveThing", params);
    if (clientSpy.count() == 0) clientSpy.wait();
    verifyThingError(response);
    checkNotification(clientSpy, "Integrations.ThingRemoved");
    checkNotification(clientSpy, "Logging.LogDatabaseUpdated");
}

void TestIntegrations::getThings()
{
    QVariant response = injectAndWait("Integrations.GetThings");

    QVariantList things = response.toMap().value("params").toMap().value("things").toList();
    QCOMPARE(things.count(), 3); // There should be: one auto created mock, one created in NymeaTestBase::initTestcase() and one created in TestIntegrations::initTestCase()
}

void TestIntegrations::getThing_data()
{
    QTest::addColumn<ThingId>("thingId");
    QTest::addColumn<Thing::ThingError>("expectedError");

    QTest::newRow("valid thingId") << ThingId(m_mockThingId) << Thing::ThingErrorNoError;
    QTest::newRow("invalid thingId") << ThingId::createThingId() << Thing::ThingErrorThingNotFound;
}

void TestIntegrations::getThing()
{
    QFETCH(ThingId, thingId);
    QFETCH(Thing::ThingError, expectedError);

    QVariantMap params;
    params.insert("thingId", thingId);
    QVariant response = injectAndWait("Integrations.GetThings", params);

//    qCDebug(dcTests()) << qUtf8Printable(QJsonDocument::fromVariant(response).toJson());

    if (expectedError == Thing::ThingErrorNoError) {
        QVariantList things = response.toMap().value("params").toMap().value("things").toList();
        QCOMPARE(things.count(), 1);
    }
}

void TestIntegrations::storedThings()
{
    QVariantMap params;
    params.insert("thingClassId", mockThingClassId);
    params.insert("name", "Test stored thing");
    QVariantList thingParams;
    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", mockThingAsyncParamTypeId);
    asyncParam.insert("value", false);
    thingParams.append(asyncParam);
    QVariantMap brokenParam;
    brokenParam.insert("paramTypeId", mockThingBrokenParamTypeId);
    brokenParam.insert("value", false);
    thingParams.append(brokenParam);
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParam.insert("value", 8889);
    thingParams.append(httpportParam);
    params.insert("thingParams", thingParams);

    QVariant response = injectAndWait("Integrations.AddThing", params);
    verifyThingError(response);
    ThingId addedThingId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY(!addedThingId.isNull());

    // Restart the core instance to check if settings are loaded at startup
    restartServer();

    response = injectAndWait("Integrations.GetThings", QVariantMap());

    bool found = false;
    foreach (const QVariant &thing, response.toMap().value("params").toMap().value("things").toList()) {
        if (ThingId(thing.toMap().value("id").toString()) == addedThingId) {
            qDebug() << "found added thing" << thing.toMap().value("params");
            qDebug() << "expected thingParams:" << thingParams;
            verifyParams(thingParams, thing.toMap().value("params").toList());
            found = true;
            break;
        }
    }
    QVERIFY2(found, "thing missing in config!");

    params.clear();
    params.insert("thingId", addedThingId);
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response);
}

void TestIntegrations::discoverThings_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<Thing::ThingError>("error");
    QTest::addColumn<QVariantList>("discoveryParams");

    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", mockDiscoveryResultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QTest::newRow("valid thingClassId") << mockThingClassId << 2 << Thing::ThingErrorNoError << QVariantList();
    QTest::newRow("valid thingClassId with params") << mockThingClassId << 1 << Thing::ThingErrorNoError << discoveryParams;
    QTest::newRow("invalid thingClassId") << ThingClassId::createThingClassId() << 0 << Thing::ThingErrorThingClassNotFound << QVariantList();
}

void TestIntegrations::discoverThings()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(int, resultCount);
    QFETCH(Thing::ThingError, error);
    QFETCH(QVariantList, discoveryParams);

    QVariantMap params;
    params.insert("thingClassId", thingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Integrations.DiscoverThings", params);

    verifyThingError(response, error);
    if (error == Thing::ThingErrorNoError) {
        QCOMPARE(response.toMap().value("params").toMap().value("thingDescriptors").toList().count(), resultCount);
    }

    // If we found something, lets try to add it
    if (error == Thing::ThingErrorNoError) {
        ThingDescriptorId descriptorId = ThingDescriptorId(response.toMap().value("params").toMap().value("thingDescriptors").toList().first().toMap().value("id").toString());

        params.clear();
        params.insert("thingClassId", thingClassId);
        params.insert("name", "Discoverd mock");
        params.insert("thingDescriptorId", descriptorId.toString());
        response = injectAndWait("Integrations.AddThing", params);

        verifyThingError(response);

        ThingId thingId(response.toMap().value("params").toMap().value("thingId").toString());
        params.clear();
        params.insert("thingId", thingId.toString());
        response = injectAndWait("Integrations.RemoveThing", params);
        verifyThingError(response);
    }
}

void TestIntegrations::addPushButtonThings_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<Thing::ThingError>("error");
    QTest::addColumn<bool>("waitForButtonPressed");

    QTest::newRow("Valid: Add PushButton thing") << pushButtonMockThingClassId << Thing::ThingErrorNoError << true;
    QTest::newRow("Invalid: Add PushButton thing (press to early)") << pushButtonMockThingClassId << Thing::ThingErrorAuthenticationFailure << false;
}

void TestIntegrations::addPushButtonThings()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(Thing::ThingError, error);
    QFETCH(bool, waitForButtonPressed);

    // Discover things
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", pushButtonMockDiscoveryResultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QVariantMap params;
    params.insert("thingClassId", thingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Integrations.DiscoverThings", params);

    verifyThingError(response, Thing::ThingErrorNoError);
    QCOMPARE(response.toMap().value("params").toMap().value("thingDescriptors").toList().count(), 1);


    // Pair thing
    ThingDescriptorId descriptorId = ThingDescriptorId(response.toMap().value("params").toMap().value("thingDescriptors").toList().first().toMap().value("id").toString());
    params.clear();
    params.insert("thingClassId", thingClassId);
    params.insert("name", "Pushbutton mock");
    params.insert("thingDescriptorId", descriptorId.toString());
    response = injectAndWait("Integrations.PairThing", params);

    verifyThingError(response);

    PairingTransactionId pairingTransactionId(response.toMap().value("params").toMap().value("pairingTransactionId").toString());
    QString displayMessage = response.toMap().value("params").toMap().value("displayMessage").toString();

    qDebug() << "displayMessage" << displayMessage;

    if (waitForButtonPressed)
        QTest::qWait(3500);

    // Confirm pairing
    params.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());
    response = injectAndWait("Integrations.ConfirmPairing", params);

    verifyThingError(response, error);

    if (error == Thing::ThingErrorNoError) {
        ThingId thingId(response.toMap().value("params").toMap().value("thingId").toString());
        params.clear();
        params.insert("thingId", thingId.toString());
        response = injectAndWait("Integrations.RemoveThing", params);
        verifyThingError(response);
    }
}

void TestIntegrations::addDisplayPinThings_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<Thing::ThingError>("error");
    QTest::addColumn<QString>("secret");

    QTest::newRow("Valid: Add DisplayPin mock") << displayPinMockThingClassId << Thing::ThingErrorNoError << "243681";
    QTest::newRow("Invalid: Add DisplayPin mock (wrong pin)") << displayPinMockThingClassId << Thing::ThingErrorAuthenticationFailure << "243682";
}

void TestIntegrations::addDisplayPinThings()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(Thing::ThingError, error);
    QFETCH(QString, secret);

    // Discover things
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", displayPinMockDiscoveryResultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    QVariantMap params;
    params.insert("thingClassId", thingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Integrations.DiscoverThings", params);

    verifyThingError(response, Thing::ThingErrorNoError);
    QCOMPARE(response.toMap().value("params").toMap().value("thingDescriptors").toList().count(), 1);

    // Pair thing
    ThingDescriptorId descriptorId = ThingDescriptorId(response.toMap().value("params").toMap().value("thingDescriptors").toList().first().toMap().value("id").toString());
    params.clear();
    params.insert("thingClassId", thingClassId);
    params.insert("name", "Display pin mock");
    params.insert("thingDescriptorId", descriptorId.toString());
    response = injectAndWait("Integrations.PairThing", params);

    verifyThingError(response);

    PairingTransactionId pairingTransactionId(response.toMap().value("params").toMap().value("pairingTransactionId").toString());
    QString displayMessage = response.toMap().value("params").toMap().value("displayMessage").toString();

    qCDebug(dcTests()) << "displayMessage" << displayMessage;

    params.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());
    params.insert("secret", secret);
    response = injectAndWait("Integrations.ConfirmPairing", params);

    verifyThingError(response, error);

    if (error == Thing::ThingErrorNoError) {
        ThingId thingId(response.toMap().value("params").toMap().value("thingId").toString());
        params.clear();
        params.insert("thingId", thingId.toString());
        response = injectAndWait("Integrations.RemoveThing", params);
        verifyThingError(response);
    }
}

void TestIntegrations::parentChildThings()
{
    // add parent
    QVariantMap params;
    params.insert("thingClassId", parentMockThingClassId);
    params.insert("name", "Parent");

    QSignalSpy thingAddedSpy(NymeaCore::instance()->thingManager(), &ThingManager::thingAdded);

    QVariant response = injectAndWait("Integrations.AddThing", params);
    verifyThingError(response);

    ThingId parentId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY(!parentId.isNull());

    thingAddedSpy.wait();
    QCOMPARE(thingAddedSpy.count(), 2);

    // find child
    response = injectAndWait("Integrations.GetThings");

    QVariantList things = response.toMap().value("params").toMap().value("things").toList();

    ThingId childId;
    foreach (const QVariant thingVariant, things) {
        QVariantMap thingMap = thingVariant.toMap();

        if (thingMap.value("thingClassId").toUuid() == childMockThingClassId) {
            if (thingMap.value("parentId").toUuid() == parentId) {
                childId = ThingId(thingMap.value("id").toString());
                break;
            }
        }
    }
    QVERIFY2(!childId.isNull(), QString("Could not find child:\nParent ID:%1\nResponse:%2")
             .arg(parentId.toString())
             .arg(qUtf8Printable(QJsonDocument::fromVariant(response).toJson()))
             .toUtf8());

    // Try to remove the child
    params.clear();
    params.insert("thingId", childId.toString());
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response, Thing::ThingErrorThingIsChild);

    // check if the child is still there
    response = injectAndWait("Integrations.GetThings");
    things = response.toMap().value("params").toMap().value("things").toList();
    bool found = false;
    foreach (const QVariant thingVariant, things) {
        QVariantMap thingMap = thingVariant.toMap();
        if (thingMap.value("thingClassId").toUuid() == childMockThingClassId) {
            if (thingMap.value("id").toUuid() == childId) {
                found = true;
                break;
            }
        }
    }
    QVERIFY2(found, "Could not find child.");

    // remove the parent
    params.clear();
    params.insert("thingId", parentId.toString());
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response);

    // check if the child is still there
    response = injectAndWait("Integrations.GetThings");
    things = response.toMap().value("params").toMap().value("things").toList();
    found = false;
    foreach (const QVariant thingVariant, things) {
        QVariantMap thingMap = thingVariant.toMap();
        if (thingMap.value("thingClassId").toString() == childMockThingClassId.toString()) {
            if (thingMap.value("id") == childId.toString()) {
                found = true;
                break;
            }
        }
    }
    QVERIFY2(!found, "Could not find child.");
}

void TestIntegrations::getActionTypes_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<QList<ActionTypeId> >("actionTypeTestData");

    QTest::newRow("valid thingClass") << mockThingClassId
                                       << (QList<ActionTypeId>() << mockAsyncActionTypeId << mockAsyncFailingActionTypeId << mockFailingActionTypeId << mockWithoutParamsActionTypeId << mockPowerActionTypeId << mockWithoutParamsActionTypeId);
    QTest::newRow("invalid thingClass") << ThingClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << QList<ActionTypeId>();
}

void TestIntegrations::getActionTypes()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(QList<ActionTypeId>, actionTypeTestData);

    QVariantMap params;
    params.insert("thingClassId", thingClassId);
    QVariant response = injectAndWait("Integrations.GetActionTypes", params);

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

void TestIntegrations::getEventTypes_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid thingClass") << mockThingClassId << 8;
    QTest::newRow("invalid thingClass") << ThingClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestIntegrations::getEventTypes()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("thingClassId", thingClassId);
    QVariant response = injectAndWait("Integrations.GetEventTypes", params);

    qDebug() << response;

    QVariantList eventTypes = response.toMap().value("params").toMap().value("eventTypes").toList();
    QCOMPARE(eventTypes.count(), resultCount);

}

void TestIntegrations::getStateTypes_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<int>("resultCount");

    QTest::newRow("valid thingClass") << mockThingClassId << 6;
    QTest::newRow("invalid thingClass") << ThingClassId("094f8024-5caa-48c1-ab6a-de486a92088f") << 0;
}

void TestIntegrations::getStateTypes()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(int, resultCount);

    QVariantMap params;
    params.insert("thingClassId", thingClassId);
    QVariant response = injectAndWait("Integrations.GetStateTypes", params);

    QVariantList stateTypes = response.toMap().value("params").toMap().value("stateTypes").toList();
    QCOMPARE(stateTypes.count(), resultCount);
    if (resultCount > 0) {
        QCOMPARE(stateTypes.first().toMap().value("id").toUuid().toString(), mockIntStateTypeId.toString());
    }
}

void TestIntegrations::getStateValue_data()
{
    QTest::addColumn<ThingId>("thingId");
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<Thing::ThingError>("statusCode");

    QTest::newRow("valid thingId") << ThingId(m_mockThingId) << mockIntStateTypeId << Thing::ThingErrorNoError;
    QTest::newRow("invalid thingId") << ThingId("094f8024-5caa-48c1-ab6a-de486a92088f") << mockIntStateTypeId << Thing::ThingErrorThingNotFound;
    QTest::newRow("invalid statetypeId") << ThingId(m_mockThingId) << StateTypeId("120514f1-343e-4621-9bff-dac616169df9") << Thing::ThingErrorStateTypeNotFound;
}

void TestIntegrations::getStateValue()
{
    QFETCH(ThingId, thingId);
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(Thing::ThingError, statusCode);

    QVariantMap params;
    params.insert("thingId", thingId);
    params.insert("stateTypeId", stateTypeId);
    QVariant response = injectAndWait("Integrations.GetStateValue", params);

    QCOMPARE(response.toMap().value("params").toMap().value("thingError").toString(), enumValueName(statusCode));
    if (statusCode == Thing::ThingErrorNoError) {
        QVariant value = response.toMap().value("params").toMap().value("value");
        QCOMPARE(value.toInt(), 10); // Mock has value 10 by default...
    }
}

void TestIntegrations::getStateValues_data()
{
    QTest::addColumn<ThingId>("thingId");
    QTest::addColumn<Thing::ThingError>("statusCode");

    QTest::newRow("valid thingId") << ThingId(m_mockThingId) << Thing::ThingErrorNoError;
    QTest::newRow("invalid thingId") << ThingId("094f8024-5caa-48c1-ab6a-de486a92088f") << Thing::ThingErrorThingNotFound;
}

void TestIntegrations::getStateValues()
{
    QFETCH(ThingId, thingId);
    QFETCH(Thing::ThingError, statusCode);

    QVariantMap params;
    params.insert("thingId", thingId);
    QVariant response = injectAndWait("Integrations.GetStateValues", params);

    QCOMPARE(response.toMap().value("params").toMap().value("thingError").toString(), enumValueName(statusCode));
    if (statusCode == Thing::ThingErrorNoError) {
        QVariantList values = response.toMap().value("params").toMap().value("values").toList();
        QCOMPARE(values.count(), 6); // Mock has 6 states...
    }
}

void TestIntegrations::editThings_data()
{
    QTest::addColumn<QString>("name");

    QTest::newRow("change name") << "New name";
    QTest::newRow("change name") << "Foo";
    QTest::newRow("change name") << "Bar";
}

void TestIntegrations::editThings()
{
    QFETCH(QString, name);

    QString originalName = "Test thing";

    // add thing
    QVariantList thingParams;
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParam.insert("value", 8889);
    thingParams.append(httpportParam);

    QVariantMap params;
    params.insert("thingClassId", mockThingClassId);
    params.insert("name", originalName);
    params.insert("thingParams", thingParams);
    QVariant response = injectAndWait("Integrations.AddThing", params);
    verifyThingError(response);
    ThingId thingId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());

    // edit thing
    params.clear();
    params.insert("thingId", thingId);
    params.insert("name", name);

    response = injectAndWait("Integrations.EditThing", params);
    verifyThingError(response);

    // verify changed
    QString newName;
    response = injectAndWait("Integrations.GetThings");
    QVariantList things = response.toMap().value("params").toMap().value("things").toList();

    foreach (const QVariant &thingVariant, things) {
        QVariantMap thing = thingVariant.toMap();
        if (ThingId(thing.value("id").toString()) == thingId) {
            newName = thing.value("name").toString();
        }
    }
    QCOMPARE(newName, name);

    restartServer();

    // check if the changed name is still there after loading
    response = injectAndWait("Integrations.GetThings");
    things = response.toMap().value("params").toMap().value("things").toList();
    foreach (const QVariant &thingVariant, things) {
        QVariantMap thing = thingVariant.toMap();
        if (ThingId(thing.value("id").toString()) == thingId) {
            newName = thing.value("name").toString();
            break;
        }
    }
    QCOMPARE(newName, name);

    params.clear();
    params.insert("thingId", thingId.toString());
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response);
}

void TestIntegrations::testThingSettings()
{
    // add thing
    QVariantList thingParams;
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParam.insert("value", 8889);
    thingParams.append(httpportParam);

    QVariantMap params;
    params.insert("thingClassId", mockThingClassId);
    params.insert("name", "Mock");
    params.insert("thingParams", thingParams);
    QVariant response = injectAndWait("Integrations.AddThing", params);
    verifyThingError(response);
    ThingId thingId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());

    // check if default settings are loaded
    params.clear();
    params.insert("thingId", thingId);
    response = injectAndWait("Integrations.GetThings", params);
    QVariantList things = response.toMap().value("params").toMap().value("things").toList();
    QVERIFY2(things.count() == 1, "Error creating thing");

    QVariantMap thing = things.first().toMap();
    QVERIFY2(ThingId(thing.value("id").toString()) == thingId, "thingId not matching");

    QVariantList settings = thing.value("settings").toList();
    QCOMPARE(settings.count(), 1);

    QCOMPARE(settings.first().toMap().value("paramTypeId").toUuid(), QUuid(mockSettingsSetting1ParamTypeId));
    QVERIFY2(settings.first().toMap().value("value").toInt() == 5, "Setting 1 default value not matching");

    // change a setting
    params.clear();
    params.insert("thingId", thingId);
    settings.clear();
    QVariantMap setting;
    setting.insert("paramTypeId", mockSettingsSetting1ParamTypeId);
    setting.insert("value", 7);
    settings.append(setting);
    params.insert("settings", settings);
    response = injectAndWait("Integrations.SetThingSettings", params);

    // Check if the change happened
    params.clear();
    params.insert("thingId", thingId);
    response = injectAndWait("Integrations.GetThings", params);
    things = response.toMap().value("params").toMap().value("things").toList();
    QVERIFY2(things.count() == 1, "Error creating thing");

    thing = things.first().toMap();
    QVERIFY2(ThingId(thing.value("id").toString()) == thingId, "thingId not matching");

    settings = thing.value("settings").toList();
    QCOMPARE(settings.count(), 1);

    QCOMPARE(settings.first().toMap().value("paramTypeId").toUuid(), QUuid(mockSettingsSetting1ParamTypeId));
    QVERIFY2(settings.first().toMap().value("value").toInt() == 7, "Setting 1 changed value not matching");

    restartServer();

    // Check if the change persisted
    params.clear();
    params.insert("thingId", thingId);
    response = injectAndWait("Integrations.GetThings", params);
    things = response.toMap().value("params").toMap().value("things").toList();
    QVERIFY2(things.count() == 1, "Error creating thing");

    thing = things.first().toMap();
    QVERIFY2(ThingId(thing.value("id").toString()) == thingId, "thingId not matching");

    settings = thing.value("settings").toList();
    QCOMPARE(settings.count(), 1);

    QCOMPARE(settings.first().toMap().value("paramTypeId").toUuid(), QUuid(mockSettingsSetting1ParamTypeId));
    QVERIFY2(settings.first().toMap().value("value").toInt() == 7, "Setting 1 changed value not persisting restart");

}

void TestIntegrations::reconfigureThings_data()
{
    QVariantList asyncChangeThingParams;
    QVariantMap asyncParamDifferent;
    asyncParamDifferent.insert("paramTypeId", mockThingAsyncParamTypeId);
    asyncParamDifferent.insert("value", true);
    asyncChangeThingParams.append(asyncParamDifferent);

    QVariantList httpportChangeThingParams;
    QVariantMap httpportParamDifferent;
    httpportParamDifferent.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParamDifferent.insert("value", 8893); // if changing this, change also newPort in reconfigureThings()
    httpportChangeThingParams.append(httpportParamDifferent);

    QVariantList brokenChangedThingParams;
    QVariantMap brokenParamDifferent;
    brokenParamDifferent.insert("paramTypeId", mockThingBrokenParamTypeId);
    brokenParamDifferent.insert("value", true);
    brokenChangedThingParams.append(brokenParamDifferent);

    QVariantList asyncAndPortChangeThingParams;
    asyncAndPortChangeThingParams.append(asyncParamDifferent);
    asyncAndPortChangeThingParams.append(httpportParamDifferent);


    QVariantList changeAllWritableThingParams;
    changeAllWritableThingParams.append(asyncParamDifferent);
    changeAllWritableThingParams.append(httpportParamDifferent);

    QTest::addColumn<bool>("broken");
    QTest::addColumn<QVariantList>("newThingParams");
    QTest::addColumn<Thing::ThingError>("thingError");

    QTest::newRow("valid - change async param") << false << asyncChangeThingParams << Thing::ThingErrorParameterNotWritable;
    QTest::newRow("valid - change httpport param") << false <<  httpportChangeThingParams << Thing::ThingErrorNoError;
    QTest::newRow("invalid - change httpport and async param") << false << asyncAndPortChangeThingParams << Thing::ThingErrorParameterNotWritable;
    QTest::newRow("invalid - change all params (except broken)") << false << changeAllWritableThingParams << Thing::ThingErrorParameterNotWritable;
}

void TestIntegrations::reconfigureThings()
{
    QFETCH(bool, broken);
    QFETCH(QVariantList, newThingParams);
    QFETCH(Thing::ThingError, thingError);

    // add thing
    QVariantMap params;
    params.insert("thingClassId", mockThingClassId);
    params.insert("name", "Thing to edit");
    QVariantList thingParams;
    QVariantMap asyncParam;
    asyncParam.insert("paramTypeId", mockThingAsyncParamTypeId);
    asyncParam.insert("value", false);
    thingParams.append(asyncParam);
    QVariantMap brokenParam;
    brokenParam.insert("paramTypeId", mockThingBrokenParamTypeId);
    brokenParam.insert("value", broken);
    thingParams.append(brokenParam);
    QVariantMap httpportParam;
    httpportParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    httpportParam.insert("value", 8892);
    thingParams.append(httpportParam);
    params.insert("thingParams", thingParams);

    // add a mock
    QVariant response = injectAndWait("Integrations.AddThing", params);
    verifyThingError(response);

    ThingId thingId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY(!thingId.isNull());

    // now EDIT the added mock
    response.clear();
    QVariantMap editParams;
    editParams.insert("thingId", thingId);
    editParams.insert("thingParams", newThingParams);
    response = injectAndWait("Integrations.ReconfigureThing", editParams);
    verifyThingError(response, thingError);

    // if the edit should have been successful
    if (thingError == Thing::ThingErrorNoError) {
        response = injectAndWait("Integrations.GetThings", QVariantMap());

        bool found = false;
        foreach (const QVariant &thing, response.toMap().value("params").toMap().value("things").toList()) {
            if (ThingId(thing.toMap().value("id").toString()) == thingId) {
                qDebug() << "found added thing" << thing.toMap().value("params");
                qDebug() << "expected thingParams:" << newThingParams;
                // check if the edit was ok
                verifyParams(newThingParams, thing.toMap().value("params").toList(), false);
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Thing missing in config!");

        // Restart the core instance to check if settings are loaded at startup
        restartServer();

        response = injectAndWait("Integrations.GetThings", QVariantMap());

        found = false;
        foreach (const QVariant &thing, response.toMap().value("params").toMap().value("things").toList()) {
            if (ThingId(thing.toMap().value("id").toString()) == thingId) {
                qDebug() << "found added thing" << thing.toMap().value("params");
                qDebug() << "expected params:" << newThingParams;
                // check if the edit was ok
                verifyParams(newThingParams, thing.toMap().value("params").toList(), false);
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Thing missing in config!");

        // delete it
        params.clear();
        params.insert("thingId", thingId);
        response.clear();
        response = injectAndWait("Integrations.RemoveThing", params);
        verifyThingError(response);
        return;
    } else {
        // The edit was not ok, check if the old params are still there
        response = injectAndWait("Integrations.GetThings", QVariantMap());

        bool found = false;
        foreach (const QVariant &thing, response.toMap().value("params").toMap().value("things").toList()) {
            if (ThingId(thing.toMap().value("id").toString()) == thingId) {
                qDebug() << "found added thing" << thing.toMap().value("params");
                qDebug() << "expected thingParams:" << newThingParams;
                // check if the params are unchanged
                verifyParams(thingParams, thing.toMap().value("params").toList());
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Thing missing in config!");

        // Restart the core instance to check if settings are loaded at startup
        restartServer();

        response = injectAndWait("Integrations.GetThings", QVariantMap());

        found = false;
        foreach (const QVariant &thing, response.toMap().value("params").toMap().value("things").toList()) {
            if (ThingId(thing.toMap().value("id").toString()) == thingId) {
                qDebug() << "found added thing" << thing.toMap().value("params");
                qDebug() << "expected thingParams:" << newThingParams;
                // check if after the reboot the settings are unchanged
                verifyParams(thingParams, thing.toMap().value("params").toList());
                found = true;
                break;
            }
        }
        QVERIFY2(found, "Thing missing in config!");
    }

    // delete it
    params.clear();
    params.insert("thingId", thingId);
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response);
}


void TestIntegrations::reconfigureByDiscovery_data()
{
    QTest::addColumn<ThingClassId>("thingClassId");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<Thing::ThingError>("error");
    QTest::addColumn<QVariantList>("discoveryParams");

    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", mockDiscoveryResultCountParamTypeId);
    resultCountParam.insert("value", 2);
    discoveryParams.append(resultCountParam);

    QTest::newRow("discover 2 things with params") << mockThingClassId << 2 << Thing::ThingErrorNoError << discoveryParams;
}

void TestIntegrations::reconfigureByDiscovery()
{
    QFETCH(ThingClassId, thingClassId);
    QFETCH(int, resultCount);
    QFETCH(Thing::ThingError, error);
    QFETCH(QVariantList, discoveryParams);

    qCDebug(dcTests()) << "Discovering...";
    QVariantMap params;
    params.insert("thingClassId", thingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Integrations.DiscoverThings", params);

    verifyThingError(response);
    if (error == Thing::ThingErrorNoError) {
        QCOMPARE(response.toMap().value("params").toMap().value("thingDescriptors").toList().count(), resultCount);
    }

    // add Discovered Thing 1 port 55555
    QVariantList thingDescriptors = response.toMap().value("params").toMap().value("thingDescriptors").toList();

    ThingDescriptorId descriptorId;
    foreach (const QVariant &descriptor, thingDescriptors) {
        // find the thing with port 55555
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
    params.insert("thingClassId", thingClassId);
    params.insert("name", "Discoverd mock");
    params.insert("thingDescriptorId", descriptorId);
    response = injectAndWait("Integrations.AddThing", params);

    ThingId thingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY(!thingId.isNull());

    // and now rediscover and find the existing thing in the discovery results
    qCDebug(dcTests()) << "Re-Discovering...";

    params.clear();
    response.clear();
    params.insert("thingClassId", thingClassId);
    params.insert("discoveryParams", discoveryParams);
    response = injectAndWait("Integrations.DiscoverThings", params);

    verifyThingError(response, error);
    if (error == Thing::ThingErrorNoError) {
        QCOMPARE(response.toMap().value("params").toMap().value("thingDescriptors").toList().count(), resultCount);
    }

    thingDescriptors = response.toMap().value("params").toMap().value("thingDescriptors").toList();

    // find the already added thing
    descriptorId = ThingDescriptorId(); // reset it first
    foreach (const QVariant &descriptor, thingDescriptors) {
        if (descriptor.toMap().value("thingId").toUuid().toString() == thingId.toString()) {
            descriptorId = ThingDescriptorId(descriptor.toMap().value("id").toString());
            break;
        }
    }
    QVERIFY2(!descriptorId.isNull(), QString("Tjhing %1 not found in discovery results: %2").arg(thingId.toString()).arg(qUtf8Printable(QJsonDocument::fromVariant(response).toJson())).toUtf8());

    qCDebug(dcTests()) << "Reconfiguring...";

    response.clear();
    params.clear();
    params.insert("thingDescriptorId", descriptorId);
    // override port param
    QVariantMap portParam;
    portParam.insert("paramTypeId", mockThingHttpportParamTypeId);
    portParam.insert("value", "55556");
    params.insert("thingParams", QVariantList() << portParam);
    response = injectAndWait("Integrations.ReconfigureThing", params);
    verifyThingError(response, error);

    response.clear();
    response = injectAndWait("Integrations.GetThings", QVariantMap());

    QVariantMap thingMap;
    bool found = false;
    foreach (const QVariant &thing, response.toMap().value("params").toMap().value("things").toList()) {
        if (ThingId(thing.toMap().value("id").toString()) == thingId) {
            qDebug() << "found added thing" << thing.toMap().value("params");
            found = true;
            thingMap = thing.toMap();
            break;
        }
    }

    QVERIFY2(found, "Thing missing in config!");
    QCOMPARE(thingMap.value("id").toUuid(), QUuid(thingId));
    if (thingMap.contains("setupComplete"))
        QVERIFY2(thingMap.value("setupComplete").toBool(), "Setup not completed after edit");

    // Note: this shows that by discovery a not editable param (name) can be changed!
    foreach (QVariant param, thingMap.value("params").toList()) {
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
    params.insert("thingId", thingId.toString());
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response);
}

void TestIntegrations::reconfigureByDiscoveryAndPair()
{
    QVariantList discoveryParams;
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", displayPinMockDiscoveryResultCountParamTypeId);
    resultCountParam.insert("value", 1);
    discoveryParams.append(resultCountParam);

    qCDebug(dcTests()) << "Discovering things...";

    QVariantMap params;
    params.insert("thingClassId", displayPinMockThingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Integrations.DiscoverThings", params);

    verifyThingError(response);
    QVariantList thingDescriptors = response.toMap().value("params").toMap().value("thingDescriptors").toList();

    qCDebug(dcTests()) << "Discovery result:" << qUtf8Printable(QJsonDocument::fromVariant(thingDescriptors).toJson(QJsonDocument::Indented));
    QCOMPARE(response.toMap().value("params").toMap().value("thingDescriptors").toList().count(), 1);

    // add Discovered thing 1 port 55555

    QVariant descriptor = thingDescriptors.first();
    ThingDescriptorId descriptorId = ThingDescriptorId(descriptor.toMap().value("id").toString());
    QVERIFY2(!descriptorId.isNull(), "ThingDescriptorId is Null");

    qCDebug(dcTests()) << "Pairing descriptorId:" << descriptorId;

    params.clear();
    response.clear();
    params.insert("thingClassId", displayPinMockThingClassId);
    params.insert("name", "Discoverd mock");
    params.insert("thingDescriptorId", descriptorId);
    response = injectAndWait("Integrations.PairThing", params);
    verifyThingError(response);

    PairingTransactionId pairingTransactionId = PairingTransactionId(response.toMap().value("params").toMap().value("pairingTransactionId").toString());
    qCDebug(dcTests()) << "PairThing result:" << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    qCDebug(dcTests()) << "Confirming pairing for transaction ID" << pairingTransactionId;
    params.clear();
    response.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());
    params.insert("secret", "243681");
    response = injectAndWait("Integrations.ConfirmPairing", params);
    verifyThingError(response);

    ThingId thingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY(!thingId.isNull());

    qCDebug(dcTests()) << "Discovering again...";

    // and now rediscover, and edit the first thing with the second
    params.clear();
    response.clear();
    params.insert("thingClassId", displayPinMockThingClassId);
    params.insert("discoveryParams", discoveryParams);
    response = injectAndWait("Integrations.DiscoverThings", params);

    thingDescriptors = response.toMap().value("params").toMap().value("thingDescriptors").toList();
    qCDebug(dcTests()) << "Discovery result:" << qUtf8Printable(QJsonDocument::fromVariant(thingDescriptors).toJson(QJsonDocument::Indented));

    verifyThingError(response, Thing::ThingErrorNoError);
    QCOMPARE(thingDescriptors.count(), 1);

    descriptor = thingDescriptors.first();
    QVERIFY2(ThingId(descriptor.toMap().value("thingId").toString()) == thingId, "thingId not set in descriptor");

    // get the descriptor again
    descriptorId = ThingDescriptorId(descriptor.toMap().value("id").toString());

    QVERIFY(!descriptorId.isNull());

    qDebug() << "Reconfiguring thing by pairing again" << descriptorId;

    params.clear();
    response.clear();
    params.insert("thingClassId", displayPinMockThingClassId);
    params.insert("name", "Discoverd mock");
    params.insert("thingDescriptorId", descriptorId);
    response = injectAndWait("Integrations.PairThing", params);
    verifyThingError(response);

    pairingTransactionId = PairingTransactionId(response.toMap().value("params").toMap().value("pairingTransactionId").toString());
    qCDebug(dcTests()) << "PairThing result:" << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));


    qCDebug(dcTests()) << "Confirming pairing for transaction ID" << pairingTransactionId;
    params.clear();
    response.clear();
    params.insert("pairingTransactionId", pairingTransactionId.toString());
    params.insert("secret", "243681");
    response = injectAndWait("Integrations.ConfirmPairing", params);
    verifyThingError(response);

    thingId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY(!thingId.isNull());

}

void TestIntegrations::reconfigureAutoThing()
{
    qCDebug(dcTests()) << "Reconfigure auto thing";

    // Get the auto mock
    QList<Thing*> things  = NymeaCore::instance()->thingManager()->findConfiguredThings(autoMockThingClassId);
    QVERIFY2(things.count() > 0, "There needs to be at least one auto-created Mock for this test");

    // Get current auto mock infos
    Thing *currentThing = things.first();
    ThingId thingId = currentThing->id();
    int currentPort = currentThing->paramValue(autoMockThingHttpportParamTypeId).toInt();

    // Trigger reconfigure signal in mock
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy spy(nam, &QNetworkAccessManager::finished);
    QNetworkReply *reply = nam->get(QNetworkRequest(QUrl(QString("http://localhost:%1/reconfigureautodevice").arg(currentPort))));
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    Thing *thing = NymeaCore::instance()->thingManager()->findConfiguredThing(thingId);
    QVERIFY(thing);
    int newPort = thing->paramValue(autoMockThingHttpportParamTypeId).toInt();
    // Note: reconfigure auto mock increases the http port by 1
    QCOMPARE(newPort, currentPort + 1);
}


void TestIntegrations::removeThing_data()
{
    QTest::addColumn<ThingId>("thingId");
    QTest::addColumn<Thing::ThingError>("thingError");

    QTest::newRow("Existing thing") << ThingId(m_mockThingId) << Thing::ThingErrorNoError;
    QTest::newRow("Not existing thing") << ThingId::createThingId() << Thing::ThingErrorThingNotFound;
//    QTest::newRow("Auto device") << m_mockThingAutoId << Thing::ThingErrorCreationMethodNotSupported;
}

void TestIntegrations::removeThing()
{
    QFETCH(ThingId, thingId);
    QFETCH(Thing::ThingError, thingError);

    NymeaSettings settings(NymeaSettings::SettingsRoleThings);
    settings.beginGroup("ThingConfig");
    if (thingError == Thing::ThingErrorNoError) {
        settings.beginGroup(m_mockThingId.toString());
        // Make sure we have some config values for this device
        QVERIFY(settings.allKeys().count() > 0);
    }

    QVariantMap params;
    params.insert("thingId", thingId);

    QVariant response = injectAndWait("Integrations.RemoveThing", params);

    verifyThingError(response, thingError);

    if (Thing::ThingErrorNoError) {
        // Make sure the device is gone from settings too
        QCOMPARE(settings.allKeys().count(), 0);
    }
}

void TestIntegrations::removeAutoThing()
{
    // Setup connection to mock client
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QSignalSpy spy(nam, SIGNAL(finished(QNetworkReply*)));

    // First try to make a manually created device disappear. It must not go away

    QList<Thing*> things = NymeaCore::instance()->thingManager()->findConfiguredThings(mockThingClassId);
    int oldCount = things.count();
    QVERIFY2(oldCount > 0, "There needs to be at least one configured Mock Device for this test");
    Thing *thing = things.first();

    // trigger disappear signal in mock device
    int port = thing->paramValue(autoMockThingHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/disappear").arg(port)));
    QNetworkReply *reply = nam->get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
    QVERIFY2(NymeaCore::instance()->thingManager()->findConfiguredThings(mockThingClassId).count() == oldCount, "Mocked thing has disappeared even though it shouldn't");

    // Ok, now do the same with an autocreated one. It should go away

    things = NymeaCore::instance()->thingManager()->findConfiguredThings(autoMockThingClassId);
    oldCount = things.count();
    QVERIFY2(oldCount > 0, "There needs to be at least one auto-created Mock Device for this test");
    thing = things.first();

    // trigger disappear signal in mock device
    spy.clear();
    port = thing->paramValue(autoMockThingHttpportParamTypeId).toInt();
    request.setUrl(QUrl(QString("http://localhost:%1/disappear").arg(port)));
    reply = nam->get(request);

    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();

    // Make sure one mock device has disappeared
    QCOMPARE(NymeaCore::instance()->thingManager()->findConfiguredThings(autoMockThingClassId).count(), oldCount - 1);
}

void TestIntegrations::testBrowsing_data()
{
    QTest::addColumn<ThingId>("thingId");

    QTest::newRow("regular mock") << ThingId(m_mockThingId);
    QTest::newRow("async mock") << ThingId(m_mockThingAsyncId);
}

void TestIntegrations::testBrowsing()
{
    QFETCH(ThingId, thingId);

    // Check if mockdevice is browsable
    QVariant response = injectAndWait("Integrations.GetThingClasses");

    QVariantMap mockThingClass;
    foreach (const QVariant &thingClassVariant, response.toMap().value("params").toMap().value("thingClasses").toList()) {
        if (ThingClassId(thingClassVariant.toMap().value("id").toString()) == mockThingClassId) {
            mockThingClass = thingClassVariant.toMap();
        }
    }

    QVERIFY2(ThingClassId(mockThingClass.value("id").toString()) == mockThingClassId, "Could not find mock device");
    QCOMPARE(mockThingClass.value("browsable").toBool(), true);


    // Browse it
    QVariantMap params;
    params.insert("thingId", thingId);
    response = injectAndWait("Integrations.BrowseThing", params);
    QCOMPARE(response.toMap().value("params").toMap().value("thingError").toString(), QString("ThingErrorNoError"));
    QVariantList browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QVERIFY2(browserEntries.count() > 0, "BrowseThing did not return any items.");

    // Browse item 001, it should be a folder with 2 items
    params.insert("itemId", "001");
    response = injectAndWait("Integrations.BrowseThing", params);
    QCOMPARE(response.toMap().value("params").toMap().value("thingError").toString(), QString("ThingErrorNoError"));
    browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QVERIFY2(browserEntries.count() == 2, "BrowseThing did not return 2 items as childs in folder with id 001.");

    // Browse a non-existent item
    params["itemId"] = "this-does-not-exist";
    response = injectAndWait("Integrations.BrowseThing", params);
    browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QCOMPARE(response.toMap().value("params").toMap().value("thingError").toString(), QString("ThingErrorItemNotFound"));
    QCOMPARE(browserEntries.count(), 0);


}

void TestIntegrations::discoverThingsParenting()
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
    QCOMPARE(addSpy.count(), 2); // Mock parent will also auto-create a child instantly

    Thing *parentThing = addSpy.at(0).first().value<Thing*>();
    qCDebug(dcTests()) << "Added parent:" << parentThing->name();
    QVERIFY(parentThing->thingClassId() == parentMockThingClassId);


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

    Thing *childThing = addSpy.at(0).first().value<Thing*>();
    qCDebug(dcTests()) << "Added child:" << childThing->name();
    QVERIFY(childThing->thingClassId() == childMockThingClassId);

    // Now delete the parent and make sure the child will be deleted too
    QSignalSpy removeSpy(NymeaCore::instance(), &NymeaCore::thingRemoved);
    QPair<Thing::ThingError, QList<RuleId> > ret = NymeaCore::instance()->removeConfiguredThing(parentThing->id(), QHash<RuleId, RuleEngine::RemovePolicy>());
    QCOMPARE(ret.first, Thing::ThingErrorNoError);
    QCOMPARE(removeSpy.count(), 3); // The parent, the auto-mock and the discovered mock

}

void TestIntegrations::testExecuteBrowserItem_data()
{
    QTest::addColumn<ThingId>("thingId");
    QTest::addColumn<QString>("itemId");
    QTest::addColumn<QString>("thingError");

    QTest::newRow("regular mock - good item") << ThingId(m_mockThingId) << "002" << "ThingErrorNoError";
    QTest::newRow("regular mock - bad item") << ThingId(m_mockThingId) << "001" << "ThingErrorItemNotExecutable";
    QTest::newRow("async mock - good item") << ThingId(m_mockThingAsyncId) << "002" << "ThingErrorNoError";
}

void TestIntegrations::testExecuteBrowserItem()
{
    QFETCH(ThingId, thingId);
    QFETCH(QString, itemId);
    QFETCH(QString, thingError);

    QVariantMap params;
    params.insert("thingId", thingId);
    params.insert("itemId", itemId);
    QVariant response = injectAndWait("Integrations.ExecuteBrowserItem", params);
    qCDebug(dcTests()) << "resp" << response;

    QCOMPARE(response.toMap().value("params").toMap().value("thingError").toString(), thingError);
}

void TestIntegrations::testExecuteBrowserItemAction_data()
{
    QTest::addColumn<ThingId>("thingId");

    QTest::newRow("regular mock") << ThingId(m_mockThingId);
    QTest::newRow("async mock") << ThingId(m_mockThingAsyncId);
}

void TestIntegrations::testExecuteBrowserItemAction()
{
    QFETCH(ThingId, thingId);

    QVariantMap getItemsParams;
    getItemsParams.insert("thingId", thingId);
    QVariant response = injectAndWait("Integrations.BrowseThing", getItemsParams);
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
    // ID is "favorites" in mock
    // It should be ampty at this point
    getItemsParams.insert("itemId", "favorites");
    response = injectAndWait("Integrations.BrowseThing", getItemsParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QVERIFY2(browserEntries.count() == 0, "Favorites should be empty at this point");

    // Now add an item to the favorites
    QVariantMap actionParams;
    actionParams.insert("thingId", thingId);
    actionParams.insert("itemId", "002");
    actionParams.insert("actionTypeId", mockAddToFavoritesBrowserItemActionTypeId);
    response = injectAndWait("Integrations.ExecuteBrowserItemAction", actionParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("thingError").toString(), QString("ThingErrorNoError"));

    // Fetch the list again
    response = injectAndWait("Integrations.BrowseThing", getItemsParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QCOMPARE(browserEntries.count(), 1);

    QString favoriteItemId = browserEntries.first().toMap().value("id").toString();
    QVERIFY2(!favoriteItemId.isEmpty(), "ItemId is empty in favorites list");

    // Now remove the again from favorites
    actionParams.clear();
    actionParams.insert("thingId", thingId);
    actionParams.insert("itemId", favoriteItemId);
    actionParams.insert("actionTypeId", mockRemoveFromFavoritesBrowserItemActionTypeId);
    response = injectAndWait("Integrations.ExecuteBrowserItemAction", actionParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));
    QCOMPARE(response.toMap().value("params").toMap().value("thingError").toString(), QString("ThingErrorNoError"));

    // Fetch the list again
    response = injectAndWait("Integrations.BrowseThing", getItemsParams);
    QCOMPARE(response.toMap().value("status").toString(), QString("success"));

    browserEntries = response.toMap().value("params").toMap().value("items").toList();
    QCOMPARE(browserEntries.count(), 0);

}

void TestIntegrations::executeAction_data()
{
    QTest::addColumn<ThingId>("thingId");
    QTest::addColumn<ActionTypeId>("actionTypeId");
    QTest::addColumn<QVariantList>("actionParams");
    QTest::addColumn<Thing::ThingError>("error");

    QVariantList params;
    QVariantMap param1;
    param1.insert("paramTypeId", mockWithParamsActionParam1ParamTypeId);
    param1.insert("value", 5);
    params.append(param1);
    QVariantMap param2;
    param2.insert("paramTypeId", mockWithParamsActionParam2ParamTypeId);
    param2.insert("value", true);
    params.append(param2);

    QTest::newRow("valid action") << ThingId(m_mockThingId) << mockWithParamsActionTypeId << params << Thing::ThingErrorNoError;
    QTest::newRow("invalid thingId") << ThingId::createThingId() << mockWithParamsActionTypeId << params << Thing::ThingErrorThingNotFound;
    QTest::newRow("invalid actionTypeId") << ThingId(m_mockThingId) << ActionTypeId::createActionTypeId() << params << Thing::ThingErrorActionTypeNotFound;
    QTest::newRow("missing params") << ThingId(m_mockThingId) << mockWithParamsActionTypeId << QVariantList() << Thing::ThingErrorMissingParameter;
    QTest::newRow("async action") << ThingId(m_mockThingId) << mockAsyncActionTypeId << QVariantList() << Thing::ThingErrorNoError;
    QTest::newRow("broken action") << ThingId(m_mockThingId) << mockFailingActionTypeId << QVariantList() << Thing::ThingErrorSetupFailed;
    QTest::newRow("async broken action") << ThingId(m_mockThingId) << mockAsyncFailingActionTypeId << QVariantList() << Thing::ThingErrorSetupFailed;
}

void TestIntegrations::executeAction()
{
    QFETCH(ThingId, thingId);
    QFETCH(ActionTypeId, actionTypeId);
    QFETCH(QVariantList, actionParams);
    QFETCH(Thing::ThingError, error);

    QVariantMap params;
    params.insert("actionTypeId", actionTypeId);
    params.insert("thingId", thingId);
    params.insert("params", actionParams);
    QVariant response = injectAndWait("Integrations.ExecuteAction", params);
    qDebug() << "executeActionresponse" << response;
    verifyError(response, "thingError", enumValueName(error));

    // Fetch action execution history from mock device
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));

    QNetworkRequest request(QUrl(QString("http://localhost:%1/actionhistory").arg(m_mockThing1Port)));
    QNetworkReply *reply = nam.get(request);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    reply->deleteLater();
    QByteArray data = reply->readAll();

    if (error == Thing::ThingErrorNoError) {
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

void TestIntegrations::triggerEvent()
{
    enableNotifications({"Integrations"});
    QList<Thing*> things = NymeaCore::instance()->thingManager()->findConfiguredThings(mockThingClassId);
    QVERIFY2(things.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Thing *thing = things.first();

    QSignalSpy spy(NymeaCore::instance(), SIGNAL(eventTriggered(const Event&)));
    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Setup connection to mock client
    QNetworkAccessManager nam;

    // trigger event in mock device
    int port = thing->paramValue(mockThingHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/generateevent?eventtypeid=%2").arg(port).arg(mockEvent1EventTypeId.toString())));
    QNetworkReply *reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);

    // Lets wait for the notification
    spy.wait();
    QVERIFY(spy.count() > 0);
    for (int i = 0; i < spy.count(); i++ ){
        Event event = spy.at(i).at(0).value<Event>();
        if (event.thingId() == thing->id()) {
            // Make sure the event contains all the stuff we expect
            QCOMPARE(event.eventTypeId(), mockEvent1EventTypeId);
        }
    }

    // Check for the notification on JSON API
    QVariantList notifications;
    notifications = checkNotifications(notificationSpy, "Integrations.EventTriggered");
    QVERIFY2(notifications.count() == 1, "Should get Integrations.EventTriggered notification");
    QVariantMap notificationContent = notifications.first().toMap().value("params").toMap();

    QCOMPARE(notificationContent.value("event").toMap().value("thingId").toUuid().toString(), thing->id().toString());
    QCOMPARE(notificationContent.value("event").toMap().value("eventTypeId").toUuid().toString(), mockEvent1EventTypeId.toString());
}

void TestIntegrations::triggerStateChangeEvent()
{
    enableNotifications({"Integrations"});

    QList<Thing*> things = NymeaCore::instance()->thingManager()->findConfiguredThings(mockThingClassId);
    QVERIFY2(things.count() > 0, "There needs to be at least one configured Mock for this test");
    Thing *thing = things.first();

    QSignalSpy spy(NymeaCore::instance(), SIGNAL(eventTriggered(const Event&)));
    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    // Setup connection to mock client
    QNetworkAccessManager nam;

    // trigger state changed event in mock device
    int port = thing->paramValue(mockThingHttpportParamTypeId).toInt();
    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(port).arg(mockIntStateTypeId.toString()).arg(11)));
    QNetworkReply *reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);

    // Lets wait for the notification
    spy.wait();
    QVERIFY(spy.count() > 0);
    for (int i = 0; i < spy.count(); i++ ){
        Event event = spy.at(i).at(0).value<Event>();
        if (event.thingId() == thing->id()) {
            // Make sure the event contains all the stuff we expect
            QCOMPARE(event.eventTypeId().toString(), mockIntStateTypeId.toString());
            QCOMPARE(event.param(ParamTypeId(mockIntStateTypeId.toString())).value().toInt(), 11);
        }
    }

    // Check for the notification on JSON API
    QVariantList notifications;
    notifications = checkNotifications(notificationSpy, "Integrations.EventTriggered");
    QVERIFY2(notifications.count() == 1, "Should get Integrations.EventTriggered notification");
    QVariantMap notificationContent = notifications.first().toMap().value("params").toMap();

    QCOMPARE(notificationContent.value("event").toMap().value("deviceId").toUuid().toString(), thing->id().toString());
    QCOMPARE(notificationContent.value("event").toMap().value("eventTypeId").toUuid().toString(), mockIntEventTypeId.toString());

}

void TestIntegrations::params()
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

void TestIntegrations::asyncSetupEmitsSetupStatusUpdate()
{
    QVariantMap configuredDevices = injectAndWait("Integrations.GetThings").toMap();
    foreach (const QVariant &deviceVariant, configuredDevices.value("params").toMap().value("things").toList()) {
        QVariantMap device = deviceVariant.toMap();
        qCDebug(dcTests()) << "configured thing" << device.value("setupStatus");
    }

    // Restart the core instance to check if settings are loaded at startup
    restartServer();
    enableNotifications({"Integrations"});

    QSignalSpy notificationSpy(m_mockTcpServer, SIGNAL(outgoingData(QUuid,QByteArray)));

    configuredDevices = injectAndWait("Integrations.GetThings").toMap();
    QList<QUuid> thingsWithSetupInProgress;
    foreach (const QVariant &deviceVariant, configuredDevices.value("params").toMap().value("things").toList()) {
        QVariantMap thing = deviceVariant.toMap();
        qCDebug(dcTests()) << "Configured thing" << thing.value("name").toString() << "with setup status" << thing.value("setupStatus").toString();
        if (thing.value("setupStatus").toString() == "ThingSetupStatusInProgress") {
            thingsWithSetupInProgress << thing.value("id").toUuid();
        }
    }
    QVERIFY2(thingsWithSetupInProgress.count() > 0, "This test requires at least one device that is still being set up at this point.");

    QDateTime maxTime = QDateTime::currentDateTime().addSecs(10);
    while (QDateTime::currentDateTime() < maxTime && thingsWithSetupInProgress.count() > 0) {
        QList<QList<QVariant>> notifications = notificationSpy;
        while (notifications.count() > 0) {
            QByteArray notificationData = notifications.takeFirst().at(1).toByteArray();
            QVariantMap notification = QJsonDocument::fromJson(notificationData).toVariant().toMap();
            if (notification.value("notification").toString() == "Integrations.ThingChanged") {
                QString setupStatus = notification.value("params").toMap().value("thing").toMap().value("setupStatus").toString();
                if (setupStatus == "ThingSetupStatusComplete") {
                    qCDebug(dcTests()) << "Device setup completed for" << notification.value("params").toMap().value("thing").toMap().value("name").toString();
                    ThingId thingId = notification.value("params").toMap().value("thing").toMap().value("id").toUuid();
                    thingsWithSetupInProgress.removeAll(thingId);
                }
            }
        }
        notificationSpy.clear();
        if (thingsWithSetupInProgress.count() > 0) {
            notificationSpy.wait();
        }
    }

    QVERIFY2(thingsWithSetupInProgress.isEmpty(), "Some things did not finish the setup!");
}

void TestIntegrations::testTranslations()
{
    // switch language to de_AT
    QVariantMap params;
    params.insert("locale", "de_AT");
    QVariantMap handShake = injectAndWait("JSONRPC.Hello", params).toMap();
    QCOMPARE(handShake.value("params").toMap().value("locale").toString(), QString("de_AT"));

    QVariantMap thingClasses = injectAndWait("Integrations.GetThingClasses").toMap();
    bool found = false;
    foreach (const QVariant &tcVariant, thingClasses.value("params").toMap().value("thingClasses").toList()) {
        QVariantMap tcMap = tcVariant.toMap();
        if (tcMap.value("id").toUuid() == autoMockThingClassId) {
            found = true;

            // Verify thingClass' displayName is translated
            QCOMPARE(tcMap.value("displayName").toString(), QString("Mock \"Thing\" (automatisch erstellt)"));

            // Verify paramTypes are translated
            bool ptFound = false;
            foreach (const QVariant &ptVariant, tcMap.value("paramTypes").toList()) {
                QVariantMap ptMap = ptVariant.toMap();
                if (ptMap.value("id").toUuid() == autoMockThingAsyncParamTypeId) {
                    ptFound = true;
                    QCOMPARE(ptMap.value("displayName").toString(), QString("asynchron"));
                }
            }
            QVERIFY2(ptFound, "ParamType not found in mock thing class.");


            // Verify settings are translated
            bool sFound = false;
            foreach (const QVariant &sVariant, tcMap.value("settingsTypes").toList()) {
                QVariantMap sMap = sVariant.toMap();
                if (sMap.value("id").toUuid() == autoMockSettingsMockSettingParamTypeId) {
                    sFound = true;
                    QCOMPARE(sMap.value("displayName").toString(), QString("Mock-Einstellung"));
                }
            }
            QVERIFY2(sFound, "SettingsType not found in mock thing class.");

            // Verify stateTypes are translated
            bool stFound = false;
            foreach (const QVariant &stVariant, tcMap.value("stateTypes").toList()) {
                QVariantMap stMap = stVariant.toMap();
                if (stMap.value("id").toUuid() == autoMockIntStateTypeId) {
                    stFound = true;
                    QCOMPARE(stMap.value("displayName").toString(), QString("Simulierter Integer Zustand"));
                }
            }
            QVERIFY2(stFound, "StateType not found in mock thing class.");

            // Verify eventTypes are translated
            bool etFound = false;
            foreach (const QVariant &etVariant, tcMap.value("eventTypes").toList()) {
                QVariantMap etMap = etVariant.toMap();
                if (etMap.value("id").toUuid() == autoMockIntEventTypeId) {
                    etFound = true;
                    QCOMPARE(etMap.value("displayName").toString(), QString("Simulierter Integer Zustand geändert"));
                }
            }
            QVERIFY2(etFound, "EventType not found in mock thing class.");

            // Verify actionTypes are translated
            bool atFound = false;
            foreach (const QVariant &atVariant, tcMap.value("actionTypes").toList()) {
                QVariantMap atMap = atVariant.toMap();
                if (atMap.value("id").toUuid() == autoMockWithParamsActionTypeId) {
                    atFound = true;
                    QCOMPARE(atMap.value("displayName").toString(), QString("Mock Aktion 1 (mit Parameter)"));
                }
            }
            QVERIFY2(atFound, "ActionType not found in mock thing class.");
        }
    }
    QVERIFY2(found, "Mock thing class not found.");

}

#include "testintegrations.moc"
QTEST_MAIN(TestIntegrations)

