/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2016 Bernhard Trinnes <bernhard.trinnes@guh.guru>        *
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
    \page orderbutton.html
    \title Orderbutton
    \brief Demonstration of an orderbutton based on 6LoWPAN networking.

    \ingroup plugins
    \ingroup guh-plugins-merkur


    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/orderbutton/devicepluginorderbutton.json
*/

#include "devicepluginorderbutton.h"
#include "plugin/device.h"
#include "plugininfo.h"

DevicePluginOrderButton::DevicePluginOrderButton()
{

}

DeviceManager::HardwareResources DevicePluginOrderButton::requiredHardware() const
{
    // We need the NetworkManager for node discovery and the timer for ping requests
    return DeviceManager::HardwareResourceNetworkManager | DeviceManager::HardwareResourceTimer;
}

DeviceManager::DeviceSetupStatus DevicePluginOrderButton::setupDevice(Device *device)
{
    qCDebug(dcOrderButton) << "Setup Plant Care" << device->name() << device->params();

    // Check if device already added with this address
    if (deviceAlreadyAdded(QHostAddress(device->paramValue("host").toString()))) {
        qCWarning(dcOrderButton) << "Device with this address already added.";
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

void DevicePluginOrderButton::deviceRemoved(Device *device)
{
    Q_UNUSED(device)

    // Delete the CoAP socket if there are no devices left
    if (myDevices().isEmpty()) {
        m_coap->deleteLater();
    }
}

DeviceManager::DeviceError DevicePluginOrderButton::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params)

    // Perform a HTTP GET on the RPL router address
    QHostAddress address(configuration().paramValue("RPL address").toString());
    qCDebug(dcOrderButton) << "Scan for new nodes on RPL" << address.toString();

    QUrl url;
    url.setScheme("http");
    url.setHost(address.toString());

    m_asyncNodeScans.insert(networkManagerGet(QNetworkRequest(url)), deviceClassId);
    return DeviceManager::DeviceErrorAsync;
}

void DevicePluginOrderButton::networkManagerReplyReady(QNetworkReply *reply)
{
    if (m_asyncNodeScans.keys().contains(reply)) {
        DeviceClassId deviceClassId = m_asyncNodeScans.take(reply);
        // Check HTTP status code
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
            qCWarning(dcOrderButton) << "Node scan reply HTTP error:" << reply->errorString();
            emit devicesDiscovered(deviceClassId, QList<DeviceDescriptor>());
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        qCDebug(dcOrderButton) << "Node discovery finished:" << endl << data;

        QList<DeviceDescriptor> deviceDescriptors;
        QList<QByteArray> lines = data.split('\n');
        qCDebug(dcOrderButton) << lines;
        foreach (const QByteArray &line, lines) {
            if (line.isEmpty())
                continue;

            QHostAddress address(QString(line.left(line.length() - 4)));
            if (address.isNull())
                continue;

            qCDebug(dcOrderButton) << "Found node" << address.toString();
            // Create a deviceDescriptor for each found address
            DeviceDescriptor descriptor(deviceClassId, "Order Button", address.toString());
            ParamList params;
            params.append(Param("host", address.toString()));
            descriptor.setParams(params);
            deviceDescriptors.append(descriptor);
        }
        // Inform the user which devices were found
        emit devicesDiscovered(deviceClassId, deviceDescriptors);
    }

    // Delete the HTTP reply
    reply->deleteLater();
}

void DevicePluginOrderButton::postSetupDevice(Device *device)
{
    // Try to ping the device after a successful setup
    pingDevice(device);
}

void DevicePluginOrderButton::guhTimer()
{
    // Try to ping each device every 10 seconds to make sure it is still reachable
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == orderbuttonDeviceClassId) {
            pingDevice(device);
        }
    }
}

DeviceManager::DeviceError DevicePluginOrderButton::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() != orderbuttonDeviceClassId)
        return DeviceManager::DeviceErrorDeviceClassNotFound;

    qCDebug(dcOrderButton) << "Execute action" << device->name() << action.params();

    // Check if the device is reachable
    if (!device->stateValue(reachableStateTypeId).toBool()) {
        qCWarning(dcOrderButton) << "Device not reachable.";
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    }

    // Check which action sould be executed
    if (action.actionTypeId() == resetActionTypeId) {
        QUrl url;
        url.setScheme("coap");
        url.setHost(device->paramValue("host").toString());
        url.setPath("/a/reset");

        CoapReply *reply = m_coap->post(CoapRequest(url));
        if (reply->isFinished() && reply->error() != CoapReply::NoError) {
            qCWarning(dcOrderButton) << "CoAP reply finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            return DeviceManager::DeviceErrorHardwareFailure;
        }

        m_resetCounterRequests.insert(reply, action);
        m_asyncActions.insert(action.id(), device);
        return DeviceManager::DeviceErrorAsync;

    } else if(action.actionTypeId() == ledActionTypeId) {
        bool led = action.param("led").value().toBool();

        QUrl url;
        url.setScheme("coap");
        url.setHost(device->paramValue("host").toString());
        url.setPath("/a/led");

        QByteArray payload = QString("mode=%1").arg(QString::number((int)led)).toUtf8();
        qCDebug(dcOrderButton()) << "Sending" << payload;
        CoapReply *reply = m_coap->post(CoapRequest(url), payload);
        if (reply->isFinished() && reply->error() != CoapReply::NoError) {
            qCWarning(dcOrderButton) << "CoAP reply finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            return DeviceManager::DeviceErrorHardwareFailure;
        }

        m_setLedPower.insert(reply, action);
        m_asyncActions.insert(action.id(), device);
        return DeviceManager::DeviceErrorAsync;
    }
    return DeviceManager::DeviceErrorActionTypeNotFound;
}

