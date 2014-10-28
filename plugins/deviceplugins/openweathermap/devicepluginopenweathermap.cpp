/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
    \page openweathermap.html
    \title Open Weather Map

    \ingroup plugins
    \ingroup services

    This plugin alows you to get the current weather data from \l{http://www.openweathermap.org}.
    The plugin offers two different search methods: if the user searches for a empty string,
    the plugin makes an autodetction with the WAN ip and offers the user the found autodetectresult.
    The autodetection function uses the geolocation of your WAN ip and searches all available weather
    stations in a radius of 2.5 km. Otherwise the plugin returns the list of the found search results
    from the search string.

    \underline{NOTE}: If you are using a VPN connection, the autodetection will show the results around of your VPN location.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \quotefile plugins/deviceplugins/openweathermap/devicepluginopenweathermap.json
*/


#include "devicepluginopenweathermap.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QJsonDocument>
#include <QVariantMap>
#include <QDateTime>


DevicePluginOpenweathermap::DevicePluginOpenweathermap()
{
    m_openweaher = new OpenWeatherMap(this);
    connect(m_openweaher, &OpenWeatherMap::searchResultReady, this, &DevicePluginOpenweathermap::searchResultsReady);
    connect(m_openweaher, &OpenWeatherMap::weatherDataReady, this, &DevicePluginOpenweathermap::weatherDataReady);
}

DeviceManager::DeviceError DevicePluginOpenweathermap::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    if(deviceClassId != openweathermapDeviceClassId){
        return report(DeviceManager::DeviceErrorDeviceClassNotFound);
    }

    QString location;
    foreach (const Param &param, params) {
        if (param.name() == "location") {
            location = param.value().toString();
        }
    }

    // if we have an empty search string, perform an autodetection of the location with the WAN ip...
    if (location.isEmpty()){
        m_openweaher->searchAutodetect();
    } else {
        m_openweaher->search(location);
    }
    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::DeviceSetupStatus DevicePluginOpenweathermap::setupDevice(Device *device)
{
    foreach (Device *deviceListDevice, deviceManager()->findConfiguredDevices(openweathermapDeviceClassId)) {
        if(deviceListDevice->paramValue("id").toString() == device->paramValue("id").toString()){
            qWarning() << QString("Location " + device->paramValue("location").toString() + "already in added");
            return DeviceManager::DeviceSetupStatusFailure;
        }
    }

    device->setName("Weather from OpenWeatherMap (" + device->paramValue("location").toString() + ")");
    m_openweaher->update(device->paramValue("id").toString(), device->id());

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::HardwareResources DevicePluginOpenweathermap::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

DeviceManager::DeviceError DevicePluginOpenweathermap::executeAction(Device *device, const Action &action)
{
    if(action.actionTypeId() == updateWeatherActionTypeId){
        m_openweaher->update(device->paramValue("id").toString(), device->id());
    }
    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginOpenweathermap::guhTimer()
{
    foreach (Device *device, deviceManager()->findConfiguredDevices(openweathermapDeviceClassId)) {
        m_openweaher->update(device->paramValue("id").toString(), device->id());
    }
}

void DevicePluginOpenweathermap::searchResultsReady(const QList<QVariantMap> &cityList)
{
    QList<DeviceDescriptor> retList;
    foreach (QVariantMap elemant, cityList) {
        DeviceDescriptor descriptor(openweathermapDeviceClassId, elemant.value("name").toString(),elemant.value("country").toString());
        ParamList params;
        Param locationParam("location", elemant.value("name"));
        params.append(locationParam);
        Param countryParam("country", elemant.value("country"));
        params.append(countryParam);
        Param idParam("id", elemant.value("id"));
        params.append(idParam);
        descriptor.setParams(params);
        retList.append(descriptor);
    }
    emit devicesDiscovered(openweathermapDeviceClassId,retList);
}

void DevicePluginOpenweathermap::weatherDataReady(const QByteArray &data, const DeviceId &deviceId)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qWarning() << "failed to parse data" << data << ":" << error.errorString();
        return;
    }

    QVariantMap dataMap = jsonDoc.toVariant().toMap();

    foreach (Device *device, deviceManager()->findConfiguredDevices(openweathermapDeviceClassId)) {
        if(device->id() == deviceId){

            if(dataMap.contains("clouds")){
                int cloudiness = dataMap.value("clouds").toMap().value("all").toInt();
                device->setStateValue(cloudinessStateTypeId,cloudiness);
            }
            if(dataMap.contains("dt")){
                uint lastUpdate = dataMap.value("dt").toUInt();
                device->setStateValue(updateTimeStateTypeId,lastUpdate);
            }

            if(dataMap.contains("main")){
                double temperatur = dataMap.value("main").toMap().value("temp").toDouble();
                double temperaturMax = dataMap.value("main").toMap().value("temp_max").toDouble();
                double temperaturMin = dataMap.value("main").toMap().value("temp_min").toDouble();
                double pressure = dataMap.value("main").toMap().value("pressure").toDouble();
                int humidity = dataMap.value("main").toMap().value("humidity").toInt();

                device->setStateValue(temperatureStateTypeId,temperatur);
                device->setStateValue(temperatureMinStateTypeId,temperaturMin);
                device->setStateValue(temperatureMaxStateTypeId,temperaturMax);
                device->setStateValue(pressureStateTypeId,pressure);
                device->setStateValue(humidityStateTypeId,humidity);
            }

            if(dataMap.contains("sys")){
                uint sunrise = dataMap.value("sys").toMap().value("sunrise").toUInt();
                uint sunset = dataMap.value("sys").toMap().value("sunset").toUInt();

                device->setStateValue(sunriseStateTypeId,sunrise);
                device->setStateValue(sunsetStateTypeId,sunset);
            }

            if(dataMap.contains("weather")){
                QString description = dataMap.value("weather").toMap().value("description").toString();
                device->setStateValue(weatherDescriptionStateTypeId,description);
            }

            if(dataMap.contains("wind")){
                int windDirection = dataMap.value("wind").toMap().value("deg").toInt();
                double windSpeed = dataMap.value("wind").toMap().value("speed").toDouble();

                device->setStateValue(windDirectionStateTypeId,windDirection);
                device->setStateValue(windSpeedStateTypeId,windSpeed);
            }
        }
    }
}


