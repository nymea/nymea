/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

/*!
    \page openweathermap.html
    \title Open Weather Map

    \ingroup plugins
    \ingroup services

    This plugin gives the possibility to get weather data...

    \chapter Plugin propertys:
        \section1 Actions
        Following list contains all plugin \l{Action}s:
            \table
            \header
                \li Name
                \li Description
                \li UUID
            \row
                \li refresh
                \li This action refreshes all states. Only, if a state value realy changed
                    a notification get emited.
                \li cfbc6504-d86f-4856-8dfa-97b6fbb385e4
            \endtable
        \section1 Events:
        Following list contains all plugin \l{Event}s:
            \table
            \header
                \li Name
                \li UUID
                \li Description
            \row
                \li
                \li
                \li
            \endtable
        \section1 States
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
#include <QStringList>
#include <QJsonDocument>
#include <QDateTime>


VendorId openweathermapVendorId = VendorId("bf1e96f0-9650-4e7c-a56c-916d54d18e7a");

DevicePluginOpenweathermap::DevicePluginOpenweathermap()
{
    m_openweaher = new OpenWeatherMap(this);
}

QList<Vendor> DevicePluginOpenweathermap::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor openweathermap(openweathermapVendorId, "openweathermap");
    ret.append(openweathermap);
    return ret;
}

QList<DeviceClass> DevicePluginOpenweathermap::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassOpenweathermap(pluginId(), openweathermapVendorId, DeviceClassId("985195aa-17ad-4530-88a4-cdd753d747d7"));
    deviceClassOpenweathermap.setName("Weather from openweathermap");
    //deviceClassOpenweathermap.setCreateMethod(DeviceClass::CreateMethodDiscovery);

    // Params
    QVariantList params;
    QVariantMap locationParam;
    locationParam.insert("name", "location");
    locationParam.insert("type", "string");
    params.append(locationParam);
    deviceClassOpenweathermap.setParams(params);

    // Actions
    QList<ActionType> weatherActions;
    ActionType updateWeather(ActionTypeId("cfbc6504-d86f-4856-8dfa-97b6fbb385e4"));
    updateWeather.setName("refresh");
    weatherActions.append(updateWeather);

    // States
    QList<StateType> weatherStates;
    StateType cityNameState(StateTypeId("fd9e7b7f-cf1f-4093-8f6d-fff5b223471f"));
    cityNameState.setName("city name");
    cityNameState.setType(QVariant::String);
    cityNameState.setDefaultValue("");
    weatherStates.append(cityNameState);

    StateType cityIdState(StateTypeId("c6ef1c07-e817-4251-b83d-115bbf6f0ae9"));
    cityIdState.setName("city id");
    cityIdState.setType(QVariant::String);
    cityIdState.setDefaultValue("");
    weatherStates.append(cityIdState);

    StateType countryNameState(StateTypeId("0e607a5f-1938-4e77-a146-15e9ad15bfad"));
    countryNameState.setName("country name");
    countryNameState.setType(QVariant::String);
    countryNameState.setDefaultValue("");
    weatherStates.append(countryNameState);

    StateType updateTimeState(StateTypeId("98e48095-87da-47a4-b812-28c6c17a3e76"));
    updateTimeState.setName("last update [unixtime]");
    updateTimeState.setType(QVariant::UInt);
    updateTimeState.setDefaultValue(0);
    weatherStates.append(updateTimeState);

    StateType temperatureState(StateTypeId("2f949fa3-ff21-4721-87ec-0a5c9d0a5b8a"));
    temperatureState.setName("temperature [°C]");
    temperatureState.setType(QVariant::Double);
    temperatureState.setDefaultValue(-999.9);
    weatherStates.append(temperatureState);

    StateType temperatureMinState(StateTypeId("701338b3-80de-4c95-8abf-26f44529d620"));
    temperatureMinState.setName("temperature minimum [Celsius]");
    temperatureMinState.setType(QVariant::Double);
    temperatureMinState.setDefaultValue(-999.9);
    weatherStates.append(temperatureMinState);

    StateType temperatureMaxState(StateTypeId("f69bedd2-c997-4a7d-9242-76bf2aab3d3d"));
    temperatureMaxState.setName("temperature maximum [Celsius]");
    temperatureMaxState.setType(QVariant::Double);
    temperatureMaxState.setDefaultValue(999.9);
    weatherStates.append(temperatureMaxState);

    StateType humidityState(StateTypeId("3f01c9f0-206b-4477-afa2-80d6e5e54fbb"));
    humidityState.setName("humidity [%]");
    humidityState.setType(QVariant::Int);
    humidityState.setDefaultValue(-1);
    weatherStates.append(humidityState);

    StateType pressureState(StateTypeId("6a57b6e9-7010-4a89-982c-ce0bc2a71f11"));
    pressureState.setName("pressure [hPa]");
    pressureState.setType(QVariant::Double);
    pressureState.setDefaultValue(-1);
    weatherStates.append(pressureState);

    StateType windSpeedState(StateTypeId("12dc85a9-825d-4375-bef4-abd66e9e301b"));
    windSpeedState.setName("wind speed [m/s]");
    windSpeedState.setType(QVariant::Double);
    windSpeedState.setDefaultValue(-1);
    weatherStates.append(windSpeedState);

    StateType windDirectionState(StateTypeId("a8b0383c-d615-41fe-82b8-9b797f045cc9"));
    windDirectionState.setName("wind direction [°]");
    windDirectionState.setType(QVariant::Int);
    windDirectionState.setDefaultValue(-1);
    weatherStates.append(windDirectionState);

    StateType cloudinessState(StateTypeId("0c1dc881-560e-40ac-a4a1-9ab69138cfe3"));
    cloudinessState.setName("cloudiness [%]");
    cloudinessState.setType(QVariant::Int);
    cloudinessState.setDefaultValue(-1);
    weatherStates.append(cloudinessState);

    StateType weatherDescriptionState(StateTypeId("e71d98e3-ebd8-4abf-ad25-9ecc2d05276a"));
    weatherDescriptionState.setName("weather description");
    weatherDescriptionState.setType(QVariant::String);
    weatherDescriptionState.setDefaultValue("");
    weatherStates.append(weatherDescriptionState);

    StateType sunsetState(StateTypeId("5dd6f5a3-25d6-4e60-82ca-e934ad76a4b6"));
    sunsetState.setName("sunset [unixtime]");
    sunsetState.setType(QVariant::UInt);
    sunsetState.setDefaultValue(0);
    weatherStates.append(sunsetState);

    StateType sunriseState(StateTypeId("413b3fc6-bd1c-46fb-8c86-03096254f94f"));
    sunriseState.setName("sunrise [unixtime]");
    sunriseState.setType(QVariant::UInt);
    sunriseState.setDefaultValue(0);
    weatherStates.append(sunriseState);

    //Events


    deviceClassOpenweathermap.setActions(weatherActions);
    deviceClassOpenweathermap.setStates(weatherStates);

    ret.append(deviceClassOpenweathermap);
    return ret;
}

DeviceManager::DeviceError DevicePluginOpenweathermap::discoverDevices(const DeviceClassId &deviceClassId, const QVariantMap &params) const
{
    qDebug() << "should discover divces for" << deviceClassId << params;
//    m_openweaher->
    return DeviceManager::DeviceErrorNoError;
}

DeviceManager::HardwareResources DevicePluginOpenweathermap::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

QString DevicePluginOpenweathermap::pluginName() const
{
    return "Openweathermap";
}

PluginId DevicePluginOpenweathermap::pluginId() const
{
    return PluginId("bc6af567-2338-41d5-aac1-462dec6e4783");
}

void DevicePluginOpenweathermap::guhTimer()
{
    m_openweaher->search("Vie");
}


