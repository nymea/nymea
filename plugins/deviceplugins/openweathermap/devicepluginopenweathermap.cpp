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

#include <QDebug>
#include <QJsonDocument>
#include <QVariantMap>
#include <QDateTime>

DeviceClassId openweathermapDeviceClassId = DeviceClassId("985195aa-17ad-4530-88a4-cdd753d747d7");

ActionTypeId updateWeatherActionTypeId = ActionTypeId("cfbc6504-d86f-4856-8dfa-97b6fbb385e4");

StateTypeId updateTimeStateTypeId = StateTypeId("36b2f09b-7d77-4fbc-a68f-23d735dda0b1");
StateTypeId temperatureStateTypeId = StateTypeId("6013402f-b5b1-46b3-8490-f0c20d62fe61");
StateTypeId temperatureMinStateTypeId = StateTypeId("14ec2781-cb04-4bbf-b097-7d01ef982630");
StateTypeId temperatureMaxStateTypeId = StateTypeId("fefe5563-452f-4833-b5cf-49c3cc67c772");
StateTypeId humidityStateTypeId = StateTypeId("6f32ec73-3240-4630-ada9-1c10b8e98123");
StateTypeId pressureStateTypeId = StateTypeId("4a42eea9-00eb-440b-915e-dbe42180f83b");
StateTypeId windSpeedStateTypeId = StateTypeId("2bf63430-e9e2-4fbf-88e6-6f1b4770f287");
StateTypeId windDirectionStateTypeId = StateTypeId("589e2ea5-65b2-4afd-9b72-e3708a589a12");
StateTypeId cloudinessStateTypeId = StateTypeId("798553bc-45c7-42eb-9105-430bddb5d9b7");
StateTypeId weatherDescriptionStateTypeId = StateTypeId("f9539108-0e0e-4736-a306-6408f8e20a26");
StateTypeId sunriseStateTypeId = StateTypeId("af155e94-9492-44e1-8608-7d0ee8b5d50d");
StateTypeId sunsetStateTypeId = StateTypeId("a1dddc3d-549f-4f20-b78b-be850548f286");


DevicePluginOpenweathermap::DevicePluginOpenweathermap()
{
    m_openweaher = new OpenWeatherMap(this);
    connect(m_openweaher, &OpenWeatherMap::searchResultReady, this, &DevicePluginOpenweathermap::searchResultsReady);
    connect(m_openweaher, &OpenWeatherMap::weatherDataReady, this, &DevicePluginOpenweathermap::weatherDataReady);
}

DeviceManager::DeviceError DevicePluginOpenweathermap::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    if(deviceClassId != openweathermapDeviceClassId){
        return DeviceManager::DeviceErrorDeviceClassNotFound;
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

    // otherwise search the given string
    m_openweaher->search(location);
    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::DeviceSetupStatus DevicePluginOpenweathermap::setupDevice(Device *device)
{
    foreach (Device *deviceListDevice, deviceManager()->findConfiguredDevices(openweathermapDeviceClassId)) {
        if(deviceListDevice->paramValue("id").toString() == device->paramValue("id").toString()){
            qWarning() << QString("Location " + device->paramValue("location").toString() + " already added.");
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


