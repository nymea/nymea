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

#include "devicepluginwemo.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include <QDebug>

VendorId belkinVendorId = VendorId("b241f7f5-8153-4a72-b260-f62beadc2d19");
DeviceClassId wemoSwitchDeviceClassId = DeviceClassId("69d97d3b-a8e6-42f3-afc0-ca8a53eb7cce");

StateTypeId powerStateTypeId = StateTypeId("7166c4f6-f68c-4188-8f7c-2205d72a5a6d");
StateTypeId reachableStateTypeId = StateTypeId("ec2f5b49-585c-4455-a233-b7aa4c608dbc");
ActionTypeId powerActionTypeId = ActionTypeId("269f25eb-d0b7-4144-b9ef-801f4ff3e90c");


DevicePluginWemo::DevicePluginWemo()
{
    m_discovery = new WemoDiscovery(this);

    connect(m_discovery,SIGNAL(discoveryDone(QList<WemoSwitch*>)),this,SLOT(discoveryDone(QList<WemoSwitch*>)));
}

QList<Vendor> DevicePluginWemo::supportedVendors() const
{
    QList<Vendor> ret;
    ret.append(Vendor(belkinVendorId, "Belkin"));
    return ret;
}

QList<DeviceClass> DevicePluginWemo::supportedDevices() const
{
    QList<DeviceClass> ret;

    // ==============================
    // WeMo Switch
    DeviceClass deviceClassWemoSwitch(pluginId(), belkinVendorId, wemoSwitchDeviceClassId);
    deviceClassWemoSwitch.setName("WeMo Switch");
    deviceClassWemoSwitch.setCreateMethod(DeviceClass::CreateMethodDiscovery);

    // params
    QList<ParamType> paramTypes;

    paramTypes.append(ParamType("name", QVariant::String));
    paramTypes.append(ParamType("uuid", QVariant::String));
    paramTypes.append(ParamType("model", QVariant::String));
    paramTypes.append(ParamType("host address", QVariant::String));
    paramTypes.append(ParamType("port", QVariant::Int));
    paramTypes.append(ParamType("model description", QVariant::String));
    paramTypes.append(ParamType("serial number", QVariant::String));
    paramTypes.append(ParamType("location", QVariant::String));
    paramTypes.append(ParamType("manufacturer", QVariant::String));
    paramTypes.append(ParamType("device type", QVariant::String));

    deviceClassWemoSwitch.setParamTypes(paramTypes);

    // States
    QList<StateType> wemoSwitchStates;

    StateType powerState(powerStateTypeId);
    powerState.setName("power");
    powerState.setType(QVariant::Bool);
    powerState.setDefaultValue(false);
    wemoSwitchStates.append(powerState);

    StateType reachableState(reachableStateTypeId);
    reachableState.setName("reachable");
    reachableState.setType(QVariant::Bool);
    reachableState.setDefaultValue(false);
    wemoSwitchStates.append(reachableState);

    deviceClassWemoSwitch.setStateTypes(wemoSwitchStates);

    // Actions
    QList<ActionType> wemoSwitchActons;

    ActionType powerAction(powerActionTypeId);
    powerAction.setName("Set power");
    QList<ParamType> actionParamsPower;
    actionParamsPower.append(ParamType("power", QVariant::Bool));
    powerAction.setParameters(actionParamsPower);
    wemoSwitchActons.append(powerAction);

    deviceClassWemoSwitch.setActions(wemoSwitchActons);

    ret.append(deviceClassWemoSwitch);
    return ret;
}

QPair<DeviceManager::DeviceError, QString> DevicePluginWemo::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    if(deviceClassId != wemoSwitchDeviceClassId){
        return report(DeviceManager::DeviceErrorDeviceClassNotFound);
    }

    m_discovery->discover(2000);

    return report(DeviceManager::DeviceErrorAsync);
}

