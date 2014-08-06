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

#include "devicepluginlgsmarttv.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include <QDebug>

VendorId lgVendorId = VendorId("a9af9673-78db-4226-a16b-f34b304f7041");
DeviceClassId lgSmartTvDeviceClassId = DeviceClassId("1d41b5a8-74ff-4a12-b365-c7bbe610848f");

StateTypeId tvReachableStateTypeId = StateTypeId("b056c36b-df87-4177-8d5d-1e7c1e8cdc7a");

ActionTypeId commandVolumeUpActionTypeId = ActionTypeId("ac5d7dcd-dfe8-4a94-9ab9-21b3f804b39e");
ActionTypeId commandVolumeDownActionTypeId = ActionTypeId("62b17bec-f461-4ffa-93d1-67a9430d55e1");
ActionTypeId commandMuteActionTypeId = ActionTypeId("1aa9d7f0-0f66-4b90-bb72-f6b7b2118221");
ActionTypeId commandUnmuteActionTypeId = ActionTypeId("b7e31999-ba67-443d-8e5c-ec104af987bd");
ActionTypeId commandChannelUpActionTypeId = ActionTypeId("acd1f6a0-2cfa-4665-9607-cf94245ec5a3");
ActionTypeId commandChannelDownActionTypeId = ActionTypeId("6ea66772-0e6d-40b1-978c-a01fb53871dd");
ActionTypeId commandPowerOffActionTypeId = ActionTypeId("cbe41134-ff11-4916-815b-3ac289c64090");
ActionTypeId commandArrowUpActionTypeId = ActionTypeId("57c483b4-4ddf-4470-828c-8d8767e7a923");
ActionTypeId commandArrowDownActionTypeId = ActionTypeId("614cf1af-5cf7-4bb2-885c-4414078d8899");
ActionTypeId commandArrowLeftActionTypeId = ActionTypeId("916394dd-7833-4875-8d7a-49d7d24ceeb2");
ActionTypeId commandArrowRightActionTypeId = ActionTypeId("01e3df1e-638b-4e14-ba85-660267766062");
ActionTypeId commandOkActionTypeId = ActionTypeId("257dfa59-0d38-4e18-a3fc-213809fdb12f");
ActionTypeId commandBackActionTypeId = ActionTypeId("ce4184b3-6b8e-4fc3-a4cb-7b8ec72f2ce9");
ActionTypeId commandHomeActionTypeId = ActionTypeId("33f941c1-f5fc-4449-b6e3-93eafca493e0");
ActionTypeId commandInputSourceActionTypeId = ActionTypeId("9a6e5111-95d3-49ac-8056-249e704b1509");
ActionTypeId commandExitActionTypeId = ActionTypeId("d76efdb8-056e-4b39-a839-2ef6d6001b00");
ActionTypeId commandInfoActionTypeId = ActionTypeId("9c1290d5-3135-4124-a576-fc7522cffdcf");
ActionTypeId commandMyAppsActionTypeId = ActionTypeId("47d65cac-fe75-4c36-9dee-9862c1c1130e");
ActionTypeId commandProgramListActionTypeId = ActionTypeId("9aa3a97e-505d-4906-9764-14b6dc4e31e8");


DevicePluginLgSmartTv::DevicePluginLgSmartTv()
{
    m_discovery = new TvDiscovery(this);

    connect(m_discovery,SIGNAL(discoveryDone(QList<TvDevice*>)),this,SLOT(discoveryDone(QList<TvDevice*>)));
}

QList<Vendor> DevicePluginLgSmartTv::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor lgVendor(lgVendorId, "LG");
    ret.append(lgVendor);
    return ret;
}

