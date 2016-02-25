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

    \note If a \l{StateType} has the parameter \tt{"writable": {...}}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/philipshue/devicepluginphilipshue.json
*/

#include "devicepluginphilipshue.h"

#include "devicemanager.h"
#include "plugin/device.h"
#include "types/param.h"
#include "plugininfo.h"

#include <QDebug>
#include <QStringList>
#include <QColor>
#include <QJsonDocument>

DevicePluginPhilipsHue::DevicePluginPhilipsHue()
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->setInterval(1500);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

DeviceManager::HardwareResources DevicePluginPhilipsHue::requiredHardware() const
{
    return DeviceManager::HardwareResourceUpnpDisovery | DeviceManager::HardwareResourceNetworkManager;
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
    // hue bridge
    if (device->deviceClassId() == hueBridgeDeviceClassId) {
        // unconfigured bridges (from pairing)
        foreach (HueBridge *b, m_unconfiguredBridges) {
            if (b->hostAddress().toString() == device->paramValue("host address").toString()) {
                m_unconfiguredBridges.removeAll(b);
                qCDebug(dcPhilipsHue) << "Setup unconfigured Hue Bridge" << b->name();
                // set data which was not known during discovery
                device->setParamValue("name", b->name());
                device->setParamValue("api key", b->apiKey());
                device->setParamValue("zigbee channel", b->zigbeeChannel());
                device->setParamValue("id", b->id());
                device->setParamValue("mac address", b->macAddress());
                m_bridges.insert(b, device);
                device->setStateValue(bridgeReachableStateTypeId, true);
                discoverBridgeDevices(b);
                m_timer->start();
                return DeviceManager::DeviceSetupStatusSuccess;
            }
        }

        // loaded bridge
        qCDebug(dcPhilipsHue) << "Setup Hue Bridge" << device->params();

        HueBridge *bridge = new HueBridge(this);
        bridge->setId(device->paramValue("id").toString());
        bridge->setApiKey(device->paramValue("api key").toString());
        bridge->setHostAddress(QHostAddress(device->paramValue("host address").toString()));
        bridge->setName(device->paramValue("name").toString());
        bridge->setMacAddress(device->paramValue("mac address").toString());
        bridge->setZigbeeChannel(device->paramValue("zigbee channel").toInt());

        m_bridges.insert(bridge, device);
        m_timer->start();
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    // hue color light
    if (device->deviceClassId() == hueLightDeviceClassId) {
        qCDebug(dcPhilipsHue) << "Setup Hue color light" << device->params();

        HueLight *hueLight = new HueLight(this);
        hueLight->setId(device->paramValue("light id").toInt());
        hueLight->setHostAddress(QHostAddress(device->paramValue("host address").toString()));
        hueLight->setName(device->paramValue("name").toString());
        hueLight->setApiKey(device->paramValue("api key").toString());
        hueLight->setModelId(device->paramValue("model id").toString());
        hueLight->setUuid(device->paramValue("uuid").toString());
        hueLight->setType(device->paramValue("type").toString());
        hueLight->setBridgeId(DeviceId(device->paramValue("bridge").toString()));
        device->setParentId(hueLight->bridgeId());

        connect(hueLight, &HueLight::stateChanged, this, &DevicePluginPhilipsHue::lightStateChanged);
        m_lights.insert(hueLight, device);

        refreshLight(device);
        setLightName(device, device->paramValue("name").toString());

        return DeviceManager::DeviceSetupStatusSuccess;
    }

    // hue white light
    if (device->deviceClassId() == hueWhiteLightDeviceClassId) {
        qCDebug(dcPhilipsHue) << "Setup Hue white light" << device->params();

        HueLight *hueLight = new HueLight(this);
        hueLight->setId(device->paramValue("light id").toInt());
        hueLight->setHostAddress(QHostAddress(device->paramValue("host address").toString()));
        hueLight->setName(device->paramValue("name").toString());
        hueLight->setApiKey(device->paramValue("api key").toString());
        hueLight->setModelId(device->paramValue("model id").toString());
        hueLight->setType(device->paramValue("type").toString());
        hueLight->setUuid(device->paramValue("uuid").toString());
        hueLight->setBridgeId(DeviceId(device->paramValue("bridge").toString()));
        device->setParentId(hueLight->bridgeId());

        connect(hueLight, &HueLight::stateChanged, this, &DevicePluginPhilipsHue::lightStateChanged);

        m_lights.insert(hueLight, device);
        refreshLight(device);

        setLightName(device, device->paramValue("name").toString());
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    // hue remote
    if (device->deviceClassId() == hueRemoteDeviceClassId) {
        qCDebug(dcPhilipsHue) << "Setup Hue remote" << device->params();

        HueRemote *hueRemote = new HueRemote(this);
        hueRemote->setId(device->paramValue("sensor id").toInt());
        hueRemote->setHostAddress(QHostAddress(device->paramValue("host address").toString()));
        hueRemote->setName(device->paramValue("name").toString());
        hueRemote->setApiKey(device->paramValue("api key").toString());
        hueRemote->setModelId(device->paramValue("model id").toString());
        hueRemote->setType(device->paramValue("type").toString());
        hueRemote->setUuid(device->paramValue("uuid").toString());
        hueRemote->setBridgeId(DeviceId(device->paramValue("bridge").toString()));
        device->setParentId(hueRemote->bridgeId());

        connect(hueRemote, &HueRemote::stateChanged, this, &DevicePluginPhilipsHue::remoteStateChanged);
        connect(hueRemote, &HueRemote::buttonPressed, this, &DevicePluginPhilipsHue::onRemoteButtonEvent);

        m_remotes.insert(hueRemote, device);
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    return DeviceManager::DeviceSetupStatusFailure;
}

void DevicePluginPhilipsHue::deviceRemoved(Device *device)
{
    if (device->deviceClassId() == hueBridgeDeviceClassId) {
        HueBridge *bridge = m_bridges.key(device);
        m_bridges.remove(bridge);
        bridge->deleteLater();
    }

    if (device->deviceClassId() == hueLightDeviceClassId || device->deviceClassId() == hueWhiteLightDeviceClassId) {
        HueLight *light = m_lights.key(device);
        m_lights.remove(light);
        light->deleteLater();
    }

    if (device->deviceClassId() == hueRemoteDeviceClassId) {
        HueRemote *remote = m_remotes.key(device);
        m_remotes.remove(remote);
        remote->deleteLater();
    }

    if (myDevices().isEmpty())
        m_timer->stop();
}

void DevicePluginPhilipsHue::upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList)
{
    if (upnpDeviceDescriptorList.isEmpty()) {
        qCDebug(dcPhilipsHue) << "No UPnP device found. Try N-UPNP discovery.";
        QNetworkRequest request(QUrl("https://www.meethue.com/api/nupnp"));
        QNetworkReply *reply = networkManagerGet(request);
        m_discoveryRequests.append(reply);
        return;
    }

    QList<DeviceDescriptor> deviceDescriptors;
    foreach (const UpnpDeviceDescriptor &upnpDevice, upnpDeviceDescriptorList) {
        if (upnpDevice.modelDescription().contains("Philips")) {
            DeviceDescriptor descriptor(hueBridgeDeviceClassId, "Philips Hue Bridge", upnpDevice.hostAddress().toString());
            ParamList params;
            params.append(Param("name", upnpDevice.friendlyName()));
            params.append(Param("host address", upnpDevice.hostAddress().toString()));
            params.append(Param("api key", QString()));
            params.append(Param("mac address", QString()));
            params.append(Param("id", upnpDevice.serialNumber().toLower()));
            params.append(Param("zigbee channel", -1));
            descriptor.setParams(params);
            deviceDescriptors.append(descriptor);
        }
    }

    emit devicesDiscovered(hueBridgeDeviceClassId, deviceDescriptors);
}

DeviceManager::DeviceSetupStatus DevicePluginPhilipsHue::confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params, const QString &secret)
{
    Q_UNUSED(secret)

    if (deviceClassId != hueBridgeDeviceClassId)
        return DeviceManager::DeviceSetupStatusFailure;

    PairingInfo *pairingInfo = new PairingInfo(this);
    pairingInfo->setPairingTransactionId(pairingTransactionId);
    pairingInfo->setHost(QHostAddress(params.paramValue("host address").toString()));

    QVariantMap deviceTypeParam;
    deviceTypeParam.insert("devicetype", "guh");

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(deviceTypeParam);

    QNetworkRequest request(QUrl("http://" + pairingInfo->host().toString() + "/api"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = networkManagerPost(request, jsonDoc.toJson());

    m_pairingRequests.insert(reply, pairingInfo);

    return DeviceManager::DeviceSetupStatusAsync;
}

void DevicePluginPhilipsHue::networkManagerReplyReady(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // create user finished
    if (m_pairingRequests.keys().contains(reply)) {
        PairingInfo *pairingInfo = m_pairingRequests.take(reply);

        // check HTTP status code
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            qCWarning(dcPhilipsHue) << "Request error:" << status << reply->errorString();
            pairingInfo->deleteLater();
            reply->deleteLater();
            return;
        }
        processPairingResponse(pairingInfo, reply->readAll());

    } else if (m_informationRequests.keys().contains(reply)) {
        PairingInfo *pairingInfo = m_informationRequests.take(reply);

        // check HTTP status code
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            qCWarning(dcPhilipsHue) << "Request error:" << status << reply->errorString();
            reply->deleteLater();
            pairingInfo->deleteLater();
            return;
        }
        processInformationResponse(pairingInfo, reply->readAll());

    } else if (m_discoveryRequests.contains(reply)) {
        m_discoveryRequests.removeAll(reply);
        // check HTTP status code
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            qCWarning(dcPhilipsHue) << "N-UPNP discovery error:" << status << reply->errorString();
            reply->deleteLater();
            return;
        }
        processNUpnpResponse(reply->readAll());

    } else if (m_bridgeLightsDiscoveryRequests.keys().contains(reply)) {
        Device *device = m_bridgeLightsDiscoveryRequests.take(reply);

        // check HTTP status code
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            qCWarning(dcPhilipsHue) << "Bridge light discovery error:" << status << reply->errorString();
            bridgeReachableChanged(device, false);
            reply->deleteLater();
            return;
        }
        processBridgeLightDiscoveryResponse(device, reply->readAll());

    } else if (m_bridgeSensorsDiscoveryRequests.keys().contains(reply)) {
        Device *device = m_bridgeSensorsDiscoveryRequests.take(reply);

        // check HTTP status code
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            qCWarning(dcPhilipsHue) << "Bridge sensor discovery error:" << status << reply->errorString();
            bridgeReachableChanged(device, false);
            reply->deleteLater();
            return;
        }
        processBridgeSensorDiscoveryResponse(device, reply->readAll());

    } else if (m_bridgeSearchDevicesRequests.keys().contains(reply)) {
        Device *device = m_bridgeSearchDevicesRequests.take(reply);

        // check HTTP status code
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            qCWarning(dcPhilipsHue) << "Bridge search new devices error:" << status << reply->errorString();
            bridgeReachableChanged(device, false);
            reply->deleteLater();
            return;
        }
        discoverBridgeDevices(m_bridges.key(device));

    } else if (m_bridgeRefreshRequests.keys().contains(reply)) {
        Device *device = m_bridgeRefreshRequests.take(reply);

        // check HTTP status code
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            if (device->stateValue(bridgeReachableStateTypeId).toBool()) {
                qCWarning(dcPhilipsHue) << "Refresh Hue Bridge request error:" << status << reply->errorString();
                bridgeReachableChanged(device, false);
            }
            reply->deleteLater();
            return;
        }
        processBridgeRefreshResponse(device, reply->readAll());

    } else if (m_lightRefreshRequests.keys().contains(reply)) {
        Device *device = m_lightRefreshRequests.take(reply);

        // check HTTP status code
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            qCWarning(dcPhilipsHue) << "Refresh Hue Light request error:" << status << reply->errorString();
            bridgeReachableChanged(device, false);
            reply->deleteLater();
            return;
        }
        processLightRefreshResponse(device, reply->readAll());

    } else if (m_lightsRefreshRequests.keys().contains(reply)) {
        Device *device = m_lightsRefreshRequests.take(reply);

        // check HTTP status code
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            if (device->stateValue(bridgeReachableStateTypeId).toBool()) {
                qCWarning(dcPhilipsHue) << "Refresh Hue lights request error:" << status << reply->errorString();
                bridgeReachableChanged(device, false);
            }
            reply->deleteLater();
            return;
        }
        processLightsRefreshResponse(device, reply->readAll());

    } else if (m_sensorsRefreshRequests.keys().contains(reply)) {
        Device *device = m_sensorsRefreshRequests.take(reply);

        // check HTTP status code
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            if (device->stateValue(bridgeReachableStateTypeId).toBool()) {
                qCWarning(dcPhilipsHue) << "Refresh Hue sensors request error:" << status << reply->errorString();
                bridgeReachableChanged(device, false);
            }
            reply->deleteLater();
            return;
        }
        processSensorsRefreshResponse(device, reply->readAll());

    } else if (m_asyncActions.keys().contains(reply)) {
        QPair<Device *, ActionId> actionInfo = m_asyncActions.take(reply);

        // check HTTP status code
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            qCWarning(dcPhilipsHue) << "Execute Hue Light action request error:" << status << reply->errorString();
            bridgeReachableChanged(actionInfo.first, false);
            emit actionExecutionFinished(actionInfo.second, DeviceManager::DeviceErrorHardwareNotAvailable);
            reply->deleteLater();
            return;
        }
        processActionResponse(actionInfo.first, actionInfo.second, reply->readAll());

    } else if (m_lightSetNameRequests.keys().contains(reply)) {
        Device *device = m_lightSetNameRequests.take(reply);

        // check HTTP status code
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            qCWarning(dcPhilipsHue) << "Set name of Hue Light request error:" << status << reply->errorString();
            bridgeReachableChanged(device, false);
            reply->deleteLater();
            return;
        }
        processSetNameResponse(device, reply->readAll());
    }
    reply->deleteLater();
}

