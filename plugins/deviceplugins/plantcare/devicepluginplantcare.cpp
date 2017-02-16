/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2016 Bernhard Trinnes <bernhard.trinnes@guh.guru>        *
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
    \page plantcare.html
    \title Plantcare
    \brief Plugin for the guh Plantcare example based on 6LoWPAN networking.

    \ingroup plugins
    \ingroup guh-plugins-merkur

    This allowes to controll the guh plantcare demo for 6LoWPAN networks.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/plantcare/devicepluginplantcare.json
*/

#include "devicepluginplantcare.h"
#include "plugin/device.h"
#include "plugininfo.h"

DevicePluginPlantCare::DevicePluginPlantCare()
{

}

DeviceManager::HardwareResources DevicePluginPlantCare::requiredHardware() const
{
    // We need the NetworkAccessManager for node discovery and the timer for ping requests
    return DeviceManager::HardwareResourceNetworkManager | DeviceManager::HardwareResourceTimer;
}

DeviceManager::DeviceSetupStatus DevicePluginPlantCare::setupDevice(Device *device)
{
    qCDebug(dcPlantCare) << "Setup Plant Care" << device->name() << device->params();

    // Check if device already added with this address
    if (deviceAlreadyAdded(QHostAddress(device->paramValue(hostParamTypeId).toString()))) {
        qCWarning(dcPlantCare) << "Device with this address already added.";
        return DeviceManager::DeviceSetupStatusFailure;
    }

    // Create the CoAP socket if not already created
    if (m_coap.isNull()) {
        m_coap = new Coap(this);
        connect(m_coap.data(), SIGNAL(replyFinished(CoapReply*)), this, SLOT(coapReplyFinished(CoapReply*)));
        connect(m_coap.data(), SIGNAL(notificationReceived(CoapObserveResource,int,QByteArray)), this, SLOT(onNotificationReceived(CoapObserveResource,int,QByteArray)));
    }

    return DeviceManager::DeviceSetupStatusSuccess;
}

void DevicePluginPlantCare::deviceRemoved(Device *device)
{
    Q_UNUSED(device)

    // Delete the CoAP socket if there are no devices left
    if (myDevices().isEmpty()) {
        m_coap->deleteLater();
    }
}

DeviceManager::DeviceError DevicePluginPlantCare::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)

    // Perform a HTTP GET on the RPL router address
    QHostAddress address(configuration().paramValue(rplParamTypeId).toString());
    qCDebug(dcPlantCare) << "Scan for new nodes on RPL" << address.toString();

    QUrl url;
    url.setScheme("http");
    url.setHost(address.toString());

    m_asyncNodeScans.insert(networkManagerGet(QNetworkRequest(url)), deviceClassId);
    return DeviceManager::DeviceErrorAsync;
}

void DevicePluginPlantCare::networkManagerReplyReady(QNetworkReply *reply)
{
    if (m_asyncNodeScans.keys().contains(reply)) {
        DeviceClassId deviceClassId = m_asyncNodeScans.take(reply);
        // Check HTTP status code
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
            qCWarning(dcPlantCare) << "Node scan reply HTTP error:" << reply->errorString();
            emit devicesDiscovered(deviceClassId, QList<DeviceDescriptor>());
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        qCDebug(dcPlantCare) << "Node discovery finished:" << endl << data;

        QList<DeviceDescriptor> deviceDescriptors;
        QList<QByteArray> lines = data.split('\n');
        qCDebug(dcPlantCare) << lines;
        foreach (const QByteArray &line, lines) {
            if (line.isEmpty())
                continue;

            QHostAddress address(QString(line.left(line.length() - 4)));
            if (address.isNull())
                continue;

            qCDebug(dcPlantCare) << "Found node" << address.toString();
            // Create a deviceDescriptor for each found address
            DeviceDescriptor descriptor(deviceClassId, "Plant Care", address.toString());
            ParamList params;
            params.append(Param(hostParamTypeId, address.toString()));
            descriptor.setParams(params);
            deviceDescriptors.append(descriptor);
        }
        // Inform the user which devices were found
        emit devicesDiscovered(deviceClassId, deviceDescriptors);
    }

    // Delete the HTTP reply
    reply->deleteLater();
}

void DevicePluginPlantCare::postSetupDevice(Device *device)
{
    // Try to ping the device after a successful setup
    pingDevice(device);
}

void DevicePluginPlantCare::guhTimer()
{
    // Try to ping each device every 10 seconds to make sure it is still reachable
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == plantCareDeviceClassId) {
            pingDevice(device);
        }
    }
}

