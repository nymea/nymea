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

#include "devicepluginmock.h"
#include "httpdaemon.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include <QDebug>
#include <QStringList>

PluginId mockPluginId = PluginId("727a4a9a-c187-446f-aadf-f1b2220607d1");
VendorId guhVendorId = VendorId("2062d64d-3232-433c-88bc-0d33c0ba2ba6");
DeviceClassId mockDeviceClassId = DeviceClassId("753f0d32-0468-4d08-82ed-1964aab03298");
DeviceClassId mockDeviceAutoClassId = DeviceClassId("ab4257b3-7548-47ee-9bd4-7dc3004fd197");
DeviceClassId mockDeviceDiscoveryClassId = DeviceClassId("1bbaf751-36b7-4d3d-b05a-58dab2a3be8c");
DeviceClassId mockDeviceAsyncSetupClassId = DeviceClassId("c08a8b27-8200-413d-b96b-4cff78b864d9");
DeviceClassId mockDeviceBrokenClassId = DeviceClassId("ba5fb404-c9ce-4db4-8cd4-f48c61c24b13");
DeviceClassId mockDeviceBrokenAsyncSetupClassId = DeviceClassId("bd5b78c5-53c9-4417-8eac-8ab2bce97bd0");
EventTypeId mockEvent1Id = EventTypeId("45bf3752-0fc6-46b9-89fd-ffd878b5b22b");
EventTypeId mockEvent2Id = EventTypeId("863d5920-b1cf-4eb9-88bd-8f7b8583b1cf");
StateTypeId mockIntStateId = StateTypeId("80baec19-54de-4948-ac46-31eabfaceb83");
StateTypeId mockBoolStateId = StateTypeId("9dd6a97c-dfd1-43dc-acbd-367932742310");
ActionTypeId mockActionIdWithParams = ActionTypeId("dea0f4e1-65e3-4981-8eaa-2701c53a9185");
ActionTypeId mockActionIdNoParams = ActionTypeId("defd3ed6-1a0d-400b-8879-a0202cf39935");
ActionTypeId mockActionIdAsync = ActionTypeId("fbae06d3-7666-483e-a39e-ec50fe89054e");
ActionTypeId mockActionIdFailing = ActionTypeId("df3cf33d-26d5-4577-9132-9823bd33fad0");
ActionTypeId mockActionIdAsyncFailing = ActionTypeId("bfe89a1d-3497-4121-8318-e77c37537219");

DevicePluginMock::DevicePluginMock()
{
}

DevicePluginMock::~DevicePluginMock()
{
}

QList<Vendor> DevicePluginMock::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor guh(guhVendorId, "guh");
    ret.append(guh);
    return ret;
}

