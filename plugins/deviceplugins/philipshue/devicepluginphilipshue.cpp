/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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
    \page philipshue.html
    \title Philips hue

    \ingroup plugins
    \ingroup network

    This plugin allows to interact with the \l{http://www2.meethue.com/}{Philips hue} bridge. Each light bulp connected to the bridge
    will appear automatically in the system, once the bridge is added to guh.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \note If a \l{StateType} has the parameter \tt{"writable": true}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/philipshue/devicepluginphilipshue.json
*/

#include "devicepluginphilipshue.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "types/param.h"
#include "huebridgeconnection.h"
#include "plugininfo.h"

#include <QDebug>
#include <QStringList>
#include <QColor>

DevicePluginPhilipsHue::DevicePluginPhilipsHue()
{
    m_bridge = new HueBridgeConnection(this);
    connect(m_bridge, &HueBridgeConnection::createUserFinished, this, &DevicePluginPhilipsHue::createUserFinished);
    connect(m_bridge, &HueBridgeConnection::getFinished, this, &DevicePluginPhilipsHue::getFinished);
}

DeviceManager::HardwareResources DevicePluginPhilipsHue::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer | DeviceManager::HardwareResourceUpnpDisovery;
}

void DevicePluginPhilipsHue::startMonitoringAutoDevices()
{
    // TODO: We could call the bridge to discover new light bulbs here maybe?
    // Although we maybe want to think of a user triggered approach to do such things.
}

QList<ParamType> DevicePluginPhilipsHue::configurationDescription() const
{
    QList<ParamType> params;
    return params;
}

DeviceManager::DeviceError DevicePluginPhilipsHue::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(deviceClassId)
    Q_UNUSED(params)
    upnpDiscover("libhue:idl");
    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::DeviceSetupStatus DevicePluginPhilipsHue::setupDevice(Device *device)
{
    //qDebug() << "setupDevice" << device->params();

    Light *light = nullptr;

    // Lets see if this a a newly added device... In which case its hue id number is not set, well, -1...
    if (device->paramValue("number").toInt() == -1) {
        if (m_unconfiguredLights.count() > 0) {
            light = m_unconfiguredLights.takeFirst();
            device->setParamValue("number", light->id());
            device->setParamValue("name", QString("Hue light %1").arg(light->id()));
        } else {
            // this shouldn't ever happen
            qWarning() << "Device not configured yet and no discovered devices around. This should not happen.";
            return DeviceManager::DeviceSetupStatusFailure;
        }
    } else {
        // In this case it most likely comes from the config. Just read all values from there...
        light = new Light(QHostAddress(device->paramValue("ip").toString()), device->paramValue("username").toString(), device->paramValue("number").toInt());
    }

    connect(light, &Light::stateChanged, this, &DevicePluginPhilipsHue::lightStateChanged);
    light->refresh();

    m_lights.insert(light, device);
    m_asyncSetups.insert(light, device);

    // If we have more unconfigured lights around, lets add them as auto devices
    QList<DeviceDescriptor> descriptorList;
    while (!m_unconfiguredLights.isEmpty()) {
        Light *light = m_unconfiguredLights.takeFirst();
        DeviceDescriptor descriptor(hueDeviceClassId, light->name());
        ParamList params;
        params.append(Param("name", light->name()));
        params.append(Param("number", light->id()));
        params.append(Param("ip", light->ip().toString()));
        params.append(Param("username", light->username()));
        descriptor.setParams(params);
        descriptorList.append(descriptor);
    }
    if (!descriptorList.isEmpty()) {
        metaObject()->invokeMethod(this, "autoDevicesAppeared", Qt::QueuedConnection, Q_ARG(DeviceClassId, hueDeviceClassId), Q_ARG(QList<DeviceDescriptor>, descriptorList));
    }

    return DeviceManager::DeviceSetupStatusAsync;
}

void DevicePluginPhilipsHue::deviceRemoved(Device *device)
{
    if (!m_lights.values().contains(device)) {
        return;
    }

    Light *light = m_lights.key(device);
    m_lights.remove(light);
    m_unconfiguredLights.append(light);
}

void DevicePluginPhilipsHue::upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList)
{
    qDebug() << "discovered bridges" << upnpDeviceDescriptorList.count();

    foreach (const UpnpDeviceDescriptor &descriptor, upnpDeviceDescriptorList) {
        qDebug() << descriptor;
    }

    QList<DeviceDescriptor> deviceDescriptors;
    foreach (const UpnpDeviceDescriptor &upnpDevice, upnpDeviceDescriptorList) {
        DeviceDescriptor descriptor(hueDeviceClassId, "Philips Hue bridge", upnpDevice.hostAddress().toString());
        ParamList params;
        params.append(Param("ip", upnpDevice.hostAddress().toString()));
        params.append(Param("username", "guh-" + QUuid::createUuid().toString().remove(QRegExp("[\\{\\}]*")).remove(QRegExp("\\-[0-9a-f\\-]*"))));
        params.append(Param("number", -1));
        descriptor.setParams(params);
        deviceDescriptors.append(descriptor);
    }

    emit devicesDiscovered(hueDeviceClassId, deviceDescriptors);
}

