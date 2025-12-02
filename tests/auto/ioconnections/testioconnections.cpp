// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeatestbase.h"
#include "nymeacore.h"
#include "nymeasettings.h"

#include "integrations/thingdiscoveryinfo.h"
#include "integrations/thingsetupinfo.h"

#include "servers/mocktcpserver.h"
#include "jsonrpc/integrationshandler.h"

#include "../plugins/mock/extern-plugininfo.h"

using namespace nymeaserver;

class TestIOConnections : public NymeaTestBase
{
    Q_OBJECT

private:
    ThingId m_ioThingId;
    ThingId m_lightThingId;
    ThingId m_tempSensorThingId;

    inline void verifyThingError(const QVariant &response, Thing::ThingError error = Thing::ThingErrorNoError) {
        verifyError(response, "thingError", enumValueName(error));
    }

private slots:

    void initTestCase();

    void testConnectionCompatibility_data();
    void testConnectionCompatibility();

    void testDigitalIO_data();
    void testDigitalIO();

    void testAnalogIO_data();
    void testAnalogIO();
};

void TestIOConnections::initTestCase()
{
    NymeaTestBase::initTestCase();
    QLoggingCategory::setFilterRules("*.debug=false\n"
                                     "Tests.debug=true\n"
                                     "Mock.debug=true\n"
                                     "ThingManager.debug=true\n"
                                     );

    // Adding generic IO mock
    QVariantMap params;
    params.insert("thingClassId", genericIoMockThingClassId);
    params.insert("name", "Generic IO mock");
    QVariant response = injectAndWait("Integrations.AddThing", params);
    m_ioThingId = ThingId(response.toMap().value("params").toMap().value("thingId").toString());
    QVERIFY2(!m_ioThingId.isNull(), "Creating generic IO mock failed");
    qCDebug(dcTests()) << "Created IO mock with ID" << m_ioThingId;

    // Adding virtual light (digital input)
    params.clear();
    params.insert("thingClassId", virtualIoLightMockThingClassId);
    params.insert("name", "light");
    response = injectAndWait("Integrations.AddThing", params);
    m_lightThingId = ThingId(response.toMap().value("params").toMap().value("thingId").toUuid());
    QVERIFY2(!m_lightThingId.isNull(), "Creating virtual light failed");

    // Adding virtual temp sensor (analog output)
    params.clear();
    params.insert("thingClassId", virtualIoTemperatureSensorMockThingClassId);
    params.insert("name", "temp sensor");
    response = injectAndWait("Integrations.AddThing", params);
    m_tempSensorThingId = ThingId(response.toMap().value("params").toMap().value("thingId").toUuid());
    QVERIFY2(!m_tempSensorThingId.isNull(), "Creating virtual temp sensor failed");
}

