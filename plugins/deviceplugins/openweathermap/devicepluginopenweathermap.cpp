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
    Otherwise the plugin return the list with the found searchresults.

    \section1 Examples
    \section2 Autodetect location
    If you want to autodetect your location dend a discovery request with an empty string.
    \code
    {
        "id":1,
        "method":"Devices.GetDiscoveredDevices",
        "params":{
            "deviceClassId":"985195aa-17ad-4530-88a4-cdd753d747d7",
            "discoveryParams": {
                "location":""
            }
        }
    }
    \endcode
    response from autodetection...
    \code
    {
        "id": 1,
        "params": {
            "deviceDescriptors": [
                {
                    "description": "AT",
                    "id": "{75607672-5354-428f-a752-910140c22b18}",
                    "title": "Vienna"
                }
            ],
            "errorMessage": "",
            "success": true
        },
        "status": "success"
    }
    \endcode
    \section2 Searching city
    If you want to search a string send following discovery message:
    \code
    {
        "id":1,
        "method":"Devices.GetDiscoveredDevices",
        "params":{
            "deviceClassId":"985195aa-17ad-4530-88a4-cdd753d747d7",
            "discoveryParams": {
                "location":"Vie"
            }
        }
    }
    \endcode
    response...
    \code
    {
        "id": 1,
        "params": {
            "deviceDescriptors": [
                {
                    "description": "DE",
                    "id": "{6dc6be43-5bdc-4dbd-bcbf-6f8e1f90000b}",
                    "title": "Viersen"
                },
                {
                    "description": "VN",
                    "id": "{af275298-77f1-40b4-843a-d0f3c7aef6bb}",
                    "title": "Viet Tri"
                },
                {
                    "description": "DE",
                    "id": "{86a4ab63-41b4-4348-9830-4bf6c87474bf}",
                    "title": "Viernheim"
                },
                {
                    "description": "AR",
                    "id": "{3b5f8eea-6159-4375-bd01-1f07de9c3a9d}",
                    "title": "Viedma"
                },
                {
                    "description": "FR",
                    "id": "{f3b91f26-3275-4bb4-a594-924202a2124e}",
                    "title": "Vierzon"
                },
                {
                    "description": "AT",
                    "id": "{b59d15f7-f52b-43a0-a9c5-a3fa80cbc2bd}",
                    "title": "Vienna"
                }
            ],
            "errorMessage": "",
            "success": true
        },
        "status": "success"
    }
    \endcode
    \section2 Adding a discovered city
    If you want to add a dicovered city send the add "AddConfiguredDevice" message
    with the deviceDescriptorId from the searchresult list. In this example the id for Vienna.
    \code
    {
        "id":1,
        "method":"Devices.AddConfiguredDevice",
        "params":{
            "deviceClassId":"985195aa-17ad-4530-88a4-cdd753d747d7",
            "deviceDescriptorId": "b59d15f7-f52b-43a0-a9c5-a3fa80cbc2bd"
        }
    }
    \endcode
    response...
    \code
    {
        "id": 1,
        "params": {
            "deviceId": "{af0f1958-b901-48da-ad97-d4d64af88cf8}",
            "errorMessage": "",
            "success": true
        },
        "status": "success"
    }
    \endcode

    \section1 Plugin propertys:
        \section2 Plugin parameters
        Each configured plugin has following paramters:

        \table
            \header
                \li Name
                \li Description
                \li Data Type
            \row
                \li location
                \li This parameter holds the name of the city
                \li string
            \row
                \li country
                \li This parameter holds the country of the city
                \li string
            \row
                \li id
                \li This parameter holds the city id from \l{http://www.openweathermap.org}
                \li string
        \endtable

        \section2 Actions
        Following list contains all plugin \l{Action}s:
            \table
            \header
                \li Name
                \li Description
                \li UUID
            \row
                \li refresh
                \li This action refreshes all states.
                \li cfbc6504-d86f-4856-8dfa-97b6fbb385e4
            \endtable

        \section2 States
        Following list contains all plugin \l{State}s:
            \table
            \header
                \li Name
                \li Description
                \li UUID
                \li Data Type
            \row
                \li city name
                \li The name of the city
                \li fd9e7b7f-cf1f-4093-8f6d-fff5b223471f
                \li string
            \row
                \li city id
                \li The city ID for openweathermap.org
                \li c6ef1c07-e817-4251-b83d-115bbf6f0ae9
                \li string
            \row
                \li country name
                \li The country name
                \li 0e607a5f-1938-4e77-a146-15e9ad15bfad
                \li string
            \row
                \li last update
                \li The timestamp of the weather data from the weatherstation in unixtime
                    format
                \li 98e48095-87da-47a4-b812-28c6c17a3e76
                \li unsignend int
            \row
                \li temperature
                \li Current temperature [Celsius]
                \li 2f949fa3-ff21-4721-87ec-0a5c9d0a5b8a
                \li double
            \row
                \li temperature minimum
                \li Today temperature minimum [Clesius]
                \li 701338b3-80de-4c95-8abf-26f44529d620
                \li double
            \row
                \li temperature maximum
                \li Today temperature maximum [Clesius]
                \li f69bedd2-c997-4a7d-9242-76bf2aab3d3d
                \li double
            \row
                \li humidity
                \li Current relative humidity  [%]
                \li 3f01c9f0-206b-4477-afa2-80d6e5e54fbb
                \li int
            \row
                \li pressure
                \li Current pressure [hPa]
                \li 6a57b6e9-7010-4a89-982c-ce0bc2a71f11
                \li double
            \row
                \li wind speed
                \li Current wind speed [m/s]
                \li 12dc85a9-825d-4375-bef4-abd66e9e301b
                \li double
            \row
                \li wind direction
                \li The wind direction rellative to the north pole [degree]
                \li a8b0383c-d615-41fe-82b8-9b797f045cc9
                \li int
            \row
                \li cloudiness
                \li This value represents how much of the sky is clowdy [%]
                \li 0c1dc881-560e-40ac-a4a1-9ab69138cfe3
                \li int
            \row
                \li weather description
                \li This string describes the current weather condition in clear words
                \li e71d98e3-ebd8-4abf-ad25-9ecc2d05276a
                \li string
            \row
                \li sunset
                \li The time of todays sunset in unixtime format
                \li 5dd6f5a3-25d6-4e60-82ca-e934ad76a4b6
                \li unsigned int
            \row
                \li sunrise
                \li The time of todays sunrise in unixtime format
                \li 413b3fc6-bd1c-46fb-8c86-03096254f94f
                \li unsigned int
            \endtable
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


