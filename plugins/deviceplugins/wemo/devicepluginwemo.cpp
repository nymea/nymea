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

/*!
    \page wemo.html
    \title WeMo

    \ingroup plugins
    \ingroup network

    This plugin allows to find and controll devices from WeMo, the
    \l{http://www.belkin.com/de/PRODUKTE/home-automation/c/wemo-home-automation/}{Belkin}
    home automation system.


    \chapter Supported devices
        \section1 WeMo Switch
            Currently, there is just the \l{http://www.belkin.com/de/F7C027-Belkin/p/P-F7C027/}
            {WeMo Switch} supportet. For the first setup, the original app from WeMo (only availabel for
            iOS and Android Smartphones) will be needed. Once the WeMo switch is in the private network,
            the device can be discovered and controlled from guh.

            \e{\underline{Note:} when the WeMo Switch will not be reachabel, remove the device from guh and re-discover/re-add
            it, because the port of the device can change (between 49152-49155 so fare...).}

            \section2 Device propertys
                \section3 Device parameters
                Following list contains all device Params:
                    \table
                    \header
                        \li Name
                        \li Description
                        \li Data Type
                    \row
                        \li name
                        \li Holds the name of the device
                        \li string
                    \row
                        \li uuid
                        \li Holds the uuid of the device (set from the manufacturer)
                        \li string
                    \row
                        \li model
                        \li Describes the model
                        \li string
                    \row
                        \li host address
                        \li Holds the ip address from the device
                        \li string
                    \row
                        \li port
                        \li Holds the port, over which the switch is reachable
                        \li int
                    \row
                        \li model description
                        \li Holds the description of the model (given from the manufacturer)
                        \li string
                    \row
                        \li serial number
                        \li Holds the serial number of the WeMo Switch
                        \li string
                    \row
                        \li location
                        \li Holds the url of the device information page (XML)
                        \li string
                    \row
                        \li manufacturer
                        \li Holds the name of the manufacturer (Belkin International Inc.)
                        \li string
                    \row
                        \li device type
                        \li Holds the type of the device (urn:Belkin:device:controllee:1)
                        \li string
                    \endtable

                \section3 Device states
                Following list contains all device \l{State}s:
                    \table
                    \header
                        \li Name
                        \li Description
                        \li UUID
                        \li Data Type
                    \row
                        \li power
                        \li Thes state holds the power sate of the switch. If the WeMo Switch power state will be changed
                            outside of guh (with the button or with the WeMo app), guh will recognize that here.
                        \li 269f25eb-d0b7-4144-b9ef-801f4ff3e90c
                        \li bool
                    \row
                        \li reachable
                        \li Thes state gives the information, if the WeMo switch is currently reachable or not. If he's not
                            reachabel, it can't be controlled.
                        \li ec2f5b49-585c-4455-a233-b7aa4c608dbc
                        \li bool
                    \endtable

                \section3 Device actions
                Following list contains all device \l{Action}s:
                    \table
                    \header
                        \li Name
                        \li Description
                        \li UUID
                    \row
                        \li Set power
                        \li With this action you can switch WeMo ON (true) or OFF (false).
                        \li 269f25eb-d0b7-4144-b9ef-801f4ff3e90c
                    \endtable

*/

#include "devicepluginwemo.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include <QDebug>

DeviceClassId wemoSwitchDeviceClassId = DeviceClassId("69d97d3b-a8e6-42f3-afc0-ca8a53eb7cce");

StateTypeId powerStateTypeId = StateTypeId("7166c4f6-f68c-4188-8f7c-2205d72a5a6d");
StateTypeId reachableStateTypeId = StateTypeId("ec2f5b49-585c-4455-a233-b7aa4c608dbc");
ActionTypeId powerActionTypeId = ActionTypeId("269f25eb-d0b7-4144-b9ef-801f4ff3e90c");
ActionTypeId rediscoverActionTypeId = ActionTypeId("269cf3b8-d4dd-42e9-8309-6cb3ca8842df");

DevicePluginWemo::DevicePluginWemo()
{
}

DeviceManager::DeviceError DevicePluginWemo::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params);
    if (deviceClassId != wemoSwitchDeviceClassId) {
        return DeviceManager::DeviceErrorDeviceClassNotFound;
    }
    upnpDiscover("upnp:rootdevice");
    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::DeviceSetupStatus DevicePluginWemo::setupDevice(Device *device)
{
    if (device->deviceClassId() == wemoSwitchDeviceClassId) {
        foreach (WemoSwitch *wemoSwitch, m_wemoSwitches.keys()) {
            if (wemoSwitch->serialNumber() == device->paramValue("serial number").toString()) {
                qWarning() << "WeMo Switch " << wemoSwitch->serialNumber() << " allready added...";
                return DeviceManager::DeviceSetupStatusFailure;
            }
        }

        UpnpDeviceDescriptor upnpDeviceDescriptor;
        upnpDeviceDescriptor.setFriendlyName(device->paramValue("name").toString());
        upnpDeviceDescriptor.setHostAddress(QHostAddress(device->paramValue("host address").toString()));
        upnpDeviceDescriptor.setPort(device->paramValue("port").toInt());
        upnpDeviceDescriptor.setSerialNumber(device->paramValue("serial number").toString());

        device->setName("WeMo Switch (" + device->paramValue("serial number").toString() + ")");

        WemoSwitch *wemoSwitch = new WemoSwitch(this, upnpDeviceDescriptor);

        connect(wemoSwitch,SIGNAL(stateChanged()),this,SLOT(wemoSwitchStateChanged()));
        connect(wemoSwitch,SIGNAL(setPowerFinished(bool,ActionId)),this,SLOT(setPowerFinished(bool,ActionId)));

        m_wemoSwitches.insert(wemoSwitch,device);
        wemoSwitch->refresh();
        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}

DeviceManager::HardwareResources DevicePluginWemo::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer | DeviceManager::HardwareResourceUpnpDisovery;
}