DeviceManager::DeviceError DevicePluginPlantCare::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() != plantCareDeviceClassId)
        return DeviceManager::DeviceErrorDeviceClassNotFound;

    qCDebug(dcPlantCare) << "Execute action" << device->name() << action.params();

    // Check if the device is reachable
    if (!device->stateValue(reachableStateTypeId).toBool()) {
        qCWarning(dcPlantCare) << "Device not reachable.";
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }

    // Check which action sould be executed
    if (action.actionTypeId() == toggleLedActionTypeId) {
        QUrl url;
        url.setScheme("coap");
        url.setHost(device->paramValue(hostParamTypeId).toString());
        url.setPath("/a/toggle");

        CoapReply *reply = m_coap->post(CoapRequest(url));
        if (reply->isFinished() && reply->error() != CoapReply::NoError) {
            qCWarning(dcPlantCare) << "CoAP reply finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            return DeviceManager::DeviceErrorHardwareFailure;
        }

        m_toggleLightRequests.insert(reply, action);
        m_asyncActions.insert(action.id(), device);
        return DeviceManager::DeviceErrorAsync;

    } else if(action.actionTypeId() == ledPowerActionTypeId) {
        int power = action.param(ledPowerStateParamTypeId).value().toInt();

        QUrl url;
        url.setScheme("coap");
        url.setHost(device->paramValue(hostParamTypeId).toString());
        url.setPath("/a/light");

        QByteArray payload = QString("pwm=%1").arg(QString::number(power)).toUtf8();
        qCDebug(dcPlantCare()) << "Sending" << payload << url.path();
        CoapReply *reply = m_coap->post(CoapRequest(url), payload);
        if (reply->isFinished() && reply->error() != CoapReply::NoError) {
            qCWarning(dcPlantCare) << "CoAP reply finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            return DeviceManager::DeviceErrorHardwareFailure;
        }

        m_setLedPower.insert(reply, action);
        m_asyncActions.insert(action.id(), device);
        return DeviceManager::DeviceErrorAsync;

    } else if(action.actionTypeId() == waterPumpActionTypeId) {
        bool pump = action.param(waterPumpStateParamTypeId).value().toBool();

        QUrl url;
        url.setScheme("coap");
        url.setHost(device->paramValue(hostParamTypeId).toString());
        url.setPath("/a/pump");

        QByteArray payload = QString("mode=%1").arg(QString::number((int)pump)).toUtf8();
        qCDebug(dcPlantCare()) << "Sending" << payload;
        CoapReply *reply = m_coap->post(CoapRequest(url), payload);
        if (reply->isFinished() && reply->error() != CoapReply::NoError) {
            qCWarning(dcPlantCare) << "CoAP reply finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            return DeviceManager::DeviceErrorHardwareFailure;
        }

        m_setPumpPower.insert(reply, action);
        m_asyncActions.insert(action.id(), device);
        return DeviceManager::DeviceErrorAsync;
    }
    return DeviceManager::DeviceErrorActionTypeNotFound;
}

void DevicePluginPlantCare::pingDevice(Device *device)
{
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue(hostParamTypeId).toString());
    m_pingReplies.insert(m_coap->ping(CoapRequest(url)), device);
}

void DevicePluginPlantCare::updateBattery(Device *device)
{
    qCDebug(dcPlantCare) << "Update" << device->name() << "battery value";
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue(hostParamTypeId).toString());
    url.setPath("/s/battery");
    CoapReply *reply = m_coap->get(CoapRequest(url));
    if (reply->isFinished() && reply->error() != CoapReply::NoError) {
        qCWarning(dcPlantCare) << "CoAP reply finished with error" << reply->errorString();
        setReachable(device, false);
        reply->deleteLater();
        return;
    }
    m_updateReplies.insert(reply, device);
}

void DevicePluginPlantCare::updateMoisture(Device *device)
{
    qCDebug(dcPlantCare) << "Update" << device->name() << "moisture value";
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue(hostParamTypeId).toString());
    url.setPath("/s/moisture");
    CoapReply *reply = m_coap->get(CoapRequest(url));
    if (reply->isFinished() && reply->error() != CoapReply::NoError) {
        qCWarning(dcPlantCare) << "CoAP reply finished with error" << reply->errorString();
        setReachable(device, false);
        reply->deleteLater();
        return;
    }

    m_updateReplies.insert(reply, device);
}

void DevicePluginPlantCare::updateWater(Device *device)
{
    qCDebug(dcPlantCare) << "Update" << device->name() << "water value";
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue(hostParamTypeId).toString());
    url.setPath("/s/water");
    CoapReply *reply = m_coap->get(CoapRequest(url));
    if (reply->isFinished() && reply->error() != CoapReply::NoError) {
        qCWarning(dcPlantCare) << "CoAP reply finished with error" << reply->errorString();
        setReachable(device, false);
        reply->deleteLater();
        return;
    }

    m_updateReplies.insert(reply, device);
}