DeviceManager::DeviceSetupStatus DevicePluginPhilipsHue::confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(deviceClassId)
    Param ipParam;
    foreach (const Param &param, params) {
        if (param.name() == "ip") {
            ipParam = param;
        }
    }
    if (!ipParam.isValid()) {
        qWarning() << "Missing parameter: ip";
        return DeviceManager::DeviceSetupStatusFailure;
    }
    Param usernameParam;
    foreach (const Param &param, params) {
        if (param.name() == "username") {
            usernameParam = param;
        }
    }
    if (!usernameParam.isValid()) {
        qWarning() << "Missing parameter: username";
        return DeviceManager::DeviceSetupStatusFailure;
    }

    int id = m_bridge->createUser(QHostAddress(ipParam.value().toString()), usernameParam.value().toString());
    PairingInfo pi;
    pi.pairingTransactionId = pairingTransactionId;
    pi.ipParam = ipParam;
    pi.usernameParam = usernameParam;
    m_pairings.insert(id, pi);
    return DeviceManager::DeviceSetupStatusAsync;
}

void DevicePluginPhilipsHue::guhTimer()
{
    foreach (Light *light, m_lights.keys()) {
        light->refresh();
    }
}

DeviceManager::DeviceError DevicePluginPhilipsHue::executeAction(Device *device, const Action &action)
{
    Light *light = m_lights.key(device);
    if (!light) {
        return DeviceManager::DeviceErrorDeviceNotFound;
    }

    if (!light->reachable()) {
        qWarning() << "Hue Bulb not reachable";
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }

    if (action.actionTypeId() == hueColorActionTypeId) {
        light->setColor(action.param("color").value().value<QColor>());
    } else if (action.actionTypeId() == huePowerActionTypeId) {
        light->setOn(action.param("power").value().toBool());
    } else if (action.actionTypeId() == hueBrightnessActionTypeId) {
        light->setBri(percentageToBrightness(action.param("brightness").value().toInt()));
    }
    return DeviceManager::DeviceErrorNoError;
}


void DevicePluginPhilipsHue::createUserFinished(int id, const QVariant &response)
{
    qDebug() << "createuser response" << response;

    PairingInfo pairingInfo = m_pairings.take(id);
    if (response.toMap().contains("error")) {
        qDebug() << "Failed to pair Hue bridge:" << response.toMap().value("error").toMap().value("description");
        emit pairingFinished(pairingInfo.pairingTransactionId, DeviceManager::DeviceSetupStatusFailure);
        return;
    }

    // Paired successfully, check how many lightbulbs there are

    int getLightsId = m_bridge->get(QHostAddress(pairingInfo.ipParam.value().toString()), pairingInfo.usernameParam.value().toString(), "lights", this, "getLightsFinished");
    m_pairings.insert(getLightsId, pairingInfo);

}

void DevicePluginPhilipsHue::getLightsFinished(int id, const QVariant &params)
{
    qDebug() << "getlightsfinished" << params;
    PairingInfo pairingInfo = m_pairings.take(id);

    if (params.toMap().count() == 0) {
        qWarning() << "No light bulbs found on this hue bridge... Cannot proceed with pairing.";
        emit pairingFinished(pairingInfo.pairingTransactionId, DeviceManager::DeviceSetupStatusFailure);
        return;
    }

    // Store a list of all known Lights
    foreach (const QString &lightId, params.toMap().keys()) {
        Light *light = new Light(QHostAddress(pairingInfo.ipParam.value().toString()), pairingInfo.usernameParam.value().toString(), lightId.toInt(), this);
        m_unconfiguredLights.insert(lightId.toInt(), light);
    }

    emit pairingFinished(pairingInfo.pairingTransactionId, DeviceManager::DeviceSetupStatusSuccess);

    // If we have more than one device on that bridge, tell DeviceManager that there are more.
    if (params.toMap().count() > 1) {
//        emit autoDevicesAppeared();
    }
}

void DevicePluginPhilipsHue::getFinished(int id, const QVariant &params)
{
    qDebug() << "got lights" << params << id;
}

void DevicePluginPhilipsHue::lightStateChanged()
{
    Light *light = static_cast<Light*>(sender());

    Device *device;
    if (m_asyncSetups.contains(light)) {
        device = m_asyncSetups.take(light);
        device->setName(light->name());
        device->setParamValue("name", light->name());
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);
    } else {
        device = m_lights.value(light);
    }
    if (!device) {
        return;
    }
    device->setStateValue(hueReachableStateTypeId, light->reachable());
    device->setStateValue(hueColorStateTypeId, QVariant::fromValue(light->color()));
    device->setStateValue(huePowerStateTypeId, light->on());
    device->setStateValue(hueBrightnessStateTypeId, brightnessToPercentage(light->bri()));
}

int DevicePluginPhilipsHue::brightnessToPercentage(int brightness)
{
    return (int)(((100.0 * brightness) / 255.0) + 0.5);
}

int DevicePluginPhilipsHue::percentageToBrightness(int percentage)
{
    return (int)(((255.0 * percentage) / 100.0) + 0.5);
}
