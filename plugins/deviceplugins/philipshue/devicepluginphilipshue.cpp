/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#include "devicepluginphilipshue.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "types/param.h"
#include "huebridgeconnection.h"

#include <QDebug>
#include <QStringList>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QColor>

VendorId hueVendorId = VendorId("0ae1e001-2aa6-47ed-b8c0-334c3728a68f");

PluginId huePluginUuid = PluginId("5f2e634b-b7f3-48ee-976a-b5ae22aa5c55");
DeviceClassId hueDeviceClassId = DeviceClassId("d8f4c397-e05e-47c1-8917-8e72d4d0d47c");

StateTypeId hueColorStateTypeId = StateTypeId("d25423e7-b924-4b20-80b6-77eecc65d089");
ActionTypeId setHueColorActionTypeId = ActionTypeId("29cc299a-818b-47b2-817f-c5a6361545e4");

DevicePluginPhilipsHue::DevicePluginPhilipsHue():
    m_discovery(new Discovery(this))
{
    connect(m_discovery, &Discovery::discoveryDone, this, &DevicePluginPhilipsHue::discoveryDone);

    m_nam = new QNetworkAccessManager(this);
}

QList<Vendor> DevicePluginPhilipsHue::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor philips(hueVendorId, "Philips");
    ret.append(philips);
    return ret;
}

QList<DeviceClass> DevicePluginPhilipsHue::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassHue(pluginId(), hueVendorId, hueDeviceClassId);
    deviceClassHue.setName("Hue");
    deviceClassHue.setCreateMethod(DeviceClass::CreateMethodDiscovery);

    deviceClassHue.setSetupMethod(DeviceClass::SetupMethodPushButton);
    deviceClassHue.setPairingInfo("Please press the button on the Hue bridge and then press OK");

    QList<ParamType> paramTypes;
    ParamType ipParam("ip", QVariant::String);
    paramTypes.append(ipParam);
    deviceClassHue.setParamTypes(paramTypes);
    
    QList<StateType> hueStates;

    StateType colorState(hueColorStateTypeId);
    colorState.setName("color");
    colorState.setType(QVariant::Color);
    colorState.setDefaultValue(QColor(Qt::black));
    hueStates.append(colorState);

    deviceClassHue.setStateTypes(hueStates);

    QList<ActionType> hueActons;

    ActionType setColorAction(setHueColorActionTypeId);
    setColorAction.setName("Set color");

    QList<ParamType> actionParamsSetColor;
    ParamType actionParamSetColor("color", QVariant::Color);
    actionParamsSetColor.append(actionParamSetColor);
    setColorAction.setParameters(actionParamsSetColor);

    hueActons.append(setColorAction);

    deviceClassHue.setActions(hueActons);

    ret.append(deviceClassHue);

    return ret;
}

DeviceManager::HardwareResources DevicePluginPhilipsHue::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

bool DevicePluginPhilipsHue::configureAutoDevice(QList<Device *> loadedDevices, Device *device) const
{
//    if (!m_bobClient->connected()) {
//        return false;
//    }
//    if (loadedDevices.count() < m_bobClient->lightsCount()) {
//        int index = loadedDevices.count();
//        device->setName("Boblight Channel " + QString::number(index));
//        QList<Param> params;
//        Param param("channel");
//        param.setValue(index);
//        params.append(param);
//        device->setParams(params);
//        device->setStateValue(colorStateTypeId, m_bobClient->currentColor(index));
//        return true;
//    }
    return false;
}

QString DevicePluginPhilipsHue::pluginName() const
{
    return "Philips Hue";
}

PluginId DevicePluginPhilipsHue::pluginId() const
{
    return huePluginUuid;
}

QList<ParamType> DevicePluginPhilipsHue::configurationDescription() const
{
    QList<ParamType> params;
    return params;
}

DeviceManager::DeviceError DevicePluginPhilipsHue::discoverDevices(const DeviceClassId &deviceClassId, const QVariantMap &params) const
{
    m_discovery->findBridges(4000);
    return DeviceManager::DeviceErrorAsync;
}