QList<DeviceClass> DevicePluginMock::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassMock(pluginId(), guhVendorId, mockDeviceClassId);
    deviceClassMock.setName("Mock Device");

    QList<ParamType> mockParams;
    ParamType portParam("httpport", QVariant::Int);
    mockParams.append(portParam);

    deviceClassMock.setParamTypes(mockParams);

    QList<StateType> mockStates;

    StateType intState(mockIntStateId);
    intState.setName("Dummy int state");
    intState.setType(QVariant::Int);
    intState.setDefaultValue(10);
    mockStates.append(intState);

    StateType boolState(mockBoolStateId);
    boolState.setName("Dummy bool state");
    boolState.setType(QVariant::Int);
    boolState.setDefaultValue(false);
    mockStates.append(boolState);

    deviceClassMock.setStateTypes(mockStates);

    QList<EventType> mockEvents;
    
    EventType event1(mockEvent1Id);
    event1.setName("Mock Event 1");
    mockEvents.append(event1);

    EventType event2(mockEvent2Id);
    event2.setName("Mock Event 2");
    QList<ParamType> event2ParamTypes;
    ParamType event2Param1Type("mockParamInt", QVariant::Int, 42);
    event2ParamTypes.append(event2Param1Type);
    event2.setParameters(event2ParamTypes);
    mockEvents.append(event2);

    deviceClassMock.setEventTypes(mockEvents);

    QList<ActionType> mockActions;

    mockParams.clear();
    ActionType action1(mockActionIdWithParams);
    action1.setName("Mock Action 1 (with params)");
    ParamType mockActionParam1("mockActionParam1", QVariant::Int);
    mockParams.append(mockActionParam1);
    ParamType mockActionParam2("mockActionParam2", QVariant::Bool);
    mockParams.append(mockActionParam2);
    action1.setParameters(mockParams);
    mockActions.append(action1);

    ActionType action2(mockActionIdNoParams);
    action2.setName("Mock Action 3 (without params)");
    mockActions.append(action2);

    ActionType action3(mockActionIdAsync);
    action3.setName("Mock Action 3 (async)");
    mockActions.append(action3);

    ActionType action4(mockActionIdFailing);
    action4.setName("Mock Action 4 (broken)");
    mockActions.append(action4);

    ActionType action5(mockActionIdAsyncFailing);
    action5.setName("Mock Action 5 (async, broken)");
    mockActions.append(action4);

    deviceClassMock.setActions(mockActions);

    ret.append(deviceClassMock);

    // Auto created mock device
    DeviceClass deviceClassMockAuto(pluginId(), guhVendorId, mockDeviceAutoClassId);
    deviceClassMockAuto.setName("Mock Device (Auto created)");
    deviceClassMockAuto.setCreateMethod(DeviceClass::CreateMethodAuto);

    mockParams.clear();
    deviceClassMockAuto.setParamTypes(mockParams);
    deviceClassMockAuto.setStateTypes(mockStates);
    deviceClassMockAuto.setEventTypes(mockEvents);
    deviceClassMockAuto.setActions(mockActions);

    ret.append(deviceClassMockAuto);

    // Discovery created device
    DeviceClass deviceClassMockDiscovery(pluginId(), guhVendorId, mockDeviceDiscoveryClassId);
    deviceClassMockDiscovery.setName("Mock Device (Discovery created)");
    deviceClassMockDiscovery.setCreateMethod(DeviceClass::CreateMethodDiscovery);

    mockParams.clear();
    mockParams.append(portParam);
    deviceClassMockDiscovery.setParamTypes(mockParams);
    deviceClassMockDiscovery.setStateTypes(mockStates);
    deviceClassMockDiscovery.setEventTypes(mockEvents);
    deviceClassMockDiscovery.setActions(mockActions);

    ret.append(deviceClassMockDiscovery);

    // Async setup device
    DeviceClass deviceClassMockAsync(pluginId(), guhVendorId, mockDeviceAsyncSetupClassId);
    deviceClassMockAsync.setName("Mock Device (Async)");
    deviceClassMockAsync.setCreateMethod(DeviceClass::CreateMethodUser);

    deviceClassMockAsync.setParamTypes(mockParams);
    deviceClassMockAsync.setStateTypes(mockStates);
    deviceClassMockAsync.setEventTypes(mockEvents);
    deviceClassMockAsync.setActions(mockActions);

    ret.append(deviceClassMockAsync);

    // Async setup device
    DeviceClass deviceClassMockBroken(pluginId(), guhVendorId, mockDeviceBrokenClassId);
    deviceClassMockBroken.setName("Mock Device (Broken setup)");
    deviceClassMockBroken.setCreateMethod(DeviceClass::CreateMethodUser);

    deviceClassMockBroken.setParamTypes(mockParams);
    deviceClassMockBroken.setStateTypes(mockStates);
    deviceClassMockBroken.setEventTypes(mockEvents);
    deviceClassMockBroken.setActions(mockActions);

    ret.append(deviceClassMockBroken);

    // Broken Async setup device
    DeviceClass deviceClassMockBrokenAsyncSetup(pluginId(), guhVendorId, mockDeviceBrokenAsyncSetupClassId);
    deviceClassMockBrokenAsyncSetup.setName("Mock Device (Async Broken setup)");
    deviceClassMockBrokenAsyncSetup.setCreateMethod(DeviceClass::CreateMethodUser);

    deviceClassMockBrokenAsyncSetup.setParamTypes(mockParams);
    deviceClassMockBrokenAsyncSetup.setStateTypes(mockStates);
    deviceClassMockBrokenAsyncSetup.setEventTypes(mockEvents);
    deviceClassMockBrokenAsyncSetup.setActions(mockActions);

    ret.append(deviceClassMockBrokenAsyncSetup);

    return ret;
}

DeviceManager::HardwareResources DevicePluginMock::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

DeviceManager::DeviceError DevicePluginMock::discoverDevices(const DeviceClassId &deviceClassId, const QList<Param> &params) const
{
    Q_UNUSED(deviceClassId)
    Q_UNUSED(params)
    QTimer::singleShot(1000, this, SLOT(emitDevicesDiscovered()));
    return DeviceManager::DeviceErrorNoError;
}

QString DevicePluginMock::pluginName() const
{
    return "Mock Devices";
}

PluginId DevicePluginMock::pluginId() const
{
    return mockPluginId;
}

QPair<DeviceManager::DeviceSetupStatus, QString> DevicePluginMock::setupDevice(Device *device)
{
    qDebug() << "Mockdevice created returning true" << device->paramValue("httpport").toInt();

    if (device->deviceClassId() == mockDeviceBrokenClassId) {
        return reportDeviceSetup(DeviceManager::DeviceSetupStatusFailure, "This device is intentionally broken.");
    }

    HttpDaemon *daemon = new HttpDaemon(device, this);
    m_daemons.insert(device, daemon);

    if (!daemon->isListening()) {
        qDebug() << "HTTP port opening failed.";
        return reportDeviceSetup(DeviceManager::DeviceSetupStatusFailure, QString("Could not bind port."));
    }

    connect(daemon, &HttpDaemon::triggerEvent, this, &DevicePluginMock::triggerEvent);
    connect(daemon, &HttpDaemon::setState, this, &DevicePluginMock::setState);

    if (device->deviceClassId() == mockDeviceAsyncSetupClassId || device->deviceClassId() == mockDeviceBrokenAsyncSetupClassId) {
        m_asyncSetupDevices.append(device);
        QTimer::singleShot(1000, this, SLOT(emitDeviceSetupFinished()));
        return reportDeviceSetup(DeviceManager::DeviceSetupStatusAsync);
    }
    return reportDeviceSetup();
}