void DevicePluginOrderButton::pingDevice(Device *device)
{
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue("host").toString());
    m_pingReplies.insert(m_coap->ping(CoapRequest(url)), device);
}

void DevicePluginOrderButton::updateBattery(Device *device)
{
    qCDebug(dcOrderButton) << "Update" << device->name() << "battery value";
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue("host").toString());
    url.setPath("/s/battery");
    CoapReply *reply = m_coap->get(CoapRequest(url));
    if (reply->isFinished() && reply->error() != CoapReply::NoError) {
        qCWarning(dcOrderButton) << "CoAP reply finished with error" << reply->errorString();
        setReachable(device, false);
        reply->deleteLater();
        return;
    }
    m_updateReplies.insert(reply, device);
}

void DevicePluginOrderButton::updateCount(Device *device)
{
    qCDebug(dcOrderButton) << "Update" << device->name() << "count value";
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue("host").toString());
    url.setPath("/s/count");
    CoapReply *reply = m_coap->get(CoapRequest(url));
    if (reply->isFinished() && reply->error() != CoapReply::NoError) {
        qCWarning(dcOrderButton) << "CoAP reply finished with error" << reply->errorString();
        setReachable(device, false);
        reply->deleteLater();
        return;
    }

    m_updateReplies.insert(reply, device);
}

void DevicePluginOrderButton::updateButton(Device *device)
{
    qCDebug(dcOrderButton) << "Update" << device->name() << "button value";
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue("host").toString());
    url.setPath("/s/button");
    CoapReply *reply = m_coap->get(CoapRequest(url));
    if (reply->isFinished() && reply->error() != CoapReply::NoError) {
        qCWarning(dcOrderButton) << "CoAP reply finished with error" << reply->errorString();
        setReachable(device, false);
        reply->deleteLater();
        return;
    }

    m_updateReplies.insert(reply, device);
}


void DevicePluginOrderButton::updateLed(Device *device)
{
    qCDebug(dcOrderButton) << "Update" << device->name() << "led value";
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue("host").toString());
    url.setPath("/a/led");
    CoapReply *reply = m_coap->get(CoapRequest(url));
    if (reply->isFinished() && reply->error() != CoapReply::NoError) {
        qCWarning(dcOrderButton) << "CoAP reply finished with error" << reply->errorString();
        setReachable(device, false);
        reply->deleteLater();
        return;
    }

    m_updateReplies.insert(reply, device);
}

void DevicePluginOrderButton::enableNotifications(Device *device)
{
    qCDebug(dcOrderButton) << "Enable" << device->name() << "notifications";
    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue("host").toString());

    url.setPath("/s/button");
    m_enableNotification.insert(m_coap->enableResourceNotifications(CoapRequest(url)), device);

    url.setPath("/s/count");
    m_enableNotification.insert(m_coap->enableResourceNotifications(CoapRequest(url)), device);

    url.setPath("/s/battery");
    m_enableNotification.insert(m_coap->enableResourceNotifications(CoapRequest(url)), device);

    url.setPath("/a/led");
    m_enableNotification.insert(m_coap->enableResourceNotifications(CoapRequest(url)), device);
}

void DevicePluginOrderButton::setReachable(Device *device, const bool &reachable)
{
    if (device->stateValue(reachableStateTypeId).toBool() != reachable) {
        if (!reachable) {
            // Warn just once that the device is not reachable
            qCWarning(dcOrderButton()) << device->name() << "reachable changed" << reachable;
        } else {
            qCDebug(dcOrderButton()) << device->name() << "reachable changed" << reachable;

            // Get current state values after a reconnect
            updateBattery(device);
            updateCount(device);
            updateButton(device);
            updateLed(device);

            // Make sure the notifications are enabled
            enableNotifications(device);
        }
    }

    device->setStateValue(reachableStateTypeId, reachable);
}

bool DevicePluginOrderButton::deviceAlreadyAdded(const QHostAddress &address)
{
    // Check if we already have a device with the given address
    foreach (Device *device, myDevices()) {
        if (device->paramValue("host").toString() == address.toString()) {
            return true;
        }
    }
    return false;
}