void TestIOConnections::testConnectionCompatibility_data()
{
    QTest::addColumn<ThingId>("inputThingId");
    QTest::addColumn<StateTypeId>("inputStateTypeId");
    QTest::addColumn<ThingId>("outputThingId");
    QTest::addColumn<StateTypeId>("outputStateTypeId");
    QTest::addColumn<Thing::ThingError>("expectedError");

    QTest::newRow("digital in, digital in") << m_ioThingId << genericIoMockDigitalInput1StateTypeId << m_ioThingId << genericIoMockDigitalInput1StateTypeId << Thing::ThingErrorInvalidParameter;
    QTest::newRow("digital in, digital out") << m_ioThingId << genericIoMockDigitalInput1StateTypeId << m_ioThingId << genericIoMockDigitalOutput1StateTypeId << Thing::ThingErrorNoError;
    QTest::newRow("digital in, analog in") << m_ioThingId << genericIoMockDigitalInput1StateTypeId << m_ioThingId << genericIoMockAnalogInput1StateTypeId << Thing::ThingErrorInvalidParameter;
    QTest::newRow("digital in, analog out") << m_ioThingId << genericIoMockDigitalInput1StateTypeId << m_ioThingId << genericIoMockAnalogOutput1StateTypeId << Thing::ThingErrorInvalidParameter;

    QTest::newRow("digital out, digital in") << m_ioThingId << genericIoMockDigitalOutput1StateTypeId << m_ioThingId << genericIoMockDigitalInput1StateTypeId << Thing::ThingErrorInvalidParameter;
    QTest::newRow("digital out, digital out") << m_ioThingId << genericIoMockDigitalOutput1StateTypeId << m_ioThingId << genericIoMockDigitalOutput1StateTypeId << Thing::ThingErrorInvalidParameter;
    QTest::newRow("digital out, analog in") << m_ioThingId << genericIoMockDigitalOutput1StateTypeId << m_ioThingId << genericIoMockAnalogInput1StateTypeId << Thing::ThingErrorInvalidParameter;
    QTest::newRow("digital out, analot out") << m_ioThingId << genericIoMockDigitalOutput1StateTypeId << m_ioThingId << genericIoMockAnalogOutput1StateTypeId << Thing::ThingErrorInvalidParameter;

    QTest::newRow("analog in, digital in") << m_ioThingId << genericIoMockAnalogInput1StateTypeId << m_ioThingId << genericIoMockDigitalInput1StateTypeId << Thing::ThingErrorInvalidParameter;
    QTest::newRow("analog in, digital out") << m_ioThingId << genericIoMockAnalogInput1StateTypeId << m_ioThingId << genericIoMockDigitalOutput1StateTypeId << Thing::ThingErrorInvalidParameter;
    QTest::newRow("analog in, analog in") << m_ioThingId << genericIoMockAnalogInput1StateTypeId << m_ioThingId << genericIoMockAnalogInput1StateTypeId << Thing::ThingErrorInvalidParameter;
    QTest::newRow("analog in, analog out") << m_ioThingId << genericIoMockAnalogInput1StateTypeId << m_ioThingId << genericIoMockAnalogOutput1StateTypeId << Thing::ThingErrorNoError;

    QTest::newRow("analog out, digital in") << m_ioThingId << genericIoMockAnalogOutput1StateTypeId << m_ioThingId << genericIoMockDigitalInput1StateTypeId << Thing::ThingErrorInvalidParameter;
    QTest::newRow("analog out, digital out") << m_ioThingId << genericIoMockAnalogOutput1StateTypeId << m_ioThingId << genericIoMockDigitalOutput1StateTypeId << Thing::ThingErrorInvalidParameter;
    QTest::newRow("analog out, analog in") << m_ioThingId << genericIoMockAnalogOutput1StateTypeId << m_ioThingId << genericIoMockAnalogInput1StateTypeId << Thing::ThingErrorInvalidParameter;
    QTest::newRow("analog out, analog out") << m_ioThingId << genericIoMockAnalogOutput1StateTypeId << m_ioThingId << genericIoMockAnalogOutput1StateTypeId << Thing::ThingErrorInvalidParameter;

    QTest::newRow("valid input, invalid output thing") << m_ioThingId << genericIoMockDigitalInput1StateTypeId << ThingId("707d5093-4915-499e-8e69-10c11972bb34") << genericIoMockDigitalOutput1StateTypeId << Thing::ThingErrorThingNotFound;
    QTest::newRow("valid input, invalid output stateType") << m_ioThingId << genericIoMockDigitalInput1StateTypeId << m_ioThingId << StateTypeId("51534cd7-8adf-4bdc-a4c1-042d0a9d4faa") << Thing::ThingErrorStateTypeNotFound;
    QTest::newRow("invalid input thing, valid output") << ThingId("99843693-8615-416a-8e59-a47b050f5c1a") << genericIoMockDigitalInput1StateTypeId << m_ioThingId << genericIoMockDigitalOutput1StateTypeId << Thing::ThingErrorThingNotFound;
    QTest::newRow("invalid input stateType, valid output") << m_ioThingId << StateTypeId("04657948-e349-4f43-bf20-13f9986ad1b4") << m_ioThingId << genericIoMockDigitalOutput1StateTypeId << Thing::ThingErrorStateTypeNotFound;
}

void TestIOConnections::testConnectionCompatibility()
{
    QFETCH(ThingId, inputThingId);
    QFETCH(StateTypeId, inputStateTypeId);
    QFETCH(ThingId, outputThingId);
    QFETCH(StateTypeId, outputStateTypeId);
    QFETCH(Thing::ThingError, expectedError);

    QVariantMap params;
    params.insert("inputThingId", inputThingId);
    params.insert("inputStateTypeId", inputStateTypeId);
    params.insert("outputThingId", outputThingId);
    params.insert("outputStateTypeId", outputStateTypeId);
    QVariant response = injectAndWait("Integrations.ConnectIO", params);
    verifyThingError(response, expectedError);

}

