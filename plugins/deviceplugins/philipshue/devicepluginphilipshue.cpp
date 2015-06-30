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
}

DeviceManager::HardwareResources DevicePluginPhilipsHue::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer | DeviceManager::HardwareResourceUpnpDisovery | DeviceManager::HardwareResourceNetworkManager;
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
            if (b->apiKey() == device->paramValue("api key").toString()) {
                m_unconfiguredBridges.removeOne(b);

                // set data which was not known during discovery
                device->setParamValue("name", b->name());
                device->setParamValue("zigbee channel", b->zigbeeChannel());
                device->setParamValue("api version", b->apiVersion());
                device->setParamValue("mac address", b->macAddress());
                device->setStateValue(bridgeReachableStateTypeId, true);
                m_bridges.insert(b, device);

                // now add the child lights from this bridge as auto device
                QList<DeviceDescriptor> descriptors;
                foreach (HueLight *light, b->lights()) {
                    DeviceDescriptor descriptor(hueLightDeviceClassId, "Philips Hue Light", light->name());
                    ParamList params;
                    params.append(Param("name", light->name()));
                    params.append(Param("api key", light->apiKey()));
                    params.append(Param("bridge", device->id().toString()));
                    params.append(Param("host address", light->hostAddress().toString()));
                    params.append(Param("model id", light->modelId()));
                    params.append(Param("light id", light->lightId()));
                    descriptor.setParams(params);
                    descriptors.append(descriptor);
                }
                emit autoDevicesAppeared(hueLightDeviceClassId, descriptors);

                return DeviceManager::DeviceSetupStatusSuccess;
            }
        }

        // loaded bridge
        HueBridge *bridge = new HueBridge(device->paramValue("api key").toString(),
                                          QHostAddress(device->paramValue("host address").toString()));

        bridge->setApiVersion(device->paramValue("api version").toString());
        bridge->setMacAddress(device->paramValue("mac address").toString());
        bridge->setName(device->paramValue("name").toString());
        bridge->setZigbeeChannel(device->paramValue("zigbee channel").toInt());

        m_bridges.insert(bridge, device);
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    // hue lights
    if (device->deviceClassId() == hueLightDeviceClassId) {

        HueLight *hueLight = 0;

        // check if this is a unconfigured light
        for (int i = 0; i < m_unconfiguredLights.count(); i++) {
            if (m_unconfiguredLights.at(i)->apiKey() == device->paramValue("api key").toString()) {
                hueLight = m_unconfiguredLights.takeAt(i);
                break;
            }
        }

        // check if this is a light from settings
        if (hueLight == 0) {
            hueLight = new HueLight(device->paramValue("light id").toInt(),
                                    QHostAddress(device->paramValue("host address").toString()),
                                    device->paramValue("name").toString(),
                                    device->paramValue("api key").toString(),
                                    device->paramValue("model id").toString(),
                                    DeviceId(device->paramValue("bridge").toString()),
                                    this);

            connect(hueLight, &HueLight::stateChanged, this, &DevicePluginPhilipsHue::lightStateChanged);
        }

        m_lights.insert(hueLight, device);
        setLightName(device, device->paramValue("name").toString());
    }

    return DeviceManager::DeviceSetupStatusAsync;
}

void DevicePluginPhilipsHue::deviceRemoved(Device *device)
{
    if (device->deviceClassId() == hueBridgeDeviceClassId) {
        HueBridge *bridge = m_bridges.key(device);
        m_bridges.remove(bridge);
        bridge->deleteLater();

        // TODO: remove lights from this bridge (over GuhCore)
    }

    if (device->deviceClassId() == hueLightDeviceClassId) {
        HueLight *light = m_lights.key(device);
        m_lights.remove(light);
        light->deleteLater();
    }
}

