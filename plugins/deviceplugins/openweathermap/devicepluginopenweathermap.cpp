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
#include <QJsonDocument>
#include <QVariantMap>
#include <QDateTime>

VendorId openweathermapVendorId = VendorId("bf1e96f0-9650-4e7c-a56c-916d54d18e7a");
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
    connect(m_openweaher, SIGNAL(searchResultReady(QList<QVariantMap>)), this, SLOT(searchResultsReady(QList<QVariantMap>)));
    connect(m_openweaher, SIGNAL(weatherDataReady(QByteArray)), this, SLOT(weatherDataReady(QByteArray)));
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

    DeviceClass deviceClassOpenweathermap(pluginId(), openweathermapVendorId, openweathermapDeviceClassId);
    deviceClassOpenweathermap.setName("Weather from openweathermap.org");
    deviceClassOpenweathermap.setCreateMethod(DeviceClass::CreateMethodDiscovery);

    // Params
    QVariantList params;
    QVariantMap locationParam;
    locationParam.insert("name", "location");
    locationParam.insert("type", "string");
    params.append(locationParam);

    QVariantMap countryParam;
    countryParam.insert("name", "country");
    countryParam.insert("type", "string");
    params.append(countryParam);

    QVariantMap idParam;
    idParam.insert("name", "id");
    idParam.insert("type", "string");
    params.append(idParam);

    deviceClassOpenweathermap.setParams(params);

    // Actions
    QList<ActionType> weatherActions;
    ActionType updateWeather(updateWeatherActionTypeId);
    updateWeather.setName("refresh");
    weatherActions.append(updateWeather);

    // States
    QList<StateType> weatherStates;
    StateType updateTimeState(updateTimeStateTypeId);
    updateTimeState.setName("last update [unixtime]");
    updateTimeState.setType(QVariant::UInt);
    updateTimeState.setDefaultValue(0);
    weatherStates.append(updateTimeState);

    StateType temperatureState(temperatureStateTypeId);
    temperatureState.setName("temperature [Celsius]");
    temperatureState.setType(QVariant::Double);
    temperatureState.setDefaultValue(-999.9);
    weatherStates.append(temperatureState);

    StateType temperatureMinState(temperatureMinStateTypeId);
    temperatureMinState.setName("temperature minimum [Celsius]");
    temperatureMinState.setType(QVariant::Double);
    temperatureMinState.setDefaultValue(-999.9);
    weatherStates.append(temperatureMinState);

    StateType temperatureMaxState(temperatureMaxStateTypeId);
    temperatureMaxState.setName("temperature maximum [Celsius]");
    temperatureMaxState.setType(QVariant::Double);
    temperatureMaxState.setDefaultValue(999.9);
    weatherStates.append(temperatureMaxState);

    StateType humidityState(humidityStateTypeId);
    humidityState.setName("humidity [%]");
    humidityState.setType(QVariant::Int);
    humidityState.setDefaultValue(-1);
    weatherStates.append(humidityState);

    StateType pressureState(pressureStateTypeId);
    pressureState.setName("pressure [hPa]");
    pressureState.setType(QVariant::Double);
    pressureState.setDefaultValue(-1);
    weatherStates.append(pressureState);

    StateType windSpeedState(windSpeedStateTypeId);
    windSpeedState.setName("wind speed [m/s]");
    windSpeedState.setType(QVariant::Double);
    windSpeedState.setDefaultValue(-1);
    weatherStates.append(windSpeedState);

    StateType windDirectionState(windDirectionStateTypeId);
    windDirectionState.setName("wind direction [degree]");
    windDirectionState.setType(QVariant::Int);
    windDirectionState.setDefaultValue(-1);
    weatherStates.append(windDirectionState);

    StateType cloudinessState(cloudinessStateTypeId);
    cloudinessState.setName("cloudiness [%]");
    cloudinessState.setType(QVariant::Int);
    cloudinessState.setDefaultValue(-1);
    weatherStates.append(cloudinessState);

    StateType weatherDescriptionState(weatherDescriptionStateTypeId);
    weatherDescriptionState.setName("weather description");
    weatherDescriptionState.setType(QVariant::String);
    weatherDescriptionState.setDefaultValue("");
    weatherStates.append(weatherDescriptionState);

    StateType sunsetState(sunsetStateTypeId);
    sunsetState.setName("sunset [unixtime]");
    sunsetState.setType(QVariant::UInt);
    sunsetState.setDefaultValue(0);
    weatherStates.append(sunsetState);

    StateType sunriseState(sunriseStateTypeId);
    sunriseState.setName("sunrise [unixtime]");
    sunriseState.setType(QVariant::UInt);
    sunriseState.setDefaultValue(0);
    weatherStates.append(sunriseState);

    deviceClassOpenweathermap.setActions(weatherActions);
    deviceClassOpenweathermap.setStates(weatherStates);

    ret.append(deviceClassOpenweathermap);
    return ret;
}

DeviceManager::DeviceError DevicePluginOpenweathermap::discoverDevices(const DeviceClassId &deviceClassId, const QVariantMap &params) const
{
    qDebug() << "should discover divces for" << deviceClassId << params.value("location").toString();
    if(params.value("location").toString() == ""){
        m_openweaher->searchAutodetect();
        return DeviceManager::DeviceErrorNoError;
    }
    m_openweaher->search(params.value("location").toString());
    return DeviceManager::DeviceErrorNoError;
}

bool DevicePluginOpenweathermap::deviceCreated(Device *device)
{
    m_openweaher->update(device->params().value("id").toString());
    return true;
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
    foreach (Device *device, deviceManager()->findConfiguredDevices(openweathermapDeviceClassId)) {
        m_openweaher->update(device->params().value("id").toString());
    }
}

void DevicePluginOpenweathermap::searchResultsReady(const QList<QVariantMap> &cityList)
{
    QList<DeviceDescriptor> retList;
    foreach (QVariantMap elemant, cityList) {
        DeviceDescriptor descriptor(openweathermapDeviceClassId, elemant.value("name").toString(),elemant.value("country").toString());
        QVariantMap params;
        params.insert("location", elemant.value("name"));
        params.insert("country", elemant.value("country"));
        params.insert("id", elemant.value("id"));
        descriptor.setParams(params);
        retList.append(descriptor);
    }
    emit devicesDiscovered(openweathermapDeviceClassId,retList);
}

void DevicePluginOpenweathermap::weatherDataReady(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qWarning() << "failed to parse data" << data << ":" << error.errorString();
        return;
    }

    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    qDebug() << "##############################";

    foreach (Device *device, deviceManager()->findConfiguredDevices(openweathermapDeviceClassId)) {
        qDebug() << device->params().value("id").toString();
        qDebug() << dataMap.value("id").toString();
        qDebug() << "##############################";
        if(device->params().value("id").toString() == dataMap.value("id").toString()){

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