QList<DeviceClass> DevicePluginLgSmartTv::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassLgSmartTv(pluginId(), lgVendorId, lgSmartTvDeviceClassId);
    deviceClassLgSmartTv.setName("LG Smart Tv");
    deviceClassLgSmartTv.setCreateMethod(DeviceClass::CreateMethodDiscovery);
    //deviceClassLgSmartTv.setSetupMethod(DeviceClass::SetupMethodDisplayPin);
    // TODO: display pin...

    // Params
    QList<ParamType> paramTypes;
    paramTypes.append(ParamType("name", QVariant::String));
    paramTypes.append(ParamType("uuid", QVariant::String));
    paramTypes.append(ParamType("model", QVariant::String));
    paramTypes.append(ParamType("host address", QVariant::String));
    paramTypes.append(ParamType("location", QVariant::String));
    paramTypes.append(ParamType("manufacturer", QVariant::String));
    paramTypes.append(ParamType("key", QVariant::String));

    deviceClassLgSmartTv.setParamTypes(paramTypes);

    // States
    QList<StateType> tvStates;

    StateType reachableState(tvReachableStateTypeId);
    reachableState.setName("reachable");
    reachableState.setType(QVariant::Bool);
    reachableState.setDefaultValue(false);
    tvStates.append(reachableState);

    deviceClassLgSmartTv.setStateTypes(tvStates);

    // Actions
    QList<ActionType> tvActions;

    ActionType commandVolumeUpAction(commandVolumeUpActionTypeId);
    commandVolumeUpAction.setName("volume up");
    tvActions.append(commandVolumeUpAction);

    ActionType commandVolumeDownAction(commandVolumeDownActionTypeId);
    commandVolumeDownAction.setName("volume down");
    tvActions.append(commandVolumeDownAction);

    ActionType commandMuteAction(commandMuteActionTypeId);
    commandMuteAction.setName("mute");
    tvActions.append(commandMuteAction);

    ActionType commandChannelUpAction(commandChannelUpActionTypeId);
    commandChannelUpAction.setName("channel up");
    tvActions.append(commandChannelUpAction);

    ActionType commandChannelDownAction(commandChannelDownActionTypeId);
    commandChannelDownAction.setName("channel down");
    tvActions.append(commandChannelDownAction);

    ActionType commandPowerOffAction(commandPowerOffActionTypeId);
    commandPowerOffAction.setName("power");
    tvActions.append(commandPowerOffAction);

    ActionType commandArrowUpAction(commandArrowUpActionTypeId);
    commandArrowUpAction.setName("arrow up");
    tvActions.append(commandArrowUpAction);

    ActionType commandArrowDownAction(commandArrowDownActionTypeId);
    commandArrowDownAction.setName("arrow down");
    tvActions.append(commandArrowDownAction);

    ActionType commandArrowLeftAction(commandArrowLeftActionTypeId);
    commandArrowLeftAction.setName("arrow left");
    tvActions.append(commandArrowLeftAction);

    ActionType commandArrowRightAction(commandArrowRightActionTypeId);
    commandArrowRightAction.setName("arrow right");
    tvActions.append(commandArrowRightAction);

    ActionType commandOkAction(commandOkActionTypeId);
    commandOkAction.setName("ok");
    tvActions.append(commandOkAction);

    ActionType commandBackAction(commandBackActionTypeId);
    commandBackAction.setName("back");
    tvActions.append(commandBackAction);

    ActionType commandHomeAction(commandHomeActionTypeId);
    commandHomeAction.setName("home");
    tvActions.append(commandHomeAction);

    ActionType commandInputSourceAction(commandInputSourceActionTypeId);
    commandInputSourceAction.setName("input source");
    tvActions.append(commandInputSourceAction);

    ActionType commandExitAction(commandExitActionTypeId);
    commandExitAction.setName("exit");
    tvActions.append(commandExitAction);

    ActionType commandInfoAction(commandInfoActionTypeId);
    commandInfoAction.setName("info");
    tvActions.append(commandInfoAction);

    ActionType commandMyAppsAction(commandMyAppsActionTypeId);
    commandMyAppsAction.setName("my apps");
    tvActions.append(commandMyAppsAction);

    ActionType commandProgramListAction(commandProgramListActionTypeId);
    commandProgramListAction.setName("program list");
    tvActions.append(commandProgramListAction);

    deviceClassLgSmartTv.setActions(tvActions);

    ret.append(deviceClassLgSmartTv);
    return ret;
}

QPair<DeviceManager::DeviceError, QString> DevicePluginLgSmartTv::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    qDebug() << "should discover devices with params:" << params;

    if(deviceClassId != lgSmartTvDeviceClassId){
        return report(DeviceManager::DeviceErrorDeviceClassNotFound);
    }

    m_discovery->discover(3000);

    return report(DeviceManager::DeviceErrorAsync);
}

QPair<DeviceManager::DeviceSetupStatus, QString> DevicePluginLgSmartTv::setupDevice(Device *device)
{

    device->setName("LG Smart Tv (" + device->paramValue("model").toString() + ")");

    TvDevice *tvDevice = new TvDevice(this);
    tvDevice->setName(device->paramValue("name").toString());
    tvDevice->setUuid(device->paramValue("uuid").toString());
    tvDevice->setModelName(device->paramValue("model").toString());
    tvDevice->setHostAddress(QHostAddress(device->paramValue("host address").toString()));
    tvDevice->setLocation(QUrl(device->paramValue("location").toString()));
    tvDevice->setUuid(device->paramValue("manufacturer").toString());
    // key if there is one...

    connect(tvDevice, &TvDevice::pairingFinished, this, &DevicePluginLgSmartTv::pairingFinished);
    connect(tvDevice, &TvDevice::sendCommandFinished, this, &DevicePluginLgSmartTv::sendingCommandFinished);

    tvDevice->requestPairing();
    m_tvList.insert(tvDevice,device);

    return reportDeviceSetup(DeviceManager::DeviceSetupStatusAsync);
}