void TestIOConnections::testDigitalIO_data()
{
    QTest::addColumn<bool>("inverted");

    QTest::newRow("normal") << false;
    QTest::newRow("inverted") << true;
}

void TestIOConnections::testDigitalIO()
{
    QFETCH(bool, inverted);

    QVariantMap params;
    params.insert("inputThingId", m_lightThingId);
    params.insert("inputStateTypeId", virtualIoLightMockPowerStateTypeId);
    params.insert("outputThingId", m_ioThingId);
    params.insert("outputStateTypeId", genericIoMockDigitalOutput1StateTypeId);
    params.insert("inverted", inverted);
    QVariant response = injectAndWait("Integrations.ConnectIO", params);
    verifyThingError(response);
    IOConnectionId ioConnectionId = response.toMap().value("params").toMap().value("ioConnectionId").toUuid();

    // verify input is off
    bool expectedValue = false;
    params.clear();
    params.insert("thingId", m_lightThingId);
    params.insert("stateTypeId", virtualIoLightMockPowerStateTypeId);
    response = injectAndWait("Integrations.GetStateValue", params);
    verifyThingError(response);
    QVERIFY2(response.toMap().value("params").toMap().value("value").toBool() == expectedValue, "Light isn't turned off");

    // verify output is off (or inverted)
    params.clear();
    params.insert("thingId", m_ioThingId);
    params.insert("stateTypeId", genericIoMockDigitalOutput1StateTypeId);
    response = injectAndWait("Integrations.GetStateValue", params);
    verifyThingError(response);
    QVERIFY2(response.toMap().value("params").toMap().value("value").toBool() == (expectedValue xor inverted), "Digital output isn't turned off");

    // Turn on light and verify digital output went on
    expectedValue = true;
    params.clear();
    params.insert("thingId", m_lightThingId);
    params.insert("actionTypeId", virtualIoLightMockPowerActionTypeId);
    QVariantMap actionParam;
    actionParam.insert("paramTypeId", virtualIoLightMockPowerActionPowerParamTypeId);
    actionParam.insert("value", true);
    params.insert("params", QVariantList() << actionParam);
    response = injectAndWait("Integrations.ExecuteAction", params);
    verifyThingError(response);

    params.clear();
    params.insert("thingId", m_ioThingId);
    params.insert("stateTypeId", genericIoMockDigitalOutput1StateTypeId);
    response = injectAndWait("Integrations.GetStateValue", params);
    verifyThingError(response);
    QVERIFY2(response.toMap().value("params").toMap().value("value").toBool() == (expectedValue xor inverted), "Digital output isn't turned on");

    // Disconnect IO again
    params.clear();
    params.insert("ioConnectionId", ioConnectionId);
    response = injectAndWait("Integrations.DisconnectIO", params);
    verifyThingError(response);

    // Turn off the light and verify digital output is still on
    expectedValue = true;
    params.clear();
    params.insert("thingId", m_lightThingId);
    params.insert("actionTypeId", virtualIoLightMockPowerActionTypeId);
    actionParam.clear();
    actionParam.insert("paramTypeId", virtualIoLightMockPowerActionPowerParamTypeId);
    actionParam.insert("value", false);
    params.insert("params", QVariantList() << actionParam);
    response = injectAndWait("Integrations.ExecuteAction", params);
    verifyThingError(response);

    params.clear();
    params.insert("thingId", m_ioThingId);
    params.insert("stateTypeId", genericIoMockDigitalOutput1StateTypeId);
    response = injectAndWait("Integrations.GetStateValue", params);
    verifyThingError(response);
    QVERIFY2(response.toMap().value("params").toMap().value("value").toBool() == (expectedValue xor inverted), "Digital output turned off while it should not");
}

void TestIOConnections::testAnalogIO_data()
{
    QTest::addColumn<bool>("inverted");

    QTest::newRow("normal") << false;
    QTest::newRow("inverted") << true;
}