void DevicePluginPhilipsHue::upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList)
{
    QList<DeviceDescriptor> deviceDescriptors;
    foreach (const UpnpDeviceDescriptor &upnpDevice, upnpDeviceDescriptorList) {
        if (upnpDevice.modelDescription().contains("Philips")) {
            DeviceDescriptor descriptor(hueBridgeDeviceClassId, "Philips Hue Bridge", upnpDevice.hostAddress().toString());
            ParamList params;
            params.append(Param("name", QString()));
            params.append(Param("host address", upnpDevice.hostAddress().toString()));
            params.append(Param("api key", "guh-" + QUuid::createUuid().toString().remove(QRegExp("[\\{\\}]*")).remove(QRegExp("\\-[0-9a-f\\-]*"))));
            params.append(Param("mac address", QString()));
            params.append(Param("api version", QString()));
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

    if (deviceClassId != hueBridgeDeviceClassId) {
        return DeviceManager::DeviceSetupStatusFailure;
    }

    PairingInfo pairingInfo;
    pairingInfo.pairingTransactionId = pairingTransactionId;
    pairingInfo.host = QHostAddress(params.paramValue("host address").toString());
    pairingInfo.apiKey = params.paramValue("api key").toString();

    QVariantMap createUserParams;
    createUserParams.insert("devicetype", "guh");
    createUserParams.insert("username", pairingInfo.apiKey);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(createUserParams);

    QNetworkRequest request(QUrl("http://" + pairingInfo.host.toString() + "/api"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkManagerPost(request, jsonDoc.toJson());

    m_pairingRequests.insert(reply, pairingInfo);

    return DeviceManager::DeviceSetupStatusAsync;
}

void DevicePluginPhilipsHue::networkManagerReplyReady(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // create user finished
    if (m_pairingRequests.keys().contains(reply)) {
        PairingInfo pairingInfo = m_pairingRequests.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcPhilipsHue) << "Request error:" << status << reply->errorString();
            reply->deleteLater();
            return;
        }
        processPairingResponse(pairingInfo, reply->readAll());

    } else if (m_informationRequests.keys().contains(reply)) {
        PairingInfo pairingInfo = m_informationRequests.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcPhilipsHue) << "Request error:" << status << reply->errorString();
            reply->deleteLater();
            return;
        }
        processInformationResponse(pairingInfo, reply->readAll());

    } else if (m_lightRefreshRequests.keys().contains(reply)) {

        Device *device = m_lightRefreshRequests.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcPhilipsHue) << "Refresh Hue Light request error:" << status << reply->errorString();
            onBridgeError(device);
            reply->deleteLater();
            return;
        }
        processLightRefreshResponse(device, reply->readAll());

    } else if (m_bridgeRefreshRequests.keys().contains(reply)) {

        Device *device = m_bridgeRefreshRequests.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcPhilipsHue) << "Refresh Hue Bridge request error:" << status << reply->errorString();
            onBridgeError(device);
            reply->deleteLater();
            return;
        }
        processBridgeRefreshResponse(device, reply->readAll());

    } else if (m_asyncActions.keys().contains(reply)) {

        QPair<Device *, ActionId> actionInfo = m_asyncActions.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcPhilipsHue) << "Refresh Hue Light request error:" << status << reply->errorString();
            onBridgeError(actionInfo.first);
            emit actionExecutionFinished(actionInfo.second, DeviceManager::DeviceErrorHardwareNotAvailable);
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        processActionResponse(actionInfo.first, actionInfo.second, data);
    } else if (m_lightSetNameRequests.keys().contains(reply)) {

        Device *device = m_lightSetNameRequests.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcPhilipsHue) << "Set name of Hue Light request error:" << status << reply->errorString();
            reply->deleteLater();
            return;
        }
        processSetNameResponse(device, reply->readAll());
    }

    reply->deleteLater();
}

void DevicePluginPhilipsHue::guhTimer()
{
    foreach (Device *device, m_bridges.values()) {
        refreshBridge(device);
    }
}