QPair<DeviceManager::DeviceSetupStatus, QString> DevicePluginPhilipsHue::confirmPairing(const QUuid &pairingTransactionId, const DeviceClassId &deviceClassId, const QList<Param> &params)
{
    Param ipParam;
    foreach (const Param &param, params) {
        if (param.name() == "ip") {
            ipParam = param;
        }
    }
    if (!ipParam.isValid()) {
        return reportDeviceSetup(DeviceManager::DeviceSetupStatusFailure, "Missing parameter: ip");
    }

    QString username = "guh-" + QUuid::createUuid().toString().remove(QRegExp("[\\{\\}]*")).remove(QRegExp("\\-[0-9a-f\\-]*"));

    QVariantMap createUserParams;
    createUserParams.insert("devicetype", "guh");
    createUserParams.insert("username", username);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(createUserParams);
    QByteArray data = jsonDoc.toJson();

    QNetworkRequest request(QUrl("http://" + ipParam.value().toString() + "/api"));
    QNetworkReply *reply = m_nam->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &DevicePluginPhilipsHue::createUserFinished);

    HueBridgeConnection *bridge = new HueBridgeConnection(QHostAddress(ipParam.value().toString()), username);
    m_pairings.insert(reply, qMakePair<QUuid, HueBridgeConnection*>(pairingTransactionId, bridge));
    return reportDeviceSetup(DeviceManager::DeviceSetupStatusAsync);
}

QPair<DeviceManager::DeviceError, QString> DevicePluginPhilipsHue::executeAction(Device *device, const Action &action)
{
//    if (!m_bobClient->connected()) {
        return report(DeviceManager::DeviceErrorSetupFailed, device->id().toString());
//    }
//    QColor newColor = action.param("color").value().value<QColor>();
//    if (!newColor.isValid()) {
//        return report(DeviceManager::DeviceErrorActionParameterError, "color");
//    }
//    qDebug() << "executing boblight action" << newColor;
//    m_bobClient->setColor(device->paramValue("channel").toInt(), newColor);
//    m_bobClient->sync();

//    device->setStateValue(colorStateTypeId, newColor);
        //    return report();
}

void DevicePluginPhilipsHue::discoveryDone(const QList<QHostAddress> &bridges)
{
    qDebug() << "discovered bridges" << bridges.count();
    QList<DeviceDescriptor> deviceDescriptors;
    foreach (const QHostAddress &bridge, bridges) {
        DeviceDescriptor descriptor(hueDeviceClassId, "Philips Hue bridge", bridge.toString());
        QList<Param> params;
        Param param("ip", bridge.toString());
        params.append(param);
        descriptor.setParams(params);
        deviceDescriptors.append(descriptor);
    }

    emit devicesDiscovered(hueDeviceClassId, deviceDescriptors);
}

void DevicePluginPhilipsHue::createUserFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    QByteArray data = reply->readAll();

    QPair<QUuid, HueBridgeConnection*> pair = m_pairings.take(reply);
    QUuid pairingTransactionId = pair.first;
    HueBridgeConnection *bridge = pair.second;

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        emit pairingFinished(pairingTransactionId, DeviceManager::DeviceSetupStatusFailure, "Pairing failed. Failed to parse response from Hue Bridge.");
        delete bridge;
        return;
    }

    QVariantMap response = jsonDoc.toVariant().toList().first().toMap();

    if (response.contains("error")) {
        qDebug() << "Failed to pair Hue bridge:" << response.value("error").toMap().value("description");
        emit pairingFinished(pairingTransactionId, DeviceManager::DeviceSetupStatusFailure, "Pairing failed:" + response.value("error").toMap().value("description").toString());
        delete bridge;
        return;
    }

    emit pairingFinished(pairingTransactionId, DeviceManager::DeviceSetupStatusSuccess, QString());

    m_bridges.append(bridge);
    qDebug() << "response" << response << data;
}