QPair<DeviceManager::DeviceSetupStatus, QString> DevicePluginWemo::setupDevice(Device *device)
{
    if(device->deviceClassId() == wemoSwitchDeviceClassId){
        foreach (WemoSwitch *wemoSwitch, m_wemoSwitches.keys()) {
            if(wemoSwitch->serialNumber() == device->paramValue("serial number").toString()){
                qWarning() << wemoSwitch->serialNumber() << " allready exists...";
                return reportDeviceSetup(DeviceManager::DeviceSetupStatusFailure,QString("Device allready added"));
            }
        }

        device->setName("WeMo Switch (" + device->paramValue("serial number").toString() + ")");

        WemoSwitch *wemoSwitch = new WemoSwitch(this);
        wemoSwitch->setName(device->paramValue("name").toString());
        wemoSwitch->setUuid(device->paramValue("uuid").toString());
        wemoSwitch->setPort(device->paramValue("port").toInt());
        wemoSwitch->setModelName(device->paramValue("model").toString());
        wemoSwitch->setHostAddress(QHostAddress(device->paramValue("host address").toString()));
        wemoSwitch->setModelDescription(device->paramValue("model description").toString());
        wemoSwitch->setSerialNumber(device->paramValue("serial number").toString());
        wemoSwitch->setLocation(QUrl(device->paramValue("location").toString()));
        wemoSwitch->setManufacturer(device->paramValue("manufacturer").toString());
        wemoSwitch->setDeviceType(device->paramValue("device type").toString());

        connect(wemoSwitch,SIGNAL(stateChanged()),this,SLOT(wemoSwitchStateChanged()));


        m_wemoSwitches.insert(wemoSwitch,device);
        return reportDeviceSetup();
    }
    return reportDeviceSetup(DeviceManager::DeviceSetupStatusSuccess);
}

DeviceManager::HardwareResources DevicePluginWemo::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

QPair<DeviceManager::DeviceError, QString> DevicePluginWemo::executeAction(Device *device, const Action &action)
{
    return report();
}

void DevicePluginWemo::deviceRemoved(Device *device)
{
    if (!m_wemoSwitches.values().contains(device)) {
        return;
    }

    WemoSwitch *wemoSwitch= m_wemoSwitches.key(device);
    qDebug() << "remove wemo swich " << wemoSwitch->serialNumber();
    m_wemoSwitches.remove(wemoSwitch);
}

QString DevicePluginWemo::pluginName() const
{
    return "WeMo";
}

PluginId DevicePluginWemo::pluginId() const
{
    return PluginId("2e3b5ce0-ecf1-43de-98f0-07df4068a583");
}

void DevicePluginWemo::guhTimer()
{
    foreach (WemoSwitch* device, m_wemoSwitches.keys()) {
        device->refresh();
    }
}

void DevicePluginWemo::discoveryDone(QList<WemoSwitch *> deviceList)
{
    QList<DeviceDescriptor> deviceDescriptors;
    foreach (WemoSwitch *device, deviceList) {
        DeviceDescriptor descriptor(wemoSwitchDeviceClassId, "WeMo Switch", device->serialNumber());
        ParamList params;
        params.append(Param("name", device->name()));
        params.append(Param("uuid", device->uuid()));
        params.append(Param("port", device->port()));
        params.append(Param("model", device->modelName()));
        params.append(Param("model description", device->modelDescription()));
        params.append(Param("serial number", device->serialNumber()));
        params.append(Param("host address", device->hostAddress().toString()));
        params.append(Param("location", device->location().toString()));
        params.append(Param("manufacturer", device->manufacturer()));
        params.append(Param("device type", device->deviceType()));
        descriptor.setParams(params);
        deviceDescriptors.append(descriptor);
    }
    emit devicesDiscovered(wemoSwitchDeviceClassId, deviceDescriptors);
}

void DevicePluginWemo::wemoSwitchStateChanged()
{




}