DeviceManager::DeviceError DevicePluginPhilipsHue::executeAction(Device *device, const Action &action)
{
    if (device->deviceClassId() == hueLightDeviceClassId) {
        HueLight *light = m_lights.key(device);

        if (!light->reachable()) {
            return DeviceManager::DeviceErrorHardwareNotAvailable;
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

    if (device->deviceClassId() == hueBridgeDeviceClassId) {
        // TODO: search if a light was added or removed from bridge
        return DeviceManager::DeviceErrorNoError;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginPhilipsHue::lightStateChanged()
{
    HueLight *light = static_cast<HueLight *>(sender());

    Device *device = m_lights.value(light);
    if (!device)
        return;

    device->setStateValue(hueReachableStateTypeId, light->reachable());
    device->setStateValue(hueColorStateTypeId, QVariant::fromValue(light->color()));
    device->setStateValue(huePowerStateTypeId, light->power());
    device->setStateValue(hueBrightnessStateTypeId, brightnessToPercentage(light->brightness()));
    device->setStateValue(hueTemperatureStateTypeId, light->ct());
    device->setStateValue(hueEffectStateTypeId, light->effect());
}

void DevicePluginPhilipsHue::refreshLight(Device *device)
{
    HueLight *light = m_lights.key(device);

    QNetworkRequest request(QUrl("http://" + light->hostAddress().toString() + "/api/" + light->apiKey() + "/lights/" + QString::number(light->lightId())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkManagerGet(request);

    m_lightRefreshRequests.insert(reply, device);
}

void DevicePluginPhilipsHue::refreshBridge(Device *device)
{
    HueBridge *bridge = m_bridges.key(device);

    QNetworkRequest request(QUrl("http://" + bridge->hostAddress().toString() + "/api/" + bridge->apiKey() + "/lights/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkManagerGet(request);

    m_bridgeRefreshRequests.insert(reply, device);
}

void DevicePluginPhilipsHue::setLightName(Device *device, QString name)
{
    HueLight *light = m_lights.key(device);

    QVariantMap requestMap;
    requestMap.insert("name", name);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(requestMap);

    QNetworkRequest request(QUrl("http://" + light->hostAddress().toString() + "/api/" + light->apiKey() +
                                 "/lights/" + QString::number(light->lightId())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply = networkManagerPut(request,jsonDoc.toJson());
    m_lightSetNameRequests.insert(reply, device);
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
        qCWarning(dcPhilipsHue) << "Failed to refresh Hue Light:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        return;
    }

    HueLight *hueLight = m_lights.key(device);
    hueLight->updateStates(jsonDoc.toVariant().toMap().value("state").toMap());
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

    // check response error
    if (data.contains("error")) {
        qCWarning(dcPhilipsHue) << "Failed to refresh Hue Bridge:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        return;
    }

    // mark bridge as reachable
    device->setStateValue(bridgeReachableStateTypeId, true);

    QVariantMap lightsMap = jsonDoc.toVariant().toMap();
    foreach (const QString &lightId, lightsMap.keys()) {
        QVariantMap lightMap = lightsMap.value(lightId).toMap();
        // get the light of this bridge
        foreach (HueLight *light, m_lights.keys()) {
            if (light->lightId() == lightId.toInt() && light->bridgeId() == device->id()) {
                light->updateStates(lightMap.value("state").toMap());
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
        qCWarning(dcPhilipsHue) << "Failed to set name of Hue:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
        return;
    }

    emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);

    if (device->deviceClassId() == hueLightDeviceClassId) {
        refreshLight(device);
    }
}

void DevicePluginPhilipsHue::processPairingResponse(const DevicePluginPhilipsHue::PairingInfo &pairingInfo, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Hue Bridge json error in response" << error.errorString();
        emit pairingFinished(pairingInfo.pairingTransactionId, DeviceManager::DeviceSetupStatusFailure);
        return;
    }

    // check response error
    if (data.contains("error")) {
        qCWarning(dcPhilipsHue) << "Failed to pair Hue Bridge:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        emit pairingFinished(pairingInfo.pairingTransactionId, DeviceManager::DeviceSetupStatusFailure);
        return;
    }

    // Paired successfully, check bridge/light information
    QNetworkRequest request(QUrl("http://" + pairingInfo.host.toString() + "/api/" + pairingInfo.apiKey + ""));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = networkManagerGet(request);

    m_informationRequests.insert(reply, pairingInfo);
}

void DevicePluginPhilipsHue::processInformationResponse(const DevicePluginPhilipsHue::PairingInfo &pairingInfo, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Hue Bridge json error in response" << error.errorString();
        emit pairingFinished(pairingInfo.pairingTransactionId, DeviceManager::DeviceSetupStatusFailure);
        return;
    }

    QVariantMap response = jsonDoc.toVariant().toMap();

    // check response error
    if (response.contains("error")) {
        qCWarning(dcPhilipsHue) << "Failed to get information from Hue Bridge:" << response.value("error").toMap().value("description").toString();
        emit pairingFinished(pairingInfo.pairingTransactionId, DeviceManager::DeviceSetupStatusFailure);
        return;
    }

    // create Bridge
    HueBridge *bridge = new HueBridge(pairingInfo.apiKey, pairingInfo.host);

    bridge->setApiVersion(response.value("config").toMap().value("apiversion").toString());
    bridge->setMacAddress(response.value("config").toMap().value("mac").toString());
    bridge->setName(response.value("config").toMap().value("name").toString());
    bridge->setZigbeeChannel(response.value("config").toMap().value("zigbeechannel").toInt());

    m_unconfiguredBridges.append(bridge);

    // create Lights
    QVariantMap lightsMap = response.value("lights").toMap();
    foreach (QString lightId, lightsMap.keys()) {
        QVariantMap lightMap = lightsMap.value(lightId).toMap();
        HueLight *hueLight = new HueLight(lightId.toInt(),
                                          bridge->hostAddress(),
                                          lightMap.value("name").toString(),
                                          pairingInfo.apiKey,
                                          lightMap.value("modelid").toString(),
                                          DeviceId(),
                                          this);

        hueLight->updateStates(lightMap.value("state").toMap());

        bridge->addLight(hueLight);
        m_unconfiguredLights.append(hueLight);

        connect(hueLight, &HueLight::stateChanged, this, &DevicePluginPhilipsHue::lightStateChanged);
    }

    emit pairingFinished(pairingInfo.pairingTransactionId, DeviceManager::DeviceSetupStatusSuccess);
}

void DevicePluginPhilipsHue::processActionResponse(Device *device, const ActionId actionId, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // check JSON error
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcPhilipsHue) << "Hue Bridge json error in response" << error.errorString();
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareNotAvailable);
        return;
    }

    // check response error
    if (data.contains("error")) {
        qCWarning(dcPhilipsHue) << "Failed to execute Hue action:" << jsonDoc.toVariant().toList().first().toMap().value("error").toMap().value("description").toString();
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareNotAvailable);
        return;
    }

    m_lights.key(device)->processActionResponse(jsonDoc.toVariant().toList());
    emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);
}

void DevicePluginPhilipsHue::onBridgeError(Device *device)
{
    // mark bridge and lamps unreachable
    if (device->deviceClassId() == hueBridgeDeviceClassId) {
        device->setStateValue(bridgeReachableStateTypeId, false);
        foreach (HueLight *light, m_lights.keys()) {
            if (light->bridgeId() == device->id()) {
                device->setStateValue(hueReachableStateTypeId, false);
            }
        }
    } else if (device->deviceClassId() == hueLightDeviceClassId) {
        DeviceId bridgeId = DeviceId(device->paramValue("bridge").toString());
        // mark lamps as unreachable
        foreach (HueLight *light, m_lights.keys()) {
            if (light->bridgeId() == bridgeId) {
                device->setStateValue(hueReachableStateTypeId, false);
            }
        }
        // mark bridge as unreachable
        foreach (Device *d, m_bridges.values()) {
            if (d->id() == bridgeId) {
                d->setStateValue(bridgeReachableStateTypeId, false);
                return;
            }
        }
    }
}

int DevicePluginPhilipsHue::brightnessToPercentage(int brightness)
{
    return (int)(((100.0 * brightness) / 255.0) + 0.5);
}

int DevicePluginPhilipsHue::percentageToBrightness(int percentage)
{
    return (int)(((255.0 * percentage) / 100.0) + 0.5);
}
