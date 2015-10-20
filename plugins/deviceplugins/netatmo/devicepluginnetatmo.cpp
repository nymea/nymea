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
    \page netatmo.html
    \title netatmo

    \ingroup plugins
    \ingroup network

    This plugin allows to receive data from you netatmo weather station.

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

    \quotefile plugins/deviceplugins/netatmo/devicepluginnetatmo.json
*/

#include "devicepluginnetatmo.h"
#include "plugin/device.h"
#include "plugininfo.h"

#include <QDebug>
#include <QUrlQuery>
#include <QJsonDocument>

DevicePluginNetatmo::DevicePluginNetatmo()
{
}

DeviceManager::HardwareResources DevicePluginNetatmo::requiredHardware() const
{
    return DeviceManager::HardwareResourceNetworkManager | DeviceManager::HardwareResourceTimer;
}

DeviceManager::DeviceSetupStatus DevicePluginNetatmo::setupDevice(Device *device)
{
    if (device->deviceClassId() == connectionDeviceClassId) {
        qCDebug(dcNetatmo) << "Setup netatmo connection";

        OAuth2 *authentication = new OAuth2("561c015d49c75f0d1cce6e13", "GuvKkdtu7JQlPD47qTTepRR9hQ0CUPAj4Tae3Ohcq", this);
        authentication->setUrl(QUrl("https://api.netatmo.net/oauth2/token"));
        authentication->setUsername(device->paramValue("username").toString());
        authentication->setPassword(device->paramValue("password").toString());
        authentication->setScope("read_station read_thermostat write_thermostat");

        m_authentications.insert(authentication, device);
        m_asyncSetups.append(device);
        connect(authentication, &OAuth2::authenticationChanged, this, &DevicePluginNetatmo::onAuthenticationChanged);

        authentication->startAuthentication();
        return DeviceManager::DeviceSetupStatusAsync;

    } else if (device->deviceClassId() == indoorDeviceClassId) {
        qCDebug(dcNetatmo) << "Setup netatmo indoor base station" << device->params();
        NetatmoBaseStation *indoor = new NetatmoBaseStation(device->paramValue("name").toString(),
                                                            device->paramValue("mac address").toString(),
                                                            device->paramValue("connection").toString(), this);

        device->setParentId(DeviceId(indoor->connectionId()));
        m_indoorDevices.insert(indoor, device);
        connect(indoor, SIGNAL(statesChanged()), this, SLOT(onIndoorStatesChanged()));

        return DeviceManager::DeviceSetupStatusSuccess;
    } else if (device->deviceClassId() == outdoorDeviceClassId) {
        qCDebug(dcNetatmo) << "Setup netatmo outdoor module" << device->params();
        NetatmoOutdoorModule *outdoor = new NetatmoOutdoorModule(device->paramValue("name").toString(),
                                                                 device->paramValue("mac address").toString(),
                                                                 device->paramValue("connection").toString(),
                                                                 device->paramValue("base station").toString(),this);

        device->setParentId(DeviceId(outdoor->connectionId()));
        m_outdoorDevices.insert(outdoor, device);
        connect(outdoor, SIGNAL(statesChanged()), this, SLOT(onOutdoorStatesChanged()));

        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}

void DevicePluginNetatmo::deviceRemoved(Device *device)
{
    if (device->deviceClassId() == connectionDeviceClassId) {
        OAuth2 * authentication = m_authentications.key(device);
        m_authentications.remove(authentication);
        authentication->deleteLater();
    } else if (device->deviceClassId() == indoorDeviceClassId) {
        NetatmoBaseStation *indoor = m_indoorDevices.key(device);
        m_indoorDevices.remove(indoor);
        indoor->deleteLater();
    } else if (device->deviceClassId() == outdoorDeviceClassId) {
        NetatmoOutdoorModule *outdoor = m_outdoorDevices.key(device);
        m_outdoorDevices.remove(outdoor);
        outdoor->deleteLater();
    }
}

void DevicePluginNetatmo::networkManagerReplyReady(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // update values request
    if (m_refreshRequest.keys().contains(reply)) {
        Device *device = m_refreshRequest.take(reply);

        // check HTTP status code
        if (status != 200) {
            qCWarning(dcNetatmo) << "Device list reply HTTP error:" << status << reply->errorString();
            device->setStateValue(availableStateTypeId, false);
            reply->deleteLater();
            return;
        }

        // check JSON file
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll(), &error);
        if (error.error != QJsonParseError::NoError) {
            qCWarning(dcNetatmo) << "Device list reply JSON error:" << error.errorString();
            reply->deleteLater();
            return;
        }

        qCDebug(dcNetatmo) << jsonDoc.toJson();
        processRefreshData(jsonDoc.toVariant().toMap(), device->id().toString());
    }

    reply->deleteLater();
}

void DevicePluginNetatmo::guhTimer()
{
    foreach (OAuth2 *authentication, m_authentications.keys()) {
        if (authentication->authenticated()) {
            refreshData(m_authentications.value(authentication), authentication->token());
        } else {
            authentication->startAuthentication();
        }
    }
}

