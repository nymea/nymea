/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 **
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "nymeatestbase.h"
#include "nymeacore.h"

using namespace nymeaserver;

class TestStates: public NymeaTestBase
{
    Q_OBJECT

private slots:
    void getStateTypes();

    void getStateValue_data();
    void getStateValue();

    void save_load_states();
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
    QList<Device*> devices = NymeaCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId);
    QVERIFY2(devices.count() > 0, "There needs to be at least one configured Mock Device for this test");
    Device *device = devices.first();

    QTest::addColumn<DeviceId>("deviceId");
    QTest::addColumn<StateTypeId>("stateTypeId");
    QTest::addColumn<Device::DeviceError>("error");

    QTest::newRow("existing state") << device->id() << mockIntStateTypeId << Device::DeviceErrorNoError;
    QTest::newRow("invalid device") << DeviceId::createDeviceId() << mockIntStateTypeId << Device::DeviceErrorDeviceNotFound;
    QTest::newRow("invalid statetype") << device->id() << StateTypeId::createStateTypeId() << Device::DeviceErrorStateTypeNotFound;
}

void TestStates::getStateValue()
{
    QFETCH(DeviceId, deviceId);
    QFETCH(StateTypeId, stateTypeId);
    QFETCH(Device::DeviceError, error);

    QVariantMap params;
    params.insert("deviceId", deviceId.toString());
    params.insert("stateTypeId", stateTypeId.toString());

    QVariant response = injectAndWait("Devices.GetStateValue", params);

    verifyError(response, "deviceError", enumValueName(error));
}

void TestStates::save_load_states()
{
    DeviceClass mockDeviceClass = NymeaCore::instance()->deviceManager()->findDeviceClass(mockDeviceClassId);

    QVERIFY2(mockDeviceClass.getStateType(mockIntStateTypeId).cached(), "Mock int state is not cached (required to be true for this test)");
    QVERIFY2(!mockDeviceClass.getStateType(mockBoolStateTypeId).cached(), "Mock bool state is cached (required to be false for this test)");

    Device* device = NymeaCore::instance()->deviceManager()->findConfiguredDevices(mockDeviceClassId).first();
    int port = device->paramValue(mockDeviceHttpportParamTypeId).toInt();
    QNetworkAccessManager nam;
    QSignalSpy spy(&nam, SIGNAL(finished(QNetworkReply*)));


    // First set the state values to something that is *not* the default
    int newIntValue = mockDeviceClass.getStateType(mockIntStateTypeId).defaultValue().toInt() + 1;
    bool newBoolValue = !mockDeviceClass.getStateType(mockBoolStateTypeId).defaultValue().toBool();

    QNetworkRequest request(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(port).arg(mockIntStateTypeId.toString()).arg(newIntValue)));
    QNetworkReply *reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    spy.wait();

    spy.clear();
    request = QNetworkRequest(QUrl(QString("http://localhost:%1/setstate?%2=%3").arg(port).arg(mockBoolStateTypeId.toString()).arg(newBoolValue)));
    reply = nam.get(request);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    spy.wait();

    // For completeness, verify through JSONRPC that they were actually yet.
    QVariantMap params;
    params.insert("deviceId", device->id());

    params["stateTypeId"] = mockIntStateTypeId;
    QVariant response = injectAndWait("Devices.GetStateValue", params);
    QCOMPARE(response.toMap().value("params").toMap().value("value").toInt(), newIntValue);

    params["stateTypeId"] = mockBoolStateTypeId;
    response = injectAndWait("Devices.GetStateValue", params);
    QCOMPARE(response.toMap().value("params").toMap().value("value").toBool(), newBoolValue);

    // Restart the server
    restartServer();

    // And check if the cached int state has successfully been restored
    params["stateTypeId"] = mockIntStateTypeId;
    response = injectAndWait("Devices.GetStateValue", params);
    QCOMPARE(response.toMap().value("params").toMap().value("value").toInt(), newIntValue);

    // and that the non-cached bool state is back to its default
    params["stateTypeId"] = mockBoolStateTypeId;
    response = injectAndWait("Devices.GetStateValue", params);
    QCOMPARE(response.toMap().value("params").toMap().value("value").toBool(), mockDeviceClass.getStateType(mockBoolStateTypeId).defaultValue().toBool());
}

#include "teststates.moc"
QTEST_MAIN(TestStates)
