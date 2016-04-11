/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "deviceplugindollhouse.h"
#include "plugininfo.h"

#include <QUrlQuery>

DevicePluginDollHouse::DevicePluginDollHouse() :
    m_houseReachable(false)
{
    m_coap = new Coap(this);
    connect(m_coap, SIGNAL(replyFinished(CoapReply*)), this, SLOT(coapReplyFinished(CoapReply*)));
}

DeviceManager::HardwareResources DevicePluginDollHouse::requiredHardware() const
{
    return DeviceManager::HardwareResourceNetworkManager | DeviceManager::HardwareResourceTimer;
}

DeviceManager::DeviceSetupStatus DevicePluginDollHouse::setupDevice(Device *device)
{
    qCDebug(dcDollhouse) << "Setup" << device->name() << device->params();

    if (device->deviceClassId() == connectionDeviceClassId) {

        foreach (Device *device, myDevices()) {
            if (device->deviceClassId() == connectionDeviceClassId) {
                qCWarning(dcDollhouse) << "Dollhouse connection allready configured.";
                return DeviceManager::DeviceSetupStatusFailure;
            }
        }

        int lookupId = QHostInfo::lookupHost(device->paramValue("RPL address").toString(), this, SLOT(hostLockupFinished(QHostInfo)));
        m_asyncSetup.insert(lookupId, device);

        return DeviceManager::DeviceSetupStatusAsync;

    } else if (device->deviceClassId() == lightDeviceClassId) {

        DollhouseLight *light = new DollhouseLight(this);
        light->setName(device->paramValue("name").toString());
        light->setHostAddress(device->paramValue("address").toString());
        light->setConnectionUuid(device->paramValue("connection uuid").toString());
        light->setLightId(device->paramValue("light id").toInt());

        device->setParentId(DeviceId(light->connectionUuid()));

        m_lights.insert(device, light);

        return DeviceManager::DeviceSetupStatusSuccess;
    }

    return DeviceManager::DeviceSetupStatusFailure;
}


void DevicePluginDollHouse::deviceRemoved(Device *device)
{
    if (device->deviceClassId() == lightDeviceClassId) {
        DollhouseLight *light = m_lights.take(device);
        light->deleteLater();
    }

    m_houseAddress.clear();
    m_houseReachable = false;
}

void DevicePluginDollHouse::guhTimer()
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == connectionDeviceClassId && m_houseAddress.isNull()) {
            scanNodes(device);
        }
    }

    if (!m_houseAddress.isNull()) {
        QUrl url;
        url.setScheme("coap");
        url.setHost(m_houseAddress.toString());
        url.setPort(5683);
        url.setPath("/a/ws2812");
        m_asyncPings.append(m_coap->ping(CoapRequest(url)));
    }

}

DeviceManager::DeviceError DevicePluginDollHouse::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == lightDeviceClassId) {
        if (!device->stateValue(reachableStateTypeId).toBool())
            return DeviceManager::DeviceErrorHardwareNotAvailable;

        DollhouseLight *light = m_lights.value(device);

        // Create URL for action
        QUrlQuery query;
        query.addQueryItem("number", QString::number(light->lightId()));

        QUrl url;
        url.setScheme("coap");
        url.setHost(device->paramValue("address").toString());
        url.setPath("/a/ws2812");
        url.setQuery(query);

        if (action.actionTypeId() == colorActionTypeId) {
            QColor color = action.param("color").value().value<QColor>().toHsv();
            QColor newColor = QColor::fromHsv(color.hue(), color.saturation(), 100 * light->brightness() / 255.0);
            QByteArray message = "color=" + newColor.toRgb().name().remove("#").toUtf8();

            qCDebug(dcDollhouse) << "Sending" << url.toString() << message;

            CoapReply *reply = m_coap->post(CoapRequest(url), message);
            m_asyncActions.insert(reply, action);
            m_asyncActionLights.insert(action.id(), light);

            return DeviceManager::DeviceErrorAsync;

        } else if (action.actionTypeId() == powerActionTypeId) {

            QByteArray message;
            if (action.param("power").value().toBool()) {
                QColor color = light->color().toHsv();
                QColor newColor = QColor::fromHsv(color.hue(), color.saturation(), 100 * light->brightness() / 255.0);
                message = "color=" + newColor.toRgb().name().remove("#").toUtf8();
            } else {
                message.append("color=000000");
            }

            qCDebug(dcDollhouse) << "Sending" << url.toString() << message;

            CoapReply *reply = m_coap->post(CoapRequest(url), message);
            m_asyncActions.insert(reply, action);
            m_asyncActionLights.insert(action.id(), light);

            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == brightnessActionTypeId) {

            int brightness = action.param("brightness").value().toInt();

            QColor color = light->color().toHsv();
            QColor newColor = QColor::fromHsv(color.hue(), color.saturation(), 100 * brightness / 255.0);

            QByteArray message = "color=" + newColor.toRgb().name().remove("#").toUtf8();

            qCDebug(dcDollhouse) << "Sending" << url.toString() << message;

            CoapReply *reply = m_coap->post(CoapRequest(url), message);
            m_asyncActions.insert(reply, action);
            m_asyncActionLights.insert(action.id(), light);

            return DeviceManager::DeviceErrorAsync;
        }
    }

    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginDollHouse::networkManagerReplyReady(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // create user finished
    if (m_asyncNodeScan.contains(reply)) {
        Device *device = m_asyncNodeScan.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcDollhouse) << "Node scan reply HTTP error:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        parseNode(device, reply->readAll());
    }
    reply->deleteLater();
}

