/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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
    \page wemo.html
    \title WeMo
    \brief Plugin for Belkin WeMo sockets.

    \ingroup plugins
    \ingroup guh-plugins

    This plugin allows to find and controll devices from WeMo, the
    \l{http://www.belkin.com/de/PRODUKTE/home-automation/c/wemo-home-automation/}{Belkin}
    home automation system.

    \note: The devices can only be discovered if they are already in the local network. In order
    to configure the WeMo devices please use the original software.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

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
    if (device->deviceClassId() != wemoSwitchDeviceClassId)
        return DeviceManager::DeviceErrorDeviceClassNotFound;

    // Set power
    if (action.actionTypeId() == powerActionTypeId) {
        // Check if wemo device is reachable
        if (device->stateValue(reachableStateTypeId).toBool()) {
            // setPower returns false, if the curent powerState is allready the new powerState
            if (setPower(device, action.param(powerStateParamTypeId).value().toBool(), action.id())) {
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
            params.append(Param(nameParamTypeId, upnpDeviceDescriptor.friendlyName()));
            params.append(Param(hostParamTypeId, upnpDeviceDescriptor.hostAddress().toString()));
            params.append(Param(portParamTypeId, upnpDeviceDescriptor.port()));
            params.append(Param(serialParamTypeId, upnpDeviceDescriptor.serialNumber()));
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
    request.setUrl(QUrl("http://" + device->paramValue(hostParamTypeId).toString() + ":" + device->paramValue(portParamTypeId).toString() + "/upnp/control/basicevent1"));
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
    request.setUrl(QUrl("http://" + device->paramValue(hostParamTypeId).toString() + ":" + device->paramValue(portParamTypeId).toString() + "/upnp/control/basicevent1"));
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
