/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
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
    \page wemo.html
    \title WeMo

    \ingroup plugins
    \ingroup network

    This plugin allows to find and controll devices from WeMo, the
    \l{http://www.belkin.com/de/PRODUKTE/home-automation/c/wemo-home-automation/}{Belkin}
    home automation system.

    \note: The devices can only be discovered if they are already in the local network. In order
    to configure the WeMo devices please use the original software.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \note If a \l{StateType} has the parameter \tt{"writable": {...}}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/wemo/devicepluginwemo.json
*/

#include "devicepluginwemo.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>

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
    if (device->deviceClassId() != wemoSwitchDeviceClassId) {
        return DeviceManager::DeviceSetupStatusFailure;
    }

    refresh(device);
    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::HardwareResources DevicePluginWemo::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer | DeviceManager::HardwareResourceUpnpDisovery | DeviceManager::HardwareResourceNetworkManager;
}

DeviceManager::DeviceError DevicePluginWemo::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() != wemoSwitchDeviceClassId) {
        return DeviceManager::DeviceErrorDeviceClassNotFound;
    }

    // Set power
    if (action.actionTypeId() == powerActionTypeId) {
        // Check if wemo device is reachable
        if (device->stateValue(reachableStateTypeId).toBool()) {
            // setPower returns false, if the curent powerState is allready the new powerState
            if (setPower(device, action.param("power").value().toBool(), action.id())) {
                return DeviceManager::DeviceErrorAsync;
            } else {
                return DeviceManager::DeviceErrorNoError;
            }
        } else {
            return DeviceManager::DeviceErrorHardwareNotAvailable;
        }
    }

    return DeviceManager::DeviceErrorActionTypeNotFound;
}

void DevicePluginWemo::deviceRemoved(Device *device)
{
    // Check if there is a missing reply for this device
    foreach (Device *d, m_refreshReplies.values()) {
        if (d->id() == device->id()) {
            QNetworkReply * reply = m_refreshReplies.key(device);
            m_refreshReplies.remove(reply);
            // Note: delete will be done in networkManagerReplyReady()
        }
    }
    foreach (Device *d, m_setPowerReplies.values()) {
        if (d->id() == device->id()) {
            QNetworkReply * reply = m_setPowerReplies.key(device);
            m_setPowerReplies.remove(reply);
            // Note: delete will be done in networkManagerReplyReady()
        }
    }
}

void DevicePluginWemo::networkManagerReplyReady(QNetworkReply *reply)
{
    if (m_refreshReplies.contains(reply)) {
        QByteArray data = reply->readAll();
        Device *device = m_refreshReplies.take(reply);
        if (reply->error()) {
            // give only error if we don't already know that is unreachable
            if (device->stateValue(reachableStateTypeId).toBool()) {
                qCWarning(dcWemo) << "WeMo reply error: " << reply->errorString();
            }
            device->setStateValue(reachableStateTypeId, false);
        } else {
            processRefreshData(data, device);
        }
    } else if (m_setPowerReplies.contains(reply)) {
        QByteArray data = reply->readAll();
        Device *device = m_setPowerReplies.take(reply);
        ActionId actionId = m_runningActionExecutions.take(reply);
        if (reply->error()) {
            // give only error if we don't already know that is unreachable
            if (device->stateValue(reachableStateTypeId).toBool()) {
                qCWarning(dcWemo) << "WeMo reply error: " << reply->errorString();
            }
            device->setStateValue(reachableStateTypeId, false);
        } else {
            processSetPowerData(data, device, actionId);
        }
    }

    reply->deleteLater();
}

void DevicePluginWemo::guhTimer()
{
    foreach (Device* device, myDevices()) {
        refresh(device);
    }
}

void DevicePluginWemo::upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList)
{
    QList<DeviceDescriptor> deviceDescriptors;
    foreach (UpnpDeviceDescriptor upnpDeviceDescriptor, upnpDeviceDescriptorList) {
        if (upnpDeviceDescriptor.friendlyName() == "WeMo Switch") {
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
    emit devicesDiscovered(wemoSwitchDeviceClassId, deviceDescriptors);
}

void DevicePluginWemo::upnpNotifyReceived(const QByteArray &notifyData)
{
    Q_UNUSED(notifyData);
}


void DevicePluginWemo::refresh(Device *device)
{
    QByteArray getBinarayStateMessage("<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:GetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"><BinaryState>1</BinaryState></u:GetBinaryState></s:Body></s:Envelope>");

    QNetworkRequest request;
    request.setUrl(QUrl("http://" + device->paramValue("host address").toString() + ":" + device->paramValue("port").toString() + "/upnp/control/basicevent1"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=\"utf-8\""));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("guh"));
    request.setRawHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#GetBinaryState\"");

    QNetworkReply *reply = networkManagerPost(request, getBinarayStateMessage);
    m_refreshReplies.insert(reply, device);
}

bool DevicePluginWemo::setPower(Device *device, const bool &power, const ActionId &actionId)
{
    // check if the power would change...
    if (device->stateValue(powerStateTypeId).toBool() == power) {
        return false;
    }

    QByteArray setPowerMessage("<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"><BinaryState>" + QByteArray::number((int)power) + "</BinaryState></u:SetBinaryState></s:Body></s:Envelope>");

    QNetworkRequest request;
    request.setUrl(QUrl("http://" + device->paramValue("host address").toString() + ":" + device->paramValue("port").toString() + "/upnp/control/basicevent1"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=\"utf-8\""));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("guh"));
    request.setRawHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#SetBinaryState\"");

    QNetworkReply *reply = networkManagerPost(request, setPowerMessage);
    m_setPowerReplies.insert(reply, device);
    m_runningActionExecutions.insert(reply, actionId);
    return true;
}


void DevicePluginWemo::processRefreshData(const QByteArray &data, Device *device)
{
    if (data.contains("<BinaryState>0</BinaryState>")) {
        device->setStateValue(powerStateTypeId, false);
        device->setStateValue(reachableStateTypeId, true);
    } else if (data.contains("<BinaryState>1</BinaryState>")) {
        device->setStateValue(powerStateTypeId, true);
        device->setStateValue(reachableStateTypeId, true);
    } else {
        device->setStateValue(reachableStateTypeId, false);
    }
}

void DevicePluginWemo::processSetPowerData(const QByteArray &data, Device *device, const ActionId &actionId)
{
    if (data.contains("<BinaryState>1</BinaryState>") || data.contains("<BinaryState>0</BinaryState>")) {
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);
        device->setStateValue(reachableStateTypeId, true);
        refresh(device);
    } else {
        device->setStateValue(reachableStateTypeId, false);
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareNotAvailable);
    }
}