DeviceManager::DeviceError DevicePluginPhilipsHue::executeAction(Device *device, const Action &action)
{
    qCDebug(dcPhilipsHue) << "Execute action" << action.actionTypeId() << action.params();

    // color light
    if (device->deviceClassId() == hueLightDeviceClassId) {
        HueLight *light = m_lights.key(device);

        if (!light->reachable()) {
            return DeviceManager::DeviceErrorHardwareNotAvailable;
            qCWarning(dcPhilipsHue) << "Light" << light->name() << "not reachable";
        }

        if (action.actionTypeId() == huePowerActionTypeId) {
            QPair<QNetworkRequest, QByteArray> request = light->createSetPowerRequest(action.param("power").value().toBool());
            m_asyncActions.insert(networkManagerPut(request.first, request.second),QPair<Device *, ActionId>(device, action.id()));
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == hueColorActionTypeId) {
            QPair<QNetworkRequest, QByteArray> request = light->createSetColorRequest(action.param("color").value().value<QColor>());
            m_asyncActions.insert(networkManagerPut(request.first, request.second),QPair<Device *, ActionId>(device, action.id()));
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == hueBrightnessActionTypeId) {
            QPair<QNetworkRequest, QByteArray> request = light->createSetBrightnessRequest(percentageToBrightness(action.param("brightness").value().toInt()));
            m_asyncActions.insert(networkManagerPut(request.first, request.second),QPair<Device *, ActionId>(device, action.id()));
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == hueEffectActionTypeId) {
            QPair<QNetworkRequest, QByteArray> request = light->createSetEffectRequest(action.param("effect").value().toString());
            m_asyncActions.insert(networkManagerPut(request.first, request.second),QPair<Device *, ActionId>(device, action.id()));
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == hueAlertActionTypeId) {
            QPair<QNetworkRequest, QByteArray> request = light->createFlashRequest(action.param("alert").value().toString());
            m_asyncActions.insert(networkManagerPut(request.first, request.second),QPair<Device *, ActionId>(device, action.id()));
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == hueTemperatureActionTypeId) {
            QPair<QNetworkRequest, QByteArray> request = light->createSetTemperatureRequest(action.param("color temperature").value().toInt());
            m_asyncActions.insert(networkManagerPut(request.first, request.second),QPair<Device *, ActionId>(device, action.id()));
            return DeviceManager::DeviceErrorAsync;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    // white light
    if (device->deviceClassId() == hueWhiteLightDeviceClassId) {
        HueLight *light = m_lights.key(device);

        if (!light->reachable()) {
            return DeviceManager::DeviceErrorHardwareNotAvailable;
            qCWarning(dcPhilipsHue) << "Light" << light->name() << "not reachable";
        }

        if (action.actionTypeId() == huePowerActionTypeId) {
            QPair<QNetworkRequest, QByteArray> request = light->createSetPowerRequest(action.param("power").value().toBool());
            m_asyncActions.insert(networkManagerPut(request.first, request.second),QPair<Device *, ActionId>(device, action.id()));
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == hueBrightnessActionTypeId) {
            QPair<QNetworkRequest, QByteArray> request = light->createSetBrightnessRequest(percentageToBrightness(action.param("brightness").value().toInt()));
            m_asyncActions.insert(networkManagerPut(request.first, request.second),QPair<Device *, ActionId>(device, action.id()));
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == hueAlertActionTypeId) {
            QPair<QNetworkRequest, QByteArray> request = light->createFlashRequest(action.param("alert").value().toString());
            m_asyncActions.insert(networkManagerPut(request.first, request.second),QPair<Device *, ActionId>(device, action.id()));
            return DeviceManager::DeviceErrorAsync;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    if (device->deviceClassId() == hueBridgeDeviceClassId) {
        HueBridge *bridge = m_bridges.key(device);
        if (!device->stateValue(bridgeReachableStateTypeId).toBool()) {
            qCWarning(dcPhilipsHue) << "Bridge" << bridge->hostAddress().toString() << "not reachable";
            return DeviceManager::DeviceErrorHardwareNotAvailable;
        }

        if (action.actionTypeId() == searchNewDevicesActionTypeId) {
            searchNewDevices(bridge);
            return DeviceManager::DeviceErrorNoError;
        } else if (action.actionTypeId() == checkForUpdatesActionTypeId) {
            QPair<QNetworkRequest, QByteArray> request = bridge->createCheckUpdatesRequest();
            m_asyncActions.insert(networkManagerPut(request.first, request.second),QPair<Device *, ActionId>(device, action.id()));
            return DeviceManager::DeviceErrorAsync;
        } else if (action.actionTypeId() == upgradeActionTypeId) {
            QPair<QNetworkRequest, QByteArray> request = bridge->createUpgradeRequest();
            m_asyncActions.insert(networkManagerPut(request.first, request.second),QPair<Device *, ActionId>(device, action.id()));
            return DeviceManager::DeviceErrorAsync;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginPhilipsHue::lightStateChanged()
{
    HueLight *light = static_cast<HueLight *>(sender());

    Device *device = m_lights.value(light);
    if (!device) {
        qCWarning(dcPhilipsHue) << "Could not find device for light" << light->name();
        return;
    }

    if (device->deviceClassId() == hueLightDeviceClassId) {
        device->setStateValue(hueReachableStateTypeId, light->reachable());
        device->setStateValue(hueColorStateTypeId, QVariant::fromValue(light->color()));
        device->setStateValue(huePowerStateTypeId, light->power());
        device->setStateValue(hueBrightnessStateTypeId, brightnessToPercentage(light->brightness()));
        device->setStateValue(hueTemperatureStateTypeId, light->ct());
        device->setStateValue(hueEffectStateTypeId, light->effect());
    } else if (device->deviceClassId() == hueWhiteLightDeviceClassId) {
        device->setStateValue(hueReachableStateTypeId, light->reachable());
        device->setStateValue(huePowerStateTypeId, light->power());
        device->setStateValue(hueBrightnessStateTypeId, brightnessToPercentage(light->brightness()));
    }
}

void DevicePluginPhilipsHue::remoteStateChanged()
{
    HueRemote *remote = static_cast<HueRemote *>(sender());

    Device *device = m_remotes.value(remote);
    if (!device) {
        qCWarning(dcPhilipsHue) << "Could not find device for remote" << remote->name();
        return;
    }

    device->setStateValue(hueReachableStateTypeId, remote->reachable());
    device->setStateValue(batteryStateTypeId, remote->battery());
}

void DevicePluginPhilipsHue::onRemoteButtonEvent(const int &buttonCode)
{
    HueRemote *remote = static_cast<HueRemote *>(sender());

    switch (buttonCode) {
    case HueRemote::OnPressed:
        emitEvent(Event(onPressedEventTypeId, m_remotes.value(remote)->id()));
        break;
    case HueRemote::OnLongPressed:
        emitEvent(Event(onLongPressedEventTypeId, m_remotes.value(remote)->id()));
        break;
    case HueRemote::DimUpPressed:
        emitEvent(Event(dimUpPressedEventTypeId, m_remotes.value(remote)->id()));
        break;
    case HueRemote::DimUpLongPressed:
        emitEvent(Event(dimUpLongPressedEventTypeId, m_remotes.value(remote)->id()));
        break;
    case HueRemote::DimDownPressed:
        emitEvent(Event(dimDownPressedEventTypeId, m_remotes.value(remote)->id()));
        break;
    case HueRemote::DimDownLongPressed:
        emitEvent(Event(dimDownLongPressedEventTypeId, m_remotes.value(remote)->id()));
        break;
    case HueRemote::OffPressed:
        emitEvent(Event(offPressedEventTypeId, m_remotes.value(remote)->id()));
        break;
    case HueRemote::OffLongPressed:
        emitEvent(Event(offLongPressedEventTypeId, m_remotes.value(remote)->id()));
        break;
    default:
        break;
    }
}

void DevicePluginPhilipsHue::onTimeout()
{
    foreach (Device *device, m_bridges.values()) {
        refreshBridge(device);
    }
}

void DevicePluginPhilipsHue::refreshLight(Device *device)
{
    HueLight *light = m_lights.key(device);

    QNetworkRequest request(QUrl("http://" + light->hostAddress().toString() + "/api/" + light->apiKey() + "/lights/" + QString::number(light->id())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = networkManagerGet(request);

    m_lightRefreshRequests.insert(reply, device);
}

void DevicePluginPhilipsHue::refreshBridge(Device *device)
{
    if (!m_bridgeRefreshRequests.isEmpty()) {
        QNetworkReply *reply = m_bridgeRefreshRequests.key(device);
        reply->abort();
        m_bridgeRefreshRequests.remove(reply);
        reply->deleteLater();
        bridgeReachableChanged(device, false);
        return;
    }

    HueBridge *bridge = m_bridges.key(device);

    QNetworkRequest request(QUrl("http://" + bridge->hostAddress().toString() + "/api/" + bridge->apiKey() + "/config"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = networkManagerGet(request);

    m_bridgeRefreshRequests.insert(reply, device);
}

void DevicePluginPhilipsHue::refreshLights(HueBridge *bridge)
{
    Device *device = m_bridges.value(bridge);

    QNetworkRequest request(QUrl("http://" + bridge->hostAddress().toString() + "/api/" + bridge->apiKey() + "/lights"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = networkManagerGet(request);

    m_lightsRefreshRequests.insert(reply, device);
}

void DevicePluginPhilipsHue::refreshSensors(HueBridge *bridge)
{
    Device *device = m_bridges.value(bridge);

    QNetworkRequest request(QUrl("http://" + bridge->hostAddress().toString() + "/api/" + bridge->apiKey() + "/sensors"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = networkManagerGet(request);

    m_sensorsRefreshRequests.insert(reply, device);
}

void DevicePluginPhilipsHue::discoverBridgeDevices(HueBridge *bridge)
{
    Device *device = m_bridges.value(bridge);
    qCDebug(dcPhilipsHue) << "Discover bridge devices" << bridge->hostAddress();

    QPair<QNetworkRequest, QByteArray> lightsRequest = bridge->createDiscoverLightsRequest();
    m_bridgeLightsDiscoveryRequests.insert(networkManagerGet(lightsRequest.first), device);

    QPair<QNetworkRequest, QByteArray> sensorsRequest = bridge->createSearchSensorsRequest();
    m_bridgeSensorsDiscoveryRequests.insert(networkManagerGet(sensorsRequest.first), device);
}

void DevicePluginPhilipsHue::searchNewDevices(HueBridge *bridge)
{
    Device *device = m_bridges.value(bridge);
    qCDebug(dcPhilipsHue) << "Discover bridge devices" << bridge->hostAddress();

    QPair<QNetworkRequest, QByteArray> request = bridge->createSearchLightsRequest();
    m_bridgeSearchDevicesRequests.insert(networkManagerPost(request.first, request.second), device);
}

void DevicePluginPhilipsHue::setLightName(Device *device, const QString &name)
{
    HueLight *light = m_lights.key(device);

    QVariantMap requestMap;
    requestMap.insert("name", name);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + light->hostAddress().toString() + "/api/" + light->apiKey() +
                                 "/lights/" + QString::number(light->id())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManagerPut(request,jsonDoc.toJson());
    m_lightSetNameRequests.insert(reply, device);
}

void DevicePluginPhilipsHue::processNUpnpResponse(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "N-UPNP discovery JSON error in response" << error.errorString();
        return;
    }

    QVariantList bridgeList = jsonDoc.toVariant().toList();

    QList<DeviceDescriptor> deviceDescriptors;
    foreach (const QVariant &bridgeVariant, bridgeList) {
        QVariantMap bridgeMap = bridgeVariant.toMap();
        DeviceDescriptor descriptor(hueBridgeDeviceClassId, "Philips Hue Bridge", bridgeMap.value("internalipaddress").toString());
        ParamList params;
        params.append(Param("name", "Philips hue"));
        params.append(Param("host address", bridgeMap.value("internalipaddress").toString()));
        params.append(Param("api key", QString()));
        params.append(Param("mac address", QString()));
        params.append(Param("id", bridgeMap.value("internalipaddress").toString().toLower()));
        params.append(Param("zigbee channel", -1));
        descriptor.setParams(params);
        deviceDescriptors.append(descriptor);
    }
    qCDebug(dcPhilipsHue) << "N-UPNP discover finished. Found" << deviceDescriptors.count() << "devices.";
    emit devicesDiscovered(hueBridgeDeviceClassId, deviceDescriptors);
}

void DevicePluginPhilipsHue::processBridgeLightDiscoveryResponse(Device *device, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Bridge light discovery json error in response" << error.errorString();
        return;
    }

    // check response error
    if (data.contains("error")) {
        if (!jsonDoc.toVariant().toList().isEmpty()) {
            qCWarning(dcPhilipsHue) << "Failed to discover Hue Bridge lights:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        } else {
            qCWarning(dcPhilipsHue) << "Failed to discover Hue Bridge lights: Invalid error message format";
        }
        return;
    }

    // create Lights if not already added
    QList<DeviceDescriptor> lightDescriptors;
    QList<DeviceDescriptor> whiteLightDescriptors;

    QVariantMap lightsMap = jsonDoc.toVariant().toMap();
    foreach (QString lightId, lightsMap.keys()) {
        QVariantMap lightMap = lightsMap.value(lightId).toMap();

        QString uuid = lightMap.value("uniqueid").toString();
        QString model = lightMap.value("modelid").toString();

        if (lightAlreadyAdded(uuid))
            continue;

        // check if this is a white light
        if (model == "LWB004" || model == "LWB006" || model == "LWB007") {
            DeviceDescriptor descriptor(hueWhiteLightDeviceClassId, "Philips Hue White Light", lightMap.value("name").toString());
            ParamList params;
            params.append(Param("name", lightMap.value("name").toString()));
            params.append(Param("api key", device->paramValue("api key").toString()));
            params.append(Param("bridge", device->id().toString()));
            params.append(Param("host address", device->paramValue("host address").toString()));
            params.append(Param("model id", model));
            params.append(Param("type", lightMap.value("type").toString()));
            params.append(Param("uuid", uuid));
            params.append(Param("light id", lightId));
            descriptor.setParams(params);
            whiteLightDescriptors.append(descriptor);

            qCDebug(dcPhilipsHue) << "Found new white light" << lightMap.value("name").toString() << model;

        } else {
            DeviceDescriptor descriptor(hueLightDeviceClassId, "Philips Hue Light", lightMap.value("name").toString());
            ParamList params;
            params.append(Param("name", lightMap.value("name").toString()));
            params.append(Param("api key", device->paramValue("api key").toString()));
            params.append(Param("bridge", device->id().toString()));
            params.append(Param("host address", device->paramValue("host address").toString()));
            params.append(Param("model id", model));
            params.append(Param("type", lightMap.value("type").toString()));
            params.append(Param("uuid", uuid));
            params.append(Param("light id", lightId));
            descriptor.setParams(params);
            lightDescriptors.append(descriptor);
            qCDebug(dcPhilipsHue) << "Found new color light" << lightMap.value("name").toString() << model;
        }
    }

    if (!lightDescriptors.isEmpty())
        emit autoDevicesAppeared(hueLightDeviceClassId, lightDescriptors);

    if (!whiteLightDescriptors.isEmpty())
        emit autoDevicesAppeared(hueWhiteLightDeviceClassId, whiteLightDescriptors);
}

void DevicePluginPhilipsHue::processBridgeSensorDiscoveryResponse(Device *device, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Bridge sensor discovery json error in response" << error.errorString();
        return;
    }

    // check response error
    if (data.contains("error")) {
        if (!jsonDoc.toVariant().toList().isEmpty()) {
            qCWarning(dcPhilipsHue) << "Failed to discover Hue Bridge sensors:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        } else {
            qCWarning(dcPhilipsHue) << "Failed to discover Hue Bridge sensors: Invalid error message format";
        }
        return;
    }

    // create sensors if not already added
    QList<DeviceDescriptor> sensorDescriptors;
    QVariantMap sensorsMap = jsonDoc.toVariant().toMap();
    foreach (QString sensorId, sensorsMap.keys()) {
        QVariantMap sensorMap = sensorsMap.value(sensorId).toMap();

        QString uuid = sensorMap.value("uniqueid").toString();
        QString model = sensorMap.value("modelid").toString();

        if (sensorAlreadyAdded(uuid))
            continue;

        // check if this is a white light
        if (model == "RWL021" || model == "RWL020") {
            DeviceDescriptor descriptor(hueRemoteDeviceClassId, "Philips Hue Remote", sensorMap.value("name").toString());
            ParamList params;
            params.append(Param("name", sensorMap.value("name").toString()));
            params.append(Param("api key", device->paramValue("api key").toString()));
            params.append(Param("bridge", device->id().toString()));
            params.append(Param("host address", device->paramValue("host address").toString()));
            params.append(Param("model id", model));
            params.append(Param("type", sensorMap.value("type").toString()));
            params.append(Param("uuid", uuid));
            params.append(Param("sensor id", sensorId));
            descriptor.setParams(params);
            sensorDescriptors.append(descriptor);
            qCDebug(dcPhilipsHue) << "Found new remote" << sensorMap.value("name").toString() << model;
        }
    }

    if (!sensorDescriptors.isEmpty())
        emit autoDevicesAppeared(hueRemoteDeviceClassId, sensorDescriptors);

}

void DevicePluginPhilipsHue::processLightRefreshResponse(Device *device, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Hue Bridge json error in response" << error.errorString();
        return;
    }

    // check response error
    if (data.contains("error")) {
        if (!jsonDoc.toVariant().toList().isEmpty()) {
            qCWarning(dcPhilipsHue) << "Failed to refresh Hue Light:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        } else {
            qCWarning(dcPhilipsHue) << "Failed to refresh Hue Light: Invalid error message format";
        }
        return;
    }

    HueLight *light = m_lights.key(device);
    light->updateStates(jsonDoc.toVariant().toMap().value("state").toMap());
}

void DevicePluginPhilipsHue::processBridgeRefreshResponse(Device *device, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Hue Bridge json error in response" << error.errorString();
        return;
    }

    if (!jsonDoc.toVariant().toList().isEmpty()) {
        qCWarning(dcPhilipsHue) << "Failed to refresh Hue Bridge:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        bridgeReachableChanged(device, false);
        return;
    }

    QVariantMap configMap = jsonDoc.toVariant().toMap();

    // mark bridge as reachable
    bridgeReachableChanged(device, true);
    device->setStateValue(apiVersionStateTypeId, configMap.value("apiversion").toString());
    device->setStateValue(softwareVersionStateTypeId, configMap.value("swversion").toString());

    int updateStatus = configMap.value("swupdate").toMap().value("updatestate").toInt();
    switch (updateStatus) {
    case 0:
        device->setStateValue(updateStatusStateTypeId, "Up to date");
        break;
    case 1:
        device->setStateValue(updateStatusStateTypeId, "Downloading updates");
        break;
    case 2:
        device->setStateValue(updateStatusStateTypeId, "Updates ready to install");
        break;
    case 3:
        device->setStateValue(updateStatusStateTypeId, "Installing updates");
        break;
    default:
        break;
    }

    // do lights/sensor update right after successfull bridge update
    HueBridge *bridge = m_bridges.key(device);
    refreshLights(bridge);
}

void DevicePluginPhilipsHue::processLightsRefreshResponse(Device *device, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Hue Bridge json error in response" << error.errorString();
        return;
    }

    // check response error
    if (data.contains("error")) {
        if (!jsonDoc.toVariant().toList().isEmpty()) {
            qCWarning(dcPhilipsHue) << "Failed to refresh Hue Lights:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        } else {
            qCWarning(dcPhilipsHue) << "Failed to refresh Hue Lights: Invalid error message format";
        }
        return;
    }

    // Update light states
    QVariantMap lightsMap = jsonDoc.toVariant().toMap();
    foreach (const QString &lightId, lightsMap.keys()) {
        QVariantMap lightMap = lightsMap.value(lightId).toMap();
        // get the light of this bridge
        foreach (HueLight *light, m_lights.keys()) {
            if (light->id() == lightId.toInt() && light->bridgeId() == device->id()) {
                light->updateStates(lightMap.value("state").toMap());
            }
        }
    }

    if (!m_remotes.isEmpty())
        refreshSensors(m_bridges.key(device));
}

void DevicePluginPhilipsHue::processSensorsRefreshResponse(Device *device, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Hue Bridge json error in response" << error.errorString();
        return;
    }

    // check response error
    if (!jsonDoc.toVariant().toList().isEmpty()) {
        qCWarning(dcPhilipsHue) << "Failed to refresh Hue Sensors:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        return;
    }

    // Update sensor states
    QVariantMap sensorsMap = jsonDoc.toVariant().toMap();
    foreach (const QString &sensorId, sensorsMap.keys()) {
        QVariantMap sensorMap = sensorsMap.value(sensorId).toMap();
        foreach (HueRemote *remote, m_remotes.keys()) {
            if (remote->id() == sensorId.toInt() && remote->bridgeId() == device->id()) {
                //qCDebug(dcPhilipsHue) << "update remote" << remote->id() << remote->name();
                remote->updateStates(sensorMap.value("state").toMap(), sensorMap.value("config").toMap());
            }
        }
    }
}

void DevicePluginPhilipsHue::processSetNameResponse(Device *device, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Hue Bridge json error in response" << error.errorString();
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
        return;
    }

    // check response error
    if (data.contains("error")) {
        if (!jsonDoc.toVariant().toList().isEmpty()) {
            qCWarning(dcPhilipsHue) << "Failed to set name of Hue:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        } else {
            qCWarning(dcPhilipsHue) << "Failed to set name of Hue: Invalid error message format";
        }
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
        return;
    }

    //emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);

    if (device->deviceClassId() == hueLightDeviceClassId || device->deviceClassId() == hueWhiteLightDeviceClassId)
        refreshLight(device);

}

void DevicePluginPhilipsHue::processPairingResponse(PairingInfo *pairingInfo, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Hue Bridge json error in response" << error.errorString();
        emit pairingFinished(pairingInfo->pairingTransactionId(), DeviceManager::DeviceSetupStatusFailure);
        pairingInfo->deleteLater();
        return;
    }

    // check response error
    if (data.contains("error")) {
        if (!jsonDoc.toVariant().toList().isEmpty()) {
            qCWarning(dcPhilipsHue) << "Failed to pair Hue Bridge:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        } else {
            qCWarning(dcPhilipsHue) << "Failed to pair Hue Bridge: Invalid error message format";
        }
        emit pairingFinished(pairingInfo->pairingTransactionId(), DeviceManager::DeviceSetupStatusFailure);
        pairingInfo->deleteLater();
        return;
    }

    pairingInfo->setApiKey(jsonDoc.toVariant().toList().first().toMap().value("success").toMap().value("username").toString());

    qCDebug(dcPhilipsHue) << "Got api key from bridge:" << pairingInfo->apiKey();

    if (pairingInfo->apiKey().isEmpty()) {
        qCWarning(dcPhilipsHue) << "Failed to pair Hue Bridge: did not get any key from the bridge";
        emit pairingFinished(pairingInfo->pairingTransactionId(), DeviceManager::DeviceSetupStatusFailure);
        pairingInfo->deleteLater();
        return;
    }

    // Paired successfully, check bridge information
    QNetworkRequest request(QUrl("http://" + pairingInfo->host().toString() + "/api/" + pairingInfo->apiKey() + "/config"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = networkManagerGet(request);

    m_informationRequests.insert(reply, pairingInfo);
}

void DevicePluginPhilipsHue::processInformationResponse(PairingInfo *pairingInfo, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Hue Bridge json error in response" << error.errorString();
        emit pairingFinished(pairingInfo->pairingTransactionId(), DeviceManager::DeviceSetupStatusFailure);
        pairingInfo->deleteLater();
        return;
    }

    QVariantMap response = jsonDoc.toVariant().toMap();

    // check response error
    if (response.contains("error")) {
        qCWarning(dcPhilipsHue) << "Failed to get information from Hue Bridge:" << response.value("error").toMap().value("description").toString();
        emit pairingFinished(pairingInfo->pairingTransactionId(), DeviceManager::DeviceSetupStatusFailure);
        pairingInfo->deleteLater();
        return;
    }

    // create Bridge
    HueBridge *bridge = new HueBridge(this);
    bridge->setId(response.value("bridgeid").toString());
    bridge->setApiKey(pairingInfo->apiKey());
    bridge->setHostAddress(pairingInfo->host());
    bridge->setApiVersion(response.value("apiversion").toString());
    bridge->setSoftwareVersion(response.value("swversion").toString());
    bridge->setMacAddress(response.value("mac").toString());
    bridge->setName(response.value("name").toString());
    bridge->setZigbeeChannel(response.value("zigbeechannel").toInt());

    if (bridgeAlreadyAdded(bridge->id())) {
        qCWarning(dcPhilipsHue) << "Bridge with id" << bridge->id() << "already added.";
        emit pairingFinished(pairingInfo->pairingTransactionId(), DeviceManager::DeviceSetupStatusFailure);
        bridge->deleteLater();
        pairingInfo->deleteLater();
    }

    m_unconfiguredBridges.append(bridge);

    emit pairingFinished(pairingInfo->pairingTransactionId(), DeviceManager::DeviceSetupStatusSuccess);
    pairingInfo->deleteLater();
}

void DevicePluginPhilipsHue::processActionResponse(Device *device, const ActionId actionId, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Hue Bridge json error in response" << error.errorString();
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareFailure);
        return;
    }

    // check response error
    if (data.contains("error")) {
        if (!jsonDoc.toVariant().toList().isEmpty()) {
            qCWarning(dcPhilipsHue) << "Failed to execute Hue action:" << jsonDoc.toJson(); //jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        } else {
            qCWarning(dcPhilipsHue) << "Failed to execute Hue action: Invalid error message format";
        }
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareFailure);
        return;
    }

    if (device->deviceClassId() != hueBridgeDeviceClassId)
        m_lights.key(device)->processActionResponse(jsonDoc.toVariant().toList());

    emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);
}

void DevicePluginPhilipsHue::bridgeReachableChanged(Device *device, const bool &reachable)
{
    if (reachable) {
        device->setStateValue(bridgeReachableStateTypeId, true);
    } else {
        // mark bridge and corresponding hue devices unreachable
        if (device->deviceClassId() == hueBridgeDeviceClassId) {
            device->setStateValue(bridgeReachableStateTypeId, false);

            foreach (HueLight *light, m_lights.keys()) {
                if (light->bridgeId() == device->id()) {
                    light->setReachable(false);
                    m_lights.value(light)->setStateValue(hueReachableStateTypeId, false);
                }
            }

            foreach (HueRemote *remote, m_remotes.keys()) {
                if (remote->bridgeId() == device->id()) {
                    remote->setReachable(false);
                    m_remotes.value(remote)->setStateValue(hueReachableStateTypeId, false);
                }
            }
        }
    }

}

bool DevicePluginPhilipsHue::bridgeAlreadyAdded(const QString &id)
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == hueBridgeDeviceClassId) {
            if (device->paramValue("id").toString() == id) {
                return true;
            }
        }
    }
    return false;
}

bool DevicePluginPhilipsHue::lightAlreadyAdded(const QString &uuid)
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == hueLightDeviceClassId || device->deviceClassId() == hueWhiteLightDeviceClassId) {
            if (device->paramValue("uuid").toString() == uuid) {
                return true;
            }
        }
    }
    return false;
}

bool DevicePluginPhilipsHue::sensorAlreadyAdded(const QString &uuid)
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == hueRemoteDeviceClassId) {
            if (device->paramValue("uuid").toString() == uuid) {
                return true;
            }
        }
    }
    return false;
}

int DevicePluginPhilipsHue::brightnessToPercentage(int brightness)
{
    return qRound((100.0 * brightness) / 255.0);
}

int DevicePluginPhilipsHue::percentageToBrightness(int percentage)
{
    return qRound((255.0 * percentage) / 100.0);
}