void DevicePluginDollHouse::scanNodes(Device *device)
{
    QUrl url;
    url.setScheme("http");
    url.setHost(device->paramValue("RPL address").toString());

    QNetworkReply *reply = networkManagerGet(QNetworkRequest(url));
    m_asyncNodeScan.insert(reply, device);
}

void DevicePluginDollHouse::parseNode(Device *device, const QByteArray &data)
{

    QList<QByteArray> lines = data.split('\n');
    QList<QHostAddress> addresses;
    foreach (const QByteArray &line, lines) {
        if (line.isEmpty())
            continue;

        // remove the '/128' from the address
        QHostAddress address(QString(data.left(line.length() - 4)));

        if (!address.isNull())
            addresses.append(address);

    }

    //    int index = data.indexOf("Routes<pre>") + 11;
    //    int delta = data.indexOf("/128",index);
    //    QHostAddress houseAddress = QHostAddress(QString(data.mid(index, delta - index)));

    if (addresses.isEmpty())
        return;

    QHostAddress houseAddress = addresses.first();
    if (houseAddress != m_houseAddress && !houseAddress.isNull()) {
        m_houseAddress = houseAddress;
        qCDebug(dcDollhouse) << "Found house at" << m_houseAddress.toString();

        if (!m_lights.isEmpty())
            return;

        QList<DeviceDescriptor> deviceDescriptorList;

        for (int i = 0; i < 5; i++) {
            DeviceDescriptor descriptor(lightDeviceClassId, "Light", QString::number(i));
            ParamList params;
            params.append(Param("address", m_houseAddress.toString()));
            params.append(Param("light id", i));
            params.append(Param("connection uuid", device->id()));

            switch (i) {
            case 0:
                params.append(Param("name", "Living room"));
                break;
            case 1:
                params.append(Param("name", "Kitchen"));
                break;
            case 2:
                params.append(Param("name", "Under the bed"));
                break;
            case 3:
                params.append(Param("name", "Bedroom"));
                break;
            case 4:
                params.append(Param("name", "Dining room"));
                break;
            default:
                params.append(Param("name", QString("Light %1").arg(QString::number(i))));
                break;
            }

            descriptor.setParams(params);
            deviceDescriptorList.append(descriptor);
        }

        if (!deviceDescriptorList.isEmpty())
            emit autoDevicesAppeared(lightDeviceClassId, deviceDescriptorList);

    }
}

void DevicePluginDollHouse::hostLockupFinished(const QHostInfo &info)
{
    Device *device = m_asyncSetup.value(info.lookupId());
    if (!device)
        return;

    if (info.error() != QHostInfo::NoError) {
        qCWarning(dcDollhouse) << "Could not look up host" << info.hostName() << info.errorString();
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
    }

    qCDebug(dcDollhouse) << "Looked up successfully" << info.hostName();
    emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);

    scanNodes(device);
}

void DevicePluginDollHouse::coapReplyFinished(CoapReply *reply)
{
    if (m_asyncPings.contains(reply)) {
        m_asyncPings.removeAll(reply);

        if (reply->error() != CoapReply::NoError || reply->statusCode() != CoapPdu::Empty) {

            if (m_houseReachable) {
                qCWarning(dcDollhouse) << "Could not ping Dollhouse:" << reply->errorString();
                m_houseReachable = false;
                foreach (Device *device, myDevices()) {
                    if (device->deviceClassId() == lightDeviceClassId) {
                        device->setStateValue(reachableStateTypeId, m_houseReachable);
                    }
                }
            }

        } else {

            if (!m_houseReachable) {
                qCDebug(dcDollhouse) << "Dollhouse reachable";
                m_houseReachable = true;
                foreach (Device *device, myDevices()) {
                    if (device->deviceClassId() == lightDeviceClassId) {
                        device->setStateValue(reachableStateTypeId, m_houseReachable);
                    }
                }
            }
        }

    } else if (m_asyncActions.contains(reply)) {
        Action action = m_asyncActions.take(reply);
        DollhouseLight *light = m_asyncActionLights.take(action.id());

        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcDollhouse) << "Got action response with error" << reply->errorString();
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            reply->deleteLater();
            return;
        }

        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcDollhouse) << "Got action response with status code" << reply;
            emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorHardwareFailure);
            reply->deleteLater();
            return;
        }

        emit actionExecutionFinished(action.id(), DeviceManager::DeviceErrorNoError);

        // Set the states
        if (action.actionTypeId() == powerActionTypeId) {
            bool power = action.param("power").value().toBool();
            light->setPower(power);
            m_lights.key(light)->setStateValue(powerStateTypeId, power);

        } else if (action.actionTypeId() == colorActionTypeId) {
            if (!light->power()) {
                light->setPower(true);
                m_lights.key(light)->setStateValue(powerStateTypeId, true);
            }

            QColor color = action.param("color").value().value<QColor>();
            light->setColor(color);
            m_lights.key(light)->setStateValue(colorStateTypeId, color);

        } else if (action.actionTypeId() == brightnessActionTypeId) {
            if (!light->power()) {
                light->setPower(true);
                m_lights.key(light)->setStateValue(powerStateTypeId, true);
            }

            int brightness = action.param("brightness").value().toInt();
            light->setBrightness(brightness);
            m_lights.key(light)->setStateValue(brightnessStateTypeId, brightness);
        }
    }

    reply->deleteLater();
}