void DevicePluginPlantCare::updateBrightness(Device *device)
{
    qCDebug(dcPlantCare) << "Update" << device->name() << "brightness value";
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue(hostParamTypeId).toString());
    url.setPath("/a/light");
    CoapReply *reply = m_coap->get(CoapRequest(url));
    if (reply->isFinished() && reply->error() != CoapReply::NoError) {
        qCWarning(dcPlantCare) << "CoAP reply finished with error" << reply->errorString();
        setReachable(device, false);
        reply->deleteLater();
        return;
    }

    m_updateReplies.insert(reply, device);
}

void DevicePluginPlantCare::updatePump(Device *device)
{
    qCDebug(dcPlantCare) << "Update" << device->name() << "pump value";
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue(hostParamTypeId).toString());
    url.setPath("/a/pump");
    CoapReply *reply = m_coap->get(CoapRequest(url));
    if (reply->isFinished() && reply->error() != CoapReply::NoError) {
        qCWarning(dcPlantCare) << "CoAP reply finished with error" << reply->errorString();
        setReachable(device, false);
        reply->deleteLater();
        return;
    }

    m_updateReplies.insert(reply, device);
}

void DevicePluginPlantCare::enableNotifications(Device *device)
{
    qCDebug(dcPlantCare) << "Enable" << device->name() << "notifications";
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue(hostParamTypeId).toString());

    url.setPath("/s/water");
    m_enableNotification.insert(m_coap->enableResourceNotifications(CoapRequest(url)), device);

    url.setPath("/s/moisture");
    m_enableNotification.insert(m_coap->enableResourceNotifications(CoapRequest(url)), device);

    url.setPath("/s/battery");
    m_enableNotification.insert(m_coap->enableResourceNotifications(CoapRequest(url)), device);

    url.setPath("/a/light");
    m_enableNotification.insert(m_coap->enableResourceNotifications(CoapRequest(url)), device);

    url.setPath("/a/pump");
    m_enableNotification.insert(m_coap->enableResourceNotifications(CoapRequest(url)), device);
}

void DevicePluginPlantCare::setReachable(Device *device, const bool &reachable)
{
    if (device->stateValue(reachableStateTypeId).toBool() != reachable) {
        if (!reachable) {
            // Warn just once that the device is not reachable
            qCWarning(dcPlantCare()) << device->name() << "reachable changed" << reachable;
        } else {
            qCDebug(dcPlantCare()) << device->name() << "reachable changed" << reachable;

            // Get current state values after a reconnect
            updateBattery(device);
            updateBrightness(device);
            updateMoisture(device);
            updateWater(device);
            updatePump(device);

            // Make sure the notifications are enabled
            enableNotifications(device);
        }
    }

    device->setStateValue(reachableStateTypeId, reachable);
}

bool DevicePluginPlantCare::deviceAlreadyAdded(const QHostAddress &address)
{
    // Check if we already have a device with the given address
    foreach (Device *device, myDevices()) {
        if (device->paramValue(hostParamTypeId).toString() == address.toString()) {
            return true;
        }
    }
    return false;
}

Device *DevicePluginPlantCare::findDevice(const QHostAddress &address)
{
    // Return the device pointer with the given address (otherwise 0)
    foreach (Device *device, myDevices()) {
        if (device->paramValue(hostParamTypeId).toString() == address.toString()) {
            return device;
        }
    }
    return NULL;
}

