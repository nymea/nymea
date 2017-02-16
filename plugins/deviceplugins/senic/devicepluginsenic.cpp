/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \page senic.html
    \title Senic - Nuimo
    \brief Plugin for Senic Nuimo.

    \ingroup plugins
    \ingroup guh-plugins

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/senic/devicepluginsenic.json
*/

#ifdef BLUETOOTH_LE

#include "devicepluginsenic.h"
#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

DevicePluginSenic::DevicePluginSenic()
{

}

DeviceManager::DeviceError DevicePluginSenic::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)

    if (deviceClassId != nuimoDeviceClassId)
        return DeviceManager::DeviceErrorDeviceClassNotFound;

    if (!discoverBluetooth())
        return DeviceManager::DeviceErrorHardwareNotAvailable;

    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::DeviceSetupStatus DevicePluginSenic::setupDevice(Device *device)
{
    QString name = device->paramValue(nameParamTypeId).toString();
    QBluetoothAddress address = QBluetoothAddress(device->paramValue(macParamTypeId).toString());
    QBluetoothDeviceInfo deviceInfo = QBluetoothDeviceInfo(address, name, 0);

    Nuimo *nuimo = new Nuimo(deviceInfo, QLowEnergyController::RandomAddress, this);
    connect(nuimo, &Nuimo::availableChanged, this, &DevicePluginSenic::connectionAvailableChanged);
    connect(nuimo, &Nuimo::batteryValueChaged, this, &DevicePluginSenic::onBatteryValueChanged);
    connect(nuimo, &Nuimo::buttonPressed, this, &DevicePluginSenic::onButtonPressed);
    connect(nuimo, &Nuimo::buttonReleased, this, &DevicePluginSenic::onButtonReleased);
    connect(nuimo, &Nuimo::swipeDetected, this, &DevicePluginSenic::onSwipeDetected);
    connect(nuimo, &Nuimo::rotationValueChanged, this, &DevicePluginSenic::onRotationValueChanged);

    m_nuimos.insert(nuimo, device);

    nuimo->connectDevice();

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::HardwareResources DevicePluginSenic::requiredHardware() const
{
    return DeviceManager::HardwareResourceBluetoothLE;
}

DeviceManager::DeviceError DevicePluginSenic::executeAction(Device *device, const Action &action)
{
    QPointer<Nuimo> nuimo = m_nuimos.key(device);
    if (nuimo.isNull())
        return DeviceManager::DeviceErrorHardwareFailure;

    // reconnect action does not need available true
    if (action.actionTypeId() == connectActionTypeId) {
        nuimo->reconnectDevice();
        return DeviceManager::DeviceErrorNoError;
    }

    if (action.actionTypeId() == disconnectActionTypeId) {
        nuimo->disconnectDevice();
        return DeviceManager::DeviceErrorNoError;
    }

    if (action.actionTypeId() == showLogoActionTypeId) {

        if (action.param(logoParamTypeId).value().toString() == "Guh")
            nuimo->showGuhLogo();

        if (action.param(logoParamTypeId).value().toString() == "Arrow up")
            nuimo->showArrowUp();

        if (action.param(logoParamTypeId).value().toString() == "Arrow down")
            nuimo->showArrowDown();

        return DeviceManager::DeviceErrorNoError;
    }

    return DeviceManager::DeviceErrorActionTypeNotFound;
}

void DevicePluginSenic::bluetoothDiscoveryFinished(const QList<QBluetoothDeviceInfo> &deviceInfos)
{
    QList<DeviceDescriptor> deviceDescriptors;
    foreach (QBluetoothDeviceInfo deviceInfo, deviceInfos) {
        if (deviceInfo.name().contains("Nuimo")) {
            if (!verifyExistingDevices(deviceInfo)) {
                DeviceDescriptor descriptor(nuimoDeviceClassId, "Nuimo", deviceInfo.address().toString());
                ParamList params;
                params.append(Param(nameParamTypeId, deviceInfo.name()));
                params.append(Param(macParamTypeId, deviceInfo.address().toString()));
                descriptor.setParams(params);
                deviceDescriptors.append(descriptor);
            }
        }
    }

    emit devicesDiscovered(nuimoDeviceClassId, deviceDescriptors);
}

void DevicePluginSenic::deviceRemoved(Device *device)
{
    if (!m_nuimos.values().contains(device))
        return;

    Nuimo *nuimo = m_nuimos.key(device);
    m_nuimos.take(nuimo);
    delete nuimo;
}

bool DevicePluginSenic::verifyExistingDevices(const QBluetoothDeviceInfo &deviceInfo)
{
    foreach (Device *device, myDevices()) {
        if (device->paramValue(macParamTypeId).toString() == deviceInfo.address().toString())
            return true;
    }

    return false;
}

void DevicePluginSenic::connectionAvailableChanged()
{
    Nuimo *nuimo = static_cast<Nuimo *>(sender());
    Device *device = m_nuimos.value(nuimo);
    device->setStateValue(availableStateTypeId, nuimo->isAvailable());
}

void DevicePluginSenic::onBatteryValueChanged(const uint &percentage)
{
    Nuimo *nuimo = static_cast<Nuimo *>(sender());
    Device *device = m_nuimos.value(nuimo);
    device->setStateValue(batteryStateTypeId, percentage);
}

void DevicePluginSenic::onButtonPressed()
{
    Nuimo *nuimo = static_cast<Nuimo *>(sender());
    Device *device = m_nuimos.value(nuimo);
    emitEvent(Event(clickedEventTypeId, device->id()));
}

void DevicePluginSenic::onButtonReleased()
{
    // TODO: user timer for detekt long pressed (if needed)
}

void DevicePluginSenic::onSwipeDetected(const Nuimo::SwipeDirection &direction)
{
    Nuimo *nuimo = static_cast<Nuimo *>(sender());
    Device *device = m_nuimos.value(nuimo);

    switch (direction) {
    case Nuimo::SwipeDirectionLeft:
        emitEvent(Event(swipeLeftEventTypeId, device->id()));
        break;
    case Nuimo::SwipeDirectionRight:
        emitEvent(Event(swipeRightEventTypeId, device->id()));
        break;
    case Nuimo::SwipeDirectionUp:
        emitEvent(Event(swipeUpEventTypeId, device->id()));
        break;
    case Nuimo::SwipeDirectionDown:
        emitEvent(Event(swipeDownEventTypeId, device->id()));
        break;
    default:
        break;
    }
}

void DevicePluginSenic::onRotationValueChanged(const uint &value)
{
    Nuimo *nuimo = static_cast<Nuimo *>(sender());
    Device *device = m_nuimos.value(nuimo);
    device->setStateValue(rotationStateTypeId, value);
}


#endif // BLUETOOTH_LE