DeviceManager::DeviceError DevicePluginWemo::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == wemoSwitchDeviceClassId) {
        if (action.actionTypeId() == powerActionTypeId) {
            WemoSwitch *wemoSwitch = m_wemoSwitches.key(device);
            if (wemoSwitch->reachable()) {
                // setPower returns false, if the curent powerState == new powerState
                if (wemoSwitch->setPower(action.param("power").value().toBool(), action.id())) {
                    return DeviceManager::DeviceErrorAsync;
                } else {
                    return DeviceManager::DeviceErrorNoError;
                }
            } else {
                return DeviceManager::DeviceErrorHardwareNotAvailable;
            }
        } else if (action.actionTypeId() == rediscoverActionTypeId) {
            upnpDiscover("upnp:rootdevice");
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginWemo::deviceRemoved(Device *device)
{
    if (!m_wemoSwitches.values().contains(device)) {
        return;
    }

    WemoSwitch *wemoSwitch= m_wemoSwitches.key(device);
    qDebug() << "remove wemo swich " << wemoSwitch->serialNumber();
    m_wemoSwitches.remove(wemoSwitch);
    wemoSwitch->deleteLater();
}

void DevicePluginWemo::guhTimer()
{
    foreach (WemoSwitch* wemoSwitch, m_wemoSwitches.keys()) {
        wemoSwitch->refresh();
    }
}

void DevicePluginWemo::upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList)
{
    QList<DeviceDescriptor> deviceDescriptors;
    foreach (UpnpDeviceDescriptor upnpDeviceDescriptor, upnpDeviceDescriptorList) {
        if (upnpDeviceDescriptor.friendlyName() == "WeMo Switch") {
            if (!verifyExistingDevices(upnpDeviceDescriptor)) {
                DeviceDescriptor descriptor(wemoSwitchDeviceClassId, "WemoSwitch", upnpDeviceDescriptor.serialNumber());
                ParamList params;
                params.append(Param("name", upnpDeviceDescriptor.friendlyName()));
                params.append(Param("host address", upnpDeviceDescriptor.hostAddress().toString()));
                params.append(Param("port", upnpDeviceDescriptor.port()));
                params.append(Param("serial number", upnpDeviceDescriptor.serialNumber()));
                descriptor.setParams(params);
                deviceDescriptors.append(descriptor);
            }
        }
    }
    emit devicesDiscovered(wemoSwitchDeviceClassId, deviceDescriptors);
}

void DevicePluginWemo::upnpNotifyReceived(const QByteArray &notifyData)
{
    Q_UNUSED(notifyData);
}

bool DevicePluginWemo::verifyExistingDevices(UpnpDeviceDescriptor deviceDescriptor)
{
    foreach (WemoSwitch *wemoSwitch, m_wemoSwitches.keys()) {
        // check if we allready have added this Wemo device and verify the params
        if (wemoSwitch->serialNumber() == deviceDescriptor.serialNumber()) {
            qDebug() << "verify wemo paramters... of" << wemoSwitch->serialNumber();
            Device *device = m_wemoSwitches.value(wemoSwitch);
            // now check if ip or port changed
            bool somethingChanged = false;
            if (wemoSwitch->hostAddress() != deviceDescriptor.hostAddress()) {
                device->setParamValue("host address", deviceDescriptor.hostAddress().toString());
                wemoSwitch->setHostAddress(deviceDescriptor.hostAddress());
                somethingChanged = true;
            }
            if(wemoSwitch->port() != deviceDescriptor.port()){
                device->setParamValue("port", deviceDescriptor.port());
                wemoSwitch->setPort(deviceDescriptor.port());
                somethingChanged = true;
            }
            if (somethingChanged) {
                wemoSwitch->refresh();
            }
            return true;
        }
    }
    return false;
}

void DevicePluginWemo::wemoSwitchStateChanged()
{
    WemoSwitch *wemoSwitch = static_cast<WemoSwitch*>(sender());

    if (m_wemoSwitches.contains(wemoSwitch)) {
        Device * device = m_wemoSwitches.value(wemoSwitch);
        device->setStateValue(powerStateTypeId, wemoSwitch->powerState());
        device->setStateValue(reachableStateTypeId, wemoSwitch->reachable());
    }
}

void DevicePluginWemo::setPowerFinished(const bool &succeeded, const ActionId &actionId)
{
    if (succeeded) {
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);
    } else {
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareNotAvailable);
    }
}