void DevicePluginPlantCare::coapReplyFinished(CoapReply *reply)
{
    if (m_pingReplies.contains(reply)) {
        Device *device = m_pingReplies.take(reply);

        // Check CoAP reply error
        if (reply->error() != CoapReply::NoError) {
            if (device->stateValue(reachableStateTypeId).toBool())
                qCWarning(dcPlantCare) << "Ping device" << reply->request().url().toString() << "reply finished with error" << reply->errorString();

            setReachable(device, false);
            reply->deleteLater();
            return;
        }
        setReachable(device, true);

    } else if (m_updateReplies.contains(reply)) {
        Device *device = m_updateReplies.take(reply);
        QString urlPath = reply->request().url().path();

        // Check CoAP reply error
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcPlantCare) << "Update resource" << urlPath << "reply finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            return;
        }

        // Check CoAP status code
        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcPlantCare) << "Update resource" << urlPath << "status code error:" << reply;
            reply->deleteLater();
            return;
        }

        // Update corresponding device state
        if (urlPath == "/s/moisture") {
            qCDebug(dcPlantCare()) << "Updated moisture value:" << reply->payload();
            device->setStateValue(moistureStateTypeId, qRound(reply->payload().toInt() * 100.0 / 1023.0));
        } else if (urlPath == "/s/water") {
            qCDebug(dcPlantCare()) << "Updated water value:" << reply->payload();
            device->setStateValue(waterStateTypeId, QVariant(reply->payload().toInt()).toBool());
        } else if (urlPath == "/s/battery") {
            qCDebug(dcPlantCare()) << "Updated battery value:" << reply->payload();
            device->setStateValue(batteryStateTypeId, reply->payload().toDouble());
        } else if (urlPath == "/a/pump") {
            qCDebug(dcPlantCare()) << "Updated pump value:" << reply->payload();
            device->setStateValue(waterPumpStateTypeId, QVariant(reply->payload().toInt()).toBool());
        } else if (urlPath == "/a/light") {
            qCDebug(dcPlantCare()) << "Updated led power value:" << reply->payload();
            int powerValue = reply->payload().toInt();
            if (powerValue > 0) {
                device->setStateValue(ledPowerStateTypeId, false);
            } else {
                device->setStateValue(ledPowerStateTypeId, true);
            }
        }

    } else if (m_toggleLightRequests.contains(reply)) {
        Action action = m_toggleLightRequests.take(reply);
        Device *device = m_asyncActions.take(action.id());

        // Check CoAP reply error
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcPlantCare) << "CoAP reply  toggle light finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            return;
        }

        // Check CoAP status code
        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcPlantCare) << "Toggle light status code error:" << reply;
            reply->deleteLater();
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            return;
        }
        // Tell the user about the action execution result
        emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorNoError);

    } else if (m_setLedPower.contains(reply)) {
        Action action = m_setLedPower.take(reply);
        Device *device = m_asyncActions.take(action.id());

        // check CoAP reply error
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcPlantCare) << "CoAP set led power reply finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            return;
        }

        // Check CoAP status code
        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcPlantCare) << "Set led power status code error:" << reply;
            reply->deleteLater();
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            return;
        }

        // Update the state here, so we don't have to wait for the notification
        device->setStateValue(ledPowerStateTypeId, action.param(ledPowerStateParamTypeId).value().toBool());
        // Tell the user about the action execution result
        emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorNoError);

    } else if (m_setPumpPower.contains(reply)) {
        Action action = m_setPumpPower.take(reply);
        Device *device = m_asyncActions.take(action.id());

        // check CoAP reply error
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcPlantCare) << "CoAP set pump power reply finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            return;
        }

        // Check CoAP status code
        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcPlantCare) << "Set pump power status code error:" << reply;
            reply->deleteLater();
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            return;
        }

        // Update the state here, so we don't have to wait for the notification
        device->setStateValue(waterPumpStateTypeId, action.param(waterPumpStateParamTypeId).value().toBool());
        // Tell the user about the action execution result
        emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorNoError);

    } else if (m_enableNotification.contains(reply)) {
        Device *device = m_enableNotification.take(reply);

        // check CoAP reply error
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcPlantCare) << "Enable notifications for" << reply->request().url().toString() << "reply finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            return;
        }

        // Check CoAP status code
        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcPlantCare) << "Enable notifications for" << reply->request().url().toString() << "reply status code error" << reply->errorString();
            reply->deleteLater();
            return;
        }

        qCDebug(dcPlantCare()) << "Enabled successfully notifications for" << device->name() << reply->request().url().path();
    }

    // Delete the CoAP reply
    reply->deleteLater();
}

void DevicePluginPlantCare::onNotificationReceived(const CoapObserveResource &resource, const int &notificationNumber, const QByteArray &payload)
{
    qCDebug(dcPlantCare) << " --> Got notification nr." << notificationNumber << resource.url().toString() << payload;
    Device *device = findDevice(QHostAddress(resource.url().host()));
    if (!device) {
        qCWarning(dcPlantCare()) << "Could not find device for this notification";
        return;
    }

    // Update the corresponding device state
    if (resource.url().path() == "/s/moisture") {
        device->setStateValue(moistureStateTypeId, qRound(payload.toInt() * 100.0 / 1023.0));
    } else if (resource.url().path() == "/s/water") {
        device->setStateValue(waterStateTypeId, QVariant(payload.toInt()).toBool());
    } else if (resource.url().path() == "/s/battery") {
        device->setStateValue(batteryStateTypeId, payload.toDouble());
    } else if (resource.url().path() == "/a/pump") {
        device->setStateValue(waterPumpStateTypeId, QVariant(payload.toInt()).toBool());
    } else if (resource.url().path() == "/a/light") {
        int powerValue = QVariant(payload).toInt();
        if (powerValue > 0) {
            device->setStateValue(ledPowerStateTypeId, false);
        } else {
            device->setStateValue(ledPowerStateTypeId, true);
        }
    }
}