DeviceManager::HardwareResources DevicePluginLgSmartTv::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

QPair<DeviceManager::DeviceError, QString> DevicePluginLgSmartTv::executeAction(Device *device, const Action &action)
{
    TvDevice * tvDevice = m_tvList.key(device);

    if(action.actionTypeId() == commandVolumeUpActionTypeId){
        tvDevice->sendCommand(TvDevice::VolUp, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandVolumeDownActionTypeId){
        tvDevice->sendCommand(TvDevice::VolDown, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandMuteActionTypeId){
        tvDevice->sendCommand(TvDevice::Mute, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandChannelUpActionTypeId){
        tvDevice->sendCommand(TvDevice::ChannelUp, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandChannelDownActionTypeId){
        tvDevice->sendCommand(TvDevice::ChannelDown, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandPowerOffActionTypeId){
        tvDevice->sendCommand(TvDevice::Power, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandArrowUpActionTypeId){
        tvDevice->sendCommand(TvDevice::Up, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandArrowDownActionTypeId){
        tvDevice->sendCommand(TvDevice::Down, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandArrowLeftActionTypeId){
        tvDevice->sendCommand(TvDevice::Left, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandArrowRightActionTypeId){
        tvDevice->sendCommand(TvDevice::Right, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandOkActionTypeId){
        tvDevice->sendCommand(TvDevice::Ok, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandBackActionTypeId){
        tvDevice->sendCommand(TvDevice::Back, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandHomeActionTypeId){
        tvDevice->sendCommand(TvDevice::Home, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandInputSourceActionTypeId){
        tvDevice->sendCommand(TvDevice::ExternalInput, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandExitActionTypeId){
        tvDevice->sendCommand(TvDevice::Exit, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandInfoActionTypeId){
        tvDevice->sendCommand(TvDevice::Info, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandMyAppsActionTypeId){
        tvDevice->sendCommand(TvDevice::MyApps, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }
    if(action.actionTypeId() == commandProgramListActionTypeId){
        tvDevice->sendCommand(TvDevice::ProgramList, action.id());
        return report(DeviceManager::DeviceErrorAsync);
    }

    return report(DeviceManager::DeviceErrorActionTypeNotFound);
}

void DevicePluginLgSmartTv::deviceRemoved(Device *device)
{
    if (!m_tvList.values().contains(device)) {
        return;
    }

    TvDevice *tvDevice= m_tvList.key(device);
    qDebug() << "remove LG Smart Tv  " << tvDevice->modelName();
    m_tvList.remove(tvDevice);
}

QString DevicePluginLgSmartTv::pluginName() const
{
    return "LG Smart Tv";
}

PluginId DevicePluginLgSmartTv::pluginId() const
{
    return PluginId("4ef7a68b-9da0-4c62-b9ac-f478dc6f9f52");
}

void DevicePluginLgSmartTv::guhTimer()
{

}

void DevicePluginLgSmartTv::discoveryDone(QList<TvDevice*> tvList)
{
    QList<DeviceDescriptor> deviceDescriptors;
    foreach (TvDevice *device, tvList) {
        DeviceDescriptor descriptor(lgSmartTvDeviceClassId, "Lg Smart Tv", device->modelName());
        ParamList params;
        params.append(Param("name", device->name()));
        params.append(Param("uuid", device->uuid()));
        params.append(Param("model", device->modelName()));
        params.append(Param("host address", device->hostAddress().toString()));
        params.append(Param("location", device->location().toString()));
        params.append(Param("manufacturer", device->manufacturer()));
        params.append(Param("key", device->key()));
        descriptor.setParams(params);
        deviceDescriptors.append(descriptor);
    }
    emit devicesDiscovered(lgSmartTvDeviceClassId, deviceDescriptors);
}

void DevicePluginLgSmartTv::pairingFinished(const bool &success)
{
    TvDevice *tvDevice = static_cast<TvDevice*>(sender());
    Device *device = m_tvList.value(tvDevice);

    if(success){
        emit deviceSetupFinished(device,DeviceManager::DeviceSetupStatusSuccess,QString(""));
    }else{
        emit deviceSetupFinished(device,DeviceManager::DeviceSetupStatusFailure,QString("Could not pair with tv."));
    }
}

void DevicePluginLgSmartTv::sendingCommandFinished(const bool &success, const ActionId &actionId)
{
    if(success){
        emit actionExecutionFinished(actionId,DeviceManager::DeviceErrorNoError,QString());
    }else{
        emit actionExecutionFinished(actionId,DeviceManager::DeviceErrorActionTypeNotFound,QString("Could not send command"));
    }
}