DeviceManager::DeviceError DevicePluginNetatmo::executeAction(Device *device, const Action &action)
{
    Q_UNUSED(device)
    Q_UNUSED(action)

    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginNetatmo::refreshData(Device *device, const QString &token)
{
    QUrlQuery query;
    query.addQueryItem("access_token", token);

    QUrl url("https://api.netatmo.com/api/devicelist");
    url.setQuery(query);

    QNetworkReply *reply = networkManagerGet(QNetworkRequest(url));
    m_refreshRequest.insert(reply, device);
}

void DevicePluginNetatmo::processRefreshData(const QVariantMap &data, const QString &connectionId)
{
    if (data.contains("body")) {

        // check devices
        if (data.value("body").toMap().contains("devices")) {
            QVariantList deviceList = data.value("body").toMap().value("devices").toList();
            // check devices
            foreach (QVariant deviceVariant, deviceList) {
                QVariantMap deviceMap = deviceVariant.toMap();
                // we support currently only NAMain devices
                if (deviceMap.value("type").toString() == "NAMain") {
                    Device *indoorDevice = findIndoorDevice(deviceMap.value("_id").toString());
                    // check if we have to create the device (auto)
                    if (!indoorDevice) {
                        DeviceDescriptor descriptor(indoorDeviceClassId, "Indoor Station", deviceMap.value("station_name").toString());
                        ParamList params;
                        params.append(Param("name", deviceMap.value("station_name").toString()));
                        params.append(Param("mac address", deviceMap.value("_id").toString()));
                        params.append(Param("connection", connectionId));
                        descriptor.setParams(params);
                        emit autoDevicesAppeared(indoorDeviceClassId, QList<DeviceDescriptor>() << descriptor);
                    } else {
                        if (m_indoorDevices.values().contains(indoorDevice)) {
                            m_indoorDevices.key(indoorDevice)->updateStates(deviceMap);
                        }
                    }
                }
            }
        }

        // check modules
        if (data.value("body").toMap().contains("modules")) {
            QVariantList modulesList = data.value("body").toMap().value("modules").toList();
            // check devices
            foreach (QVariant moduleVariant, modulesList) {
                QVariantMap moduleMap = moduleVariant.toMap();
                // we support currently only NAModule1
                if (moduleMap.value("type").toString() == "NAModule1") {
                    Device *outdoorDevice = findOutdoorDevice(moduleMap.value("_id").toString());
                    // check if we have to create the device (auto)
                    if (!outdoorDevice) {
                        DeviceDescriptor descriptor(outdoorDeviceClassId, "Outdoor Module", moduleMap.value("module_name").toString());
                        ParamList params;
                        params.append(Param("name", moduleMap.value("module_name").toString()));
                        params.append(Param("mac address", moduleMap.value("_id").toString()));
                        params.append(Param("connection", connectionId));
                        params.append(Param("base station", moduleMap.value("main_device").toString()));
                        descriptor.setParams(params);
                        emit autoDevicesAppeared(outdoorDeviceClassId, QList<DeviceDescriptor>() << descriptor);
                    } else {
                        if (m_outdoorDevices.values().contains(outdoorDevice)) {
                            m_outdoorDevices.key(outdoorDevice)->updateStates(moduleMap);
                        }
                    }
                }
            }
        }
    }
}

Device *DevicePluginNetatmo::findIndoorDevice(const QString &macAddress)
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == indoorDeviceClassId) {
            if (device->paramValue("mac address").toString() == macAddress) {
                return device;
            }
        }
    }
    return 0;
}

Device *DevicePluginNetatmo::findOutdoorDevice(const QString &macAddress)
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == outdoorDeviceClassId) {
            if (device->paramValue("mac address").toString() == macAddress) {
                return device;
            }
        }
    }
    return 0;
}

void DevicePluginNetatmo::onAuthenticationChanged()
{
    OAuth2 *authentication = static_cast<OAuth2 *>(sender());
    Device *device = m_authentications.value(authentication);

    if (!device)
        return;

    // set the available state
    device->setStateValue(availableStateTypeId, authentication->authenticated());

    // check if this is was a setup athentication
    if (m_asyncSetups.contains(device)) {
        if (authentication->authenticated()) {
            m_asyncSetups.removeAll(device);
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);
            refreshData(device, authentication->token());
        } else {
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
        }
    }
}

void DevicePluginNetatmo::onIndoorStatesChanged()
{
    NetatmoBaseStation *indoor = static_cast<NetatmoBaseStation *>(sender());
    Device *device = m_indoorDevices.value(indoor);

    device->setStateValue(updateTimeStateTypeId, indoor->lastUpdate());
    device->setStateValue(temperatureStateTypeId, indoor->temperature());
    device->setStateValue(temperatureMinStateTypeId, indoor->minTemperature());
    device->setStateValue(temperatureMaxStateTypeId, indoor->maxTemperature());
    device->setStateValue(pressureStateTypeId, indoor->pressure());
    device->setStateValue(humidityStateTypeId, indoor->humidity());
    device->setStateValue(co2StateTypeId, indoor->co2());
    device->setStateValue(noiseStateTypeId, indoor->noise());
    device->setStateValue(wifiStrengthStateTypeId, indoor->wifiStrength());
}

void DevicePluginNetatmo::onOutdoorStatesChanged()
{
    NetatmoOutdoorModule *outdoor = static_cast<NetatmoOutdoorModule *>(sender());
    Device *device = m_outdoorDevices.value(outdoor);

    device->setStateValue(updateTimeStateTypeId, outdoor->lastUpdate());
    device->setStateValue(temperatureStateTypeId, outdoor->temperature());
    device->setStateValue(temperatureMinStateTypeId, outdoor->minTemperature());
    device->setStateValue(temperatureMaxStateTypeId, outdoor->maxTemperature());
    device->setStateValue(humidityStateTypeId, outdoor->humidity());
    device->setStateValue(signalStrengthStateTypeId, outdoor->signalStrength());
    device->setStateValue(batteryStateTypeId, outdoor->battery());
}