void TestIOConnections::testAnalogIO()
{
    QFETCH(bool, inverted);

    // Set input to 0
    QVariantMap params;
    params.insert("thingId", m_ioThingId);
    params.insert("actionTypeId", genericIoMockAnalogInput1StateTypeId);
    QVariantMap actionParam;
    actionParam.insert("paramTypeId", genericIoMockAnalogInput1ActionAnalogInput1ParamTypeId);
    actionParam.insert("value", 0); // goes from 0 to 3.3
    params.insert("params", QVariantList() << actionParam);
    QVariant response = injectAndWait("Integrations.ExecuteAction", params);
    verifyThingError(response);

    // Connect IO to it
    params.clear();
    params.insert("inputThingId", m_ioThingId);
    params.insert("inputStateTypeId", genericIoMockAnalogInput1StateTypeId);
    params.insert("outputThingId", m_tempSensorThingId);
    params.insert("outputStateTypeId", virtualIoTemperatureSensorMockInputStateTypeId);
    params.insert("inverted", inverted);

    response = injectAndWait("Integrations.ConnectIO", params);
    verifyThingError(response);
    IOConnectionId ioConnectionId = response.toMap().value("params").toMap().value("ioConnectionId").toUuid();

    // and check temp senser
    double expectedTemp = inverted ? 50 : -20;
    params.clear();
    params.insert("thingId", m_tempSensorThingId);
    params.insert("stateTypeId", virtualIoTemperatureSensorMockTemperatureStateTypeId);
    response = injectAndWait("Integrations.GetStateValue", params);
    verifyThingError(response);
    QVERIFY2(qFuzzyCompare(response.toMap().value("params").toMap().value("value").toDouble(), expectedTemp), QString("Temp sensor is not at %1 but at %2").arg(expectedTemp).arg(response.toMap().value("params").toMap().value("value").toDouble()).toUtf8());


    // set analog input to 0.5 and verify temp aligned
    params.clear();
    params.insert("thingId", m_ioThingId);
    params.insert("actionTypeId", genericIoMockAnalogInput1StateTypeId);
    actionParam.clear();
    actionParam.insert("paramTypeId", genericIoMockAnalogInput1ActionAnalogInput1ParamTypeId);
    actionParam.insert("value", 1.65); // goes from 0 to 3.3
    params.insert("params", QVariantList() << actionParam);
    response = injectAndWait("Integrations.ExecuteAction", params);
    verifyThingError(response);

    params.clear();
    params.insert("thingId", m_tempSensorThingId);
    params.insert("stateTypeId", virtualIoTemperatureSensorMockTemperatureStateTypeId);
    response = injectAndWait("Integrations.GetStateValue", params);
    verifyThingError(response);
    // generic IO output goes from 0 to 3.3. We're setting 1.65V which 50%
    // temp goes from -20 to 50. A input of 1.65 should output a temperature of 15°C
    expectedTemp = 70.0 / 2 - 20;
    QVERIFY2(qFuzzyCompare(response.toMap().value("params").toMap().value("value").toDouble(), expectedTemp), QString("Temp sensor is not at %1 but at %2").arg(expectedTemp).arg(response.toMap().value("params").toMap().value("value").toDouble()).toUtf8());

    // Disconnect IO again
    params.clear();
    params.insert("ioConnectionId", ioConnectionId);
    response = injectAndWait("Integrations.DisconnectIO", params);
    verifyThingError(response);

    // set analog input to 3 and verify temp is still at the old expectedTemp
    params.clear();
    params.insert("thingId", m_ioThingId);
    params.insert("actionTypeId", genericIoMockAnalogInput1StateTypeId);
    actionParam.clear();
    actionParam.insert("paramTypeId", genericIoMockAnalogInput1ActionAnalogInput1ParamTypeId);
    actionParam.insert("value", 3); // goes from 0 to 3.3
    params.insert("params", QVariantList() << actionParam);
    response = injectAndWait("Integrations.ExecuteAction", params);
    verifyThingError(response);

    params.clear();
    params.insert("thingId", m_tempSensorThingId);
    params.insert("stateTypeId", virtualIoTemperatureSensorMockTemperatureStateTypeId);
    response = injectAndWait("Integrations.GetStateValue", params);
    verifyThingError(response);
    QVERIFY2(qFuzzyCompare(response.toMap().value("params").toMap().value("value").toDouble(), expectedTemp), QString("Temp sensor is not at %1 but at %2").arg(expectedTemp).arg(response.toMap().value("params").toMap().value("value").toDouble()).toUtf8());

}

#include "testioconnections.moc"
QTEST_MAIN(TestIOConnections)

