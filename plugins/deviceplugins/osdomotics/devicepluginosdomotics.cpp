/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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
    \page osdomotics.html
    \title OSDomotics
    \brief Plugin for the OSDomotics Merkur board based on the OSDomotics tutorials.

    \ingroup plugins
    \ingroup guh-plugins-merkur

    This plugin allows you to connect guh to a 6LoWPAN network by adding a Mercury Board from OSDomotics
    as a RPL router to your devices \l{http://osdwiki.open-entry.com/doku.php/de:tutorials:contiki:merkur_board_rpl_usb_router}{OSDomotics Tutorial- RPL Router}.
    All nodes in the 6LoWPAN network of the added RPL router will appear automatically in the system.

    \note Currently the plugin recognizes only one node. That node has to be flashed like the Node in this \l{http://osdwiki.open-entry.com/doku.php/de:tutorials:contiki:use_example_firmware}{OSDomotics Tutorial - Firmware}.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/osdomotics/devicepluginosdomotics.json
*/

#include "devicepluginosdomotics.h"
#include "plugin/device.h"
#include "plugininfo.h"

DevicePluginOsdomotics::DevicePluginOsdomotics()
{
    m_coap = new Coap(this);
    connect(m_coap, SIGNAL(replyFinished(CoapReply*)), this, SLOT(coapReplyFinished(CoapReply*)));
}

DeviceManager::HardwareResources DevicePluginOsdomotics::requiredHardware() const
{
    return DeviceManager::HardwareResourceNetworkManager | DeviceManager::HardwareResourceTimer;
}

DeviceManager::DeviceSetupStatus DevicePluginOsdomotics::setupDevice(Device *device)
{

    if (device->deviceClassId() == rplRouterDeviceClassId) {
        qCDebug(dcOsdomotics) << "Setup RPL router" << device->paramValue("host").toString();
        QHostAddress address(device->paramValue("host").toString());

        if (address.isNull()) {
            qCWarning(dcOsdomotics) << "Got invalid address" << device->paramValue("host").toString();
            return DeviceManager::DeviceSetupStatusFailure;
        }

        QUrl url;
        url.setScheme("http");
        url.setHost(address.toString());

        QNetworkReply *reply = networkManagerGet(QNetworkRequest(url));
        m_asyncSetup.insert(reply, device);

        return DeviceManager::DeviceSetupStatusAsync;
    } else if (device->deviceClassId() == merkurNodeDeviceClassId) {
        qCDebug(dcOsdomotics) << "Setup Merkur node" << device->paramValue("host").toString();
        device->setParentId(DeviceId(device->paramValue("router id").toString()));
        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}

void DevicePluginOsdomotics::deviceRemoved(Device *device)
{
    Q_UNUSED(device)
}

void DevicePluginOsdomotics::networkManagerReplyReady(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // create user finished
    if (m_asyncSetup.contains(reply)) {
        Device *device = m_asyncSetup.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcOsdomotics) << "Setup reply HTTP error:" << reply->errorString();
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        parseNodes(device, data);

        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);
    } else if (m_asyncNodeRescans.contains(reply)) {
        Device *device = m_asyncSetup.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcOsdomotics) << "Setup reply HTTP error:" << reply->errorString();
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        parseNodes(device, data);
    }
    reply->deleteLater();
}

void DevicePluginOsdomotics::postSetupDevice(Device *device)
{
    updateNode(device);
}

void DevicePluginOsdomotics::guhTimer()
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == merkurNodeDeviceClassId) {
            updateNode(device);
        } else if(device->deviceClassId() == rplRouterDeviceClassId) {
            scanNodes(device);
        }
    }
}