void DevicePluginMock::deviceRemoved(Device *device)
{
    delete m_daemons.take(device);
}

void DevicePluginMock::startMonitoringAutoDevices()
{
    DeviceDescriptor mockDescriptor(mockDeviceAutoClassId, "Mock Device (Auto created)");

    QList<Param> params;
    Param param("httpport", 4242);
    params.append(param);
    mockDescriptor.setParams(params);

    QList<DeviceDescriptor> deviceDescriptorList;
    deviceDescriptorList.append(mockDescriptor);

    emit autoDevicesAppeared(mockDeviceAutoClassId, deviceDescriptorList);
}

QList<ParamType> DevicePluginMock::configurationDescription() const
{
    QList<ParamType> params;
    ParamType mockParam1("configParamInt", QVariant::Int, 42);
    mockParam1.setLimits(1, 50);
    params.append(mockParam1);

    ParamType mockParam2("configParamBool", QVariant::Bool, true);
    params.append(mockParam2);

    return params;
}

QPair<DeviceManager::DeviceError, QString> DevicePluginMock::executeAction(Device *device, const Action &action)
{
    if (!myDevices().contains(device)) {
        qWarning() << "Should execute action for a device which doesn't seem to be mine.";
        return report(DeviceManager::DeviceErrorDeviceNotFound, "Should execute an action for a device which doesn't seem to be mine.");
    }

    if (action.actionTypeId() == mockActionIdAsync || action.actionTypeId() == mockActionIdAsyncFailing) {
        m_asyncActions.append(qMakePair<Action, Device*>(action, device));
        QTimer::singleShot(1000, this, SLOT(emitActionExecuted()));
        return report(DeviceManager::DeviceErrorAsync);
    }

    if (action.actionTypeId() == mockActionIdFailing) {
        return report(DeviceManager::DeviceErrorSetupFailed);
    }

    qDebug() << "Should execute action" << action.actionTypeId();
    m_daemons.value(device)->actionExecuted(action.actionTypeId());
    return report();
}

void DevicePluginMock::setState(const StateTypeId &stateTypeId, const QVariant &value)
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon) {
        return;
    }

    Device *device = m_daemons.key(daemon);
    device->setStateValue(stateTypeId, value);
}

void DevicePluginMock::triggerEvent(const EventTypeId &id)
{
    HttpDaemon *daemon = qobject_cast<HttpDaemon*>(sender());
    if (!daemon) {
        return;
    }

    Device *device = m_daemons.key(daemon);

    Event event(id, device->id());

    qDebug() << "Emitting event " << event.eventTypeId();
    emit emitEvent(event);
}

void DevicePluginMock::emitDevicesDiscovered()
{
    QList<DeviceDescriptor> deviceDescriptors;

    DeviceDescriptor d1(mockDeviceDiscoveryClassId, "Mock Device (Discovered)");
    QList<Param> params;
    Param httpParam("httpport", "7777");
    params.append(httpParam);
    d1.setParams(params);
    deviceDescriptors.append(d1);

    DeviceDescriptor d2(mockDeviceDiscoveryClassId, "Mock Device (Discovered)");
    params.clear();
    httpParam.setValue("7778");
    params.append(httpParam);
    d2.setParams(params);
    deviceDescriptors.append(d2);

    emit devicesDiscovered(mockDeviceDiscoveryClassId, deviceDescriptors);
}

void DevicePluginMock::emitDeviceSetupFinished()
{
    qDebug() << "emitting setup finised";
    Device *device = m_asyncSetupDevices.takeFirst();
    if (device->deviceClassId() == mockDeviceAsyncSetupClassId) {
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess, QString());
    } else {
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure, QString("This device is intentionally broken"));
    }
}

void DevicePluginMock::emitActionExecuted()
{
    QPair<Action, Device*> action = m_asyncActions.takeFirst();
    if (action.first.actionTypeId() == mockActionIdAsync) {
        m_daemons.value(action.second)->actionExecuted(action.first.actionTypeId());
        emit actionExecutionFinished(action.first.id(), DeviceManager::DeviceErrorNoError, QString());
    } else if (action.first.actionTypeId() == mockActionIdAsyncFailing) {
        emit actionExecutionFinished(action.first.id(), DeviceManager::DeviceErrorSetupFailed, QString());
    }
}
