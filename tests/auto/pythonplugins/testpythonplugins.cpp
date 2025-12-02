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

#include <typeutils.h>
#include <integrations/thing.h>

ThingClassId pyMockThingClassId = ThingClassId("1761c256-99b1-41bd-988a-a76087f6a4f1");
ThingClassId pyMockDiscoveryPairingThingClassId = ThingClassId("248c5046-847b-44d0-ab7c-684ff79197dc");
ParamTypeId pyMockDiscoveryPairingResultCountDiscoveryParamTypeID = ParamTypeId("ef5f6b90-e9d8-4e77-a14d-6725cfb07116");

using namespace nymeaserver;

class TestPythonPlugins: public NymeaTestBase
{
    Q_OBJECT

private:
    inline void verifyThingError(const QVariant &response, Thing::ThingError error = Thing::ThingErrorNoError) {
        verifyError(response, "thingError", enumValueName(error));
    }

private slots:

#ifdef WITH_PYTHON
    void initTestCase();

    void testRestartServer();

    void setupAndRemoveThing();
    void testDiscoverPairAndRemoveThing();
#endif

};

#ifdef WITH_PYTHON

void TestPythonPlugins::testRestartServer()
{
    NymeaTestBase::restartServer();
}

void TestPythonPlugins::initTestCase()
{
    NymeaTestBase::initTestCase("*.debug=false\n*.info=false\n*.warning=false\n"
                                     "Tests.debug=true\n"
                                     "PyMock.debug=true\n"
                                     "PythonIntegrations.debug=true\n"
                                     );
}

void TestPythonPlugins::setupAndRemoveThing()
{
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", pyMockDiscoveryPairingResultCountDiscoveryParamTypeID);
    resultCountParam.insert("value", 2);

    QVariantList discoveryParams;
    discoveryParams.append(resultCountParam);

    QVariantMap params;
    params.insert("thingClassId", pyMockThingClassId);
    params.insert("name", "Py test thing");
    QVariant response = injectAndWait("Integrations.AddThing", params);

    verifyThingError(response, Thing::ThingErrorNoError);
    ThingId thingId = response.toMap().value("params").toMap().value("thingId").toUuid();
    qCDebug(dcTests()) << "New thing id" << thingId;

    params.clear();
    params.insert("thingId", thingId);
    injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response, Thing::ThingErrorNoError);
}

void TestPythonPlugins::testDiscoverPairAndRemoveThing()
{
    // Discover
    QVariantMap resultCountParam;
    resultCountParam.insert("paramTypeId", pyMockDiscoveryPairingResultCountDiscoveryParamTypeID);
    resultCountParam.insert("value", 2);

    QVariantList discoveryParams;
    discoveryParams.append(resultCountParam);

    QVariantMap params;
    params.insert("thingClassId", pyMockDiscoveryPairingThingClassId);
    params.insert("discoveryParams", discoveryParams);
    QVariant response = injectAndWait("Integrations.DiscoverThings", params);

    verifyThingError(response, Thing::ThingErrorNoError);
    QCOMPARE(response.toMap().value("params").toMap().value("thingDescriptors").toList().count(), 2);

    ThingDescriptorId descriptorId = response.toMap().value("params").toMap().value("thingDescriptors").toList().first().toMap().value("id").toUuid();

    // Pair
    params.clear();
    params.insert("thingDescriptorId", descriptorId);
    response = injectAndWait("Integrations.PairThing", params);
    verifyThingError(response, Thing::ThingErrorNoError);

    qWarning() << "respo" << response.toMap().value("params").toMap();
    PairingTransactionId transactionId = response.toMap().value("params").toMap().value("pairingTransactionId").toUuid();
    qWarning() << "transactionId" << transactionId;

    params.clear();
    params.insert("pairingTransactionId", transactionId);
    params.insert("username", "john");
    params.insert("secret", "smith");
    response = injectAndWait("Integrations.ConfirmPairing", params);
    verifyThingError(response, Thing::ThingErrorNoError);
    ThingId thingId = response.toMap().value("params").toMap().value("thingId").toUuid();

    // Remove
    params.clear();
    params.insert("thingId", thingId);
    response = injectAndWait("Integrations.RemoveThing", params);
    verifyThingError(response, Thing::ThingErrorNoError);
}
#endif

#include "testpythonplugins.moc"
QTEST_MAIN(TestPythonPlugins)