DeviceManager::DeviceError DevicePluginOsdomotics::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == merkurNodeDeviceClassId) {
        if (action.actionTypeId() == toggleLedActionTypeId) {
            QUrl url;
            url.setScheme("coap");
            url.setHost(device->paramValue("host").toString());
            url.setPath("/actuators/toggle");

            qCDebug(dcOsdomotics) << "Toggle light";

            CoapReply *reply = m_coap->post(CoapRequest(url));

            if (reply->isFinished()) {
                if (reply->error() != CoapReply::NoError) {
                    qCWarning(dcOsdomotics) << "CoAP reply finished with error" << reply->errorString();
                    reply->deleteLater();
                    return DeviceManager::DeviceErrorHardwareNotAvailable;
                }
            }

            m_toggleLightRequests.insert(reply, action);

            return DeviceManager::DeviceErrorAsync;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginOsdomotics::scanNodes(Device *device)
{
    QHostAddress address(device->paramValue("host").toString());
    qCDebug(dcOsdomotics) << "Scan for new nodes" << address.toString();

    QUrl url;
    url.setScheme("http");
    url.setHost(address.toString());

    QNetworkReply *reply = networkManagerGet(QNetworkRequest(url));
    m_asyncNodeRescans.insert(reply, device);
}

void DevicePluginOsdomotics::parseNodes(Device *device, const QByteArray &data)
{
    //qCDebug(dcOsdomotics) << data;

    // TODO: get all nodes
    //       find better method to get nodes

    int index = data.indexOf("Routes<pre>") + 11;
    int delta = data.indexOf("/128",index);

    QHostAddress nodeAddress(QString(data.mid(index, delta - index)));

    // check if we allready have found this node
    foreach (Device *device, myDevices()) {
        if (device->paramValue("host").toString() == nodeAddress.toString()) {
            return;
        }
    }

    QUrl url;
    url.setScheme("coap");
    url.setHost(nodeAddress.toString());
    url.setPath("/.well-known/core");

    qCDebug(dcOsdomotics) << "Discover node on" << url.toString();

    CoapReply *reply = m_coap->get(CoapRequest(url));
    if (reply->isFinished()) {
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcOsdomotics) << "Reply finished with error" << reply->errorString();
        } else {
            qCDebug(dcOsdomotics) << "Reply finished" << reply;
        }

        // Note: please don't forget to delete the reply
        reply->deleteLater();
        return;
    }
    m_discoveryRequests.insert(reply, device);
}

void DevicePluginOsdomotics::updateNode(Device *device)
{
    qCDebug(dcOsdomotics) << "Update node" << device->paramValue("host").toString() << "battery value";

    QUrl url;
    url.setScheme("coap");
    url.setHost(device->paramValue("host").toString());
    url.setPath("/sensors/battery");

    CoapReply *reply = m_coap->get(CoapRequest(url));

    if (reply->isFinished()) {
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcOsdomotics) << "CoAP reply finished with error" << reply->errorString();
            reply->deleteLater();
        }
    }
    m_updateRequests.insert(reply, device);
}

Device *DevicePluginOsdomotics::findDevice(const QHostAddress &address)
{
    foreach (Device *device, myDevices()) {
        if (device->paramValue("host").toString() == address.toString()) {
            return device;
        }
    }
    return 0;
}

void DevicePluginOsdomotics::coapReplyFinished(CoapReply *reply)
{
    qCDebug(dcOsdomotics) << "coap reply finished" << reply;

    if (m_discoveryRequests.contains(reply)) {
        Device *device = m_discoveryRequests.take(reply);
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcOsdomotics) << "CoAP discover reply finished with error" << reply->errorString();
            reply->deleteLater();
            return;
        }

        // TODO: parse CoRE links and get the type of the node

        DeviceDescriptor descriptor(merkurNodeDeviceClassId, "Merkur Node", reply->request().url().host());
        ParamList params;
        params.append(Param("name", "Merkur Node"));
        params.append(Param("host",  reply->request().url().host()));
        params.append(Param("router id", device->id()));
        descriptor.setParams(params);
        emit autoDevicesAppeared(merkurNodeDeviceClassId, QList<DeviceDescriptor>() << descriptor);

    } else if (m_toggleLightRequests.contains(reply)) {
        Action action = m_toggleLightRequests.take(reply);
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcOsdomotics) << "CoAP toggle reply finished with error" << reply->errorString();
            reply->deleteLater();
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            return;
        }

        emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorNoError);
    } else if (m_updateRequests.contains(reply)) {
        Device *device = m_updateRequests.take(reply);
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcOsdomotics) << "CoAP update reply finished with error" << reply->errorString();
            reply->deleteLater();
            return;
        }
        int batteryValue = reply->payload().toInt();
        qCDebug(dcOsdomotics) << "Node updated" << batteryValue;
        device->setStateValue(batteryStateTypeId, batteryValue);
    }

    reply->deleteLater();
}


