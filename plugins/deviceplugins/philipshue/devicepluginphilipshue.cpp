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
#include <QColor>

VendorId hueVendorId = VendorId("0ae1e001-2aa6-47ed-b8c0-334c3728a68f");

PluginId huePluginUuid = PluginId("5f2e634b-b7f3-48ee-976a-b5ae22aa5c55");
DeviceClassId hueDeviceClassId = DeviceClassId("d8f4c397-e05e-47c1-8917-8e72d4d0d47c");

StateTypeId hueColorStateTypeId = StateTypeId("d25423e7-b924-4b20-80b6-77eecc65d089");
ActionTypeId hueSetColorActionTypeId = ActionTypeId("29cc299a-818b-47b2-817f-c5a6361545e4");

StateTypeId huePowerStateTypeId = StateTypeId("6ac64eee-f356-4ae4-bc85-8c1244d12b02");
ActionTypeId hueSetPowerActionTypeId = ActionTypeId("7782d91e-d73a-4321-8828-da768e2f6827");

DevicePluginPhilipsHue::DevicePluginPhilipsHue():
    m_discovery(new Discovery(this))
{
    connect(m_discovery, &Discovery::discoveryDone, this, &DevicePluginPhilipsHue::discoveryDone);

    m_bridge = new HueBridgeConnection(this);
    connect(m_bridge, &HueBridgeConnection::createUserFinished, this, &DevicePluginPhilipsHue::createUserFinished);
    connect(m_bridge, &HueBridgeConnection::getFinished, this, &DevicePluginPhilipsHue::getFinished);
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
    ParamType usernameParam("username", QVariant::String);
    paramTypes.append(usernameParam);
    ParamType numberParam("number", QVariant::Int, -1);
    paramTypes.append(numberParam);
    deviceClassHue.setParamTypes(paramTypes);
    
    QList<StateType> hueStates;

    StateType colorState(hueColorStateTypeId);
    colorState.setName("color");
    colorState.setType(QVariant::Color);
    colorState.setDefaultValue(QColor(Qt::black));
    hueStates.append(colorState);

    StateType powerState(huePowerStateTypeId);
    powerState.setName("power");
    powerState.setType(QVariant::Bool);
    powerState.setDefaultValue(false);
    hueStates.append(powerState);

    deviceClassHue.setStateTypes(hueStates);

    QList<ActionType> hueActons;

    ActionType setColorAction(hueSetColorActionTypeId);
    setColorAction.setName("Set color");
    QList<ParamType> actionParamsSetColor;
    ParamType actionParamSetColor("color", QVariant::Color);
    actionParamsSetColor.append(actionParamSetColor);
    setColorAction.setParameters(actionParamsSetColor);
    hueActons.append(setColorAction);

    ActionType setPowerAction(hueSetPowerActionTypeId);
    setPowerAction.setName("Power");
    QList<ParamType> actionParamsSetPower;
    ParamType actionParamSetPower("power", QVariant::Bool);
    actionParamsSetPower.append(actionParamSetPower);
    setPowerAction.setParameters(actionParamsSetPower);
    hueActons.append(setPowerAction);

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

DeviceManager::DeviceError DevicePluginPhilipsHue::discoverDevices(const DeviceClassId &deviceClassId, const QList<Param> &params) const
{
    m_discovery->findBridges(4000);
    return DeviceManager::DeviceErrorAsync;
}

QPair<DeviceManager::DeviceSetupStatus, QString> DevicePluginPhilipsHue::setupDevice(Device *device)
{
    qDebug() << "setupDevice" << device->params();

    Light *light = nullptr;

    // Lets see if this a a newly added device... In which case its hue id number is not set, well, -1...
    if (device->paramValue("number").toInt() == -1) {
        if (m_unconfiguredLights.count() > 0) {
            light = m_unconfiguredLights.takeFirst();
            device->setParamValue("number", light->id());
        } else {
            // this shouldn't ever happen
            return reportDeviceSetup(DeviceManager::DeviceSetupStatusFailure, "Device not configured yet and no discovered devices around.");
        }
    } else {
        // In this case it most likely comes from the config. Just read all values from there...
        light = new Light(QHostAddress(device->paramValue("ip").toString()), device->paramValue("username").toString(), device->paramValue("number").toInt());
    }

    connect(light, &Light::stateChanged, this, &DevicePluginPhilipsHue::lightStateChanged);
    light->refresh();

    m_lights.insert(light, device);
    m_asyncSetups.insert(light, device);

    return reportDeviceSetup(DeviceManager::DeviceSetupStatusAsync);
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
    Param usernameParam;
    foreach (const Param &param, params) {
        if (param.name() == "username") {
            usernameParam = param;
        }
    }
    if (!usernameParam.isValid()) {
        return reportDeviceSetup(DeviceManager::DeviceSetupStatusFailure, "Missing parameter: username");
    }

    int id = m_bridge->createUser(QHostAddress(ipParam.value().toString()), usernameParam.value().toString());
    PairingInfo pi;
    pi.pairingTransactionId = pairingTransactionId;
    pi.ipParam = ipParam;
    pi.usernameParam = usernameParam;
    m_pairings.insert(id, pi);
    return reportDeviceSetup(DeviceManager::DeviceSetupStatusAsync);
}

QPair<DeviceManager::DeviceError, QString> DevicePluginPhilipsHue::executeAction(Device *device, const Action &action)
{
    qDebug() << "Should execute action in hue plugin";

    Light *light = m_lights.key(device);
    if (!light) {
        return report(DeviceManager::DeviceErrorDeviceNotFound, device->id().toString());
    }

    if (action.actionTypeId() == hueSetColorActionTypeId) {
        light->setColor(action.param("color").value().value<QColor>());
    } else if (action.actionTypeId() == hueSetPowerActionTypeId) {
        light->setOn(action.param("power").value().toBool());
    }
    return report();
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
        Param userParam("username", "guh-" + QUuid::createUuid().toString().remove(QRegExp("[\\{\\}]*")).remove(QRegExp("\\-[0-9a-f\\-]*")));
        params.append(userParam);
        Param numberParam("number", -1);
        params.append(numberParam);
        descriptor.setParams(params);
        deviceDescriptors.append(descriptor);
    }

    emit devicesDiscovered(hueDeviceClassId, deviceDescriptors);
}