Device *DevicePluginOrderButton::findDevice(const QHostAddress &address)
{
    // Return the device pointer with the given address (otherwise 0)
    foreach (Device *device, myDevices()) {
        if (device->paramValue("host").toString() == address.toString()) {
            return device;
        }
    }
    return NULL;
}

void DevicePluginOrderButton::coapReplyFinished(CoapReply *reply)
{
    if (m_pingReplies.contains(reply)) {
        Device *device = m_pingReplies.take(reply);

        // Check CoAP reply error
        if (reply->error() != CoapReply::NoError) {
            if (device->stateValue(reachableStateTypeId).toBool())
                qCWarning(dcOrderButton) << "Ping device" << reply->request().url().toString() << "reply finished with error" << reply->errorString();

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
            qCWarning(dcOrderButton) << "Update resource" << urlPath << "reply finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            return;
        }

        // Check CoAP status code
        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcOrderButton) << "Update resource" << urlPath << "status code error:" << reply;
            reply->deleteLater();
            return;
        }

        // Update corresponding device state
        if (urlPath == "/s/count") {
            qCDebug(dcOrderButton()) << "Updated count value:" << reply->payload();
            device->setStateValue(countStateTypeId, reply->payload().toInt());
        } else if (urlPath == "/s/button") {
            qCDebug(dcOrderButton()) << "Updated button value:" << reply->payload();
            //device->(buttonStateTypeId, QVariant(reply->payload().toInt()).toBool());
            emit emitEvent(Event(buttonEventTypeId, device->id()));
        } else if (urlPath == "/s/battery") {
            qCDebug(dcOrderButton()) << "Updated battery value:" << reply->payload();
            device->setStateValue(batteryStateTypeId, reply->payload().toDouble());
        } else if (urlPath == "/a/led") {
            qCDebug(dcOrderButton()) << "Updated led value:" << reply->payload();
            device->setStateValue(ledStateTypeId, QVariant(reply->payload().toInt()).toBool());
        }

    } else if (m_resetCounterRequests.contains(reply)) {
        Action action = m_resetCounterRequests.take(reply);
        Device *device = m_asyncActions.take(action.id());

        // Check CoAP reply error
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcOrderButton) << "CoAP reply reset counter finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            return;
        }

        // Check CoAP status code
        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcOrderButton) << "reset counter status code error:" << reply;
            reply->deleteLater();
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            return;
        }
        // Tell the user about the action execution result
        emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorNoError);

    }else if (m_setLedPower.contains(reply)) {
        Action action = m_setLedPower.take(reply);
        Device *device = m_asyncActions.take(action.id());

        // check CoAP reply error
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcOrderButton) << "CoAP set led power reply finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            return;
        }

        // Check CoAP status code
        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcOrderButton) << "Set led power status code error:" << reply;
            reply->deleteLater();
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            return;
        }

        // Update the state here, so we don't have to wait for the notification
        device->setStateValue(ledStateTypeId, action.param("led").value().toBool());
        // Tell the user about the action execution result
        emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorNoError);

    } else if (m_enableNotification.contains(reply)) {
        Device *device = m_enableNotification.take(reply);

        // check CoAP reply error
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcOrderButton) << "Enable notifications for" << reply->request().url().toString() << "reply finished with error" << reply->errorString();
            setReachable(device, false);
            reply->deleteLater();
            return;
        }

        // Check CoAP status code
        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcOrderButton) << "Enable notifications for" << reply->request().url().toString() << "reply status code error" << reply->errorString();
            reply->deleteLater();
            return;
        }

        qCDebug(dcOrderButton()) << "Enabled successfully notifications for" << device->name() << reply->request().url().path();
    }

    // Delete the CoAP reply
    reply->deleteLater();
}

void DevicePluginOrderButton::onNotificationReceived(const CoapObserveResource &resource, const int &notificationNumber, const QByteArray &payload)
{
    qCDebug(dcOrderButton) << " --> Got notification nr." << notificationNumber << resource.url().toString() << payload;
    Device *device = findDevice(QHostAddress(resource.url().host()));
    if (!device) {
        qCWarning(dcOrderButton()) << "Could not find device for this notification";
        return;
    }

    // Update the corresponding device state
    if (resource.url().path() == "/s/button") {
        emit emitEvent(Event(buttonEventTypeId, device->id()));
        //device->setStateValue(buttonStateTypeId, QVariant(payload.toInt()).toBool());
    } else if (resource.url().path() == "/s/battery") {
        device->setStateValue(batteryStateTypeId, payload.toDouble());
    } else if (resource.url().path() == "/a/led") {
        device->setStateValue(ledStateTypeId, QVariant(payload.toInt()).toBool());
    } else if (resource.url().path() == "/s/count") {
        device->setStateValue(countStateTypeId, payload.toInt());
    }
}