void DevicePluginPhilipsHue::createUserFinished(int id, const QVariantMap &response)
{
    qDebug() << "createuser response" << response;

    PairingInfo pairingInfo = m_pairings.take(id);
    if (response.contains("error")) {
        qDebug() << "Failed to pair Hue bridge:" << response.value("error").toMap().value("description");
        emit pairingFinished(pairingInfo.pairingTransactionId, DeviceManager::DeviceSetupStatusFailure, "Pairing failed:" + response.value("error").toMap().value("description").toString());
        return;
    }

    // Paired successfully, check how many lightbulbs there are

    int getLightsId = m_bridge->get(QHostAddress(pairingInfo.ipParam.value().toString()), pairingInfo.usernameParam.value().toString(), "lights", this, "getLightsFinished");
    m_pairings.insert(getLightsId, pairingInfo);

}

void DevicePluginPhilipsHue::getLightsFinished(int id, const QVariantMap &params)
{
    qDebug() << "getlightsfinished" << params;
    PairingInfo pairingInfo = m_pairings.take(id);

    if (params.count() == 0) {
        qWarning() << "No light bulbs found on this hue bridge... Cannot proceed with pairing.";
        emit pairingFinished(pairingInfo.pairingTransactionId, DeviceManager::DeviceSetupStatusFailure, "No light bulbs found on this Hue bridge.");
        return;
    }

    // Store a list of all known Lights
    foreach (const QString &lightId, params.keys()) {
        Light *light = new Light(QHostAddress(pairingInfo.ipParam.value().toString()), pairingInfo.usernameParam.value().toString(), lightId.toInt(), this);
        m_unconfiguredLights.insert(lightId.toInt(), light);
    }

    emit pairingFinished(pairingInfo.pairingTransactionId, DeviceManager::DeviceSetupStatusSuccess, QString());

//    // If we have more than one device on that bridge, tell DeviceManager that there are more.
//    if (params.count() > 1) {
//        emit autoDevicesAppeared();
//    }
}

void DevicePluginPhilipsHue::getFinished(int id, const QVariantMap &params)
{
    qDebug() << "got lights" << params;
}

void DevicePluginPhilipsHue::lightStateChanged()
{
    Light *light = static_cast<Light*>(sender());

    Device *device;
    if (m_asyncSetups.contains(light)) {
        device = m_asyncSetups.take(light);
        device->setName(light->name());
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess, QString());
    } else {
        device = m_lights.value(light);
    }
    if (!device) {
        return;
    }
    device->setStateValue(hueColorStateTypeId, QVariant::fromValue(light->color()));
    device->setStateValue(huePowerStateTypeId, light->on());
}
