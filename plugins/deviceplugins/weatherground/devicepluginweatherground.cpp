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

#include "devicepluginweatherground.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include <QDebug>
#include <QStringList>
#include <QJsonDocument>
#include <QDateTime>

// Key: 779a480dea5163c6

VendorId weathergroundVendorId = VendorId("68f84197-b158-4d24-9d7b-709cfff843c1");

DevicePluginWeatherground::DevicePluginWeatherground()
{
    m_parser = new WeathergroundParser(this);
}

QList<Vendor> DevicePluginWeatherground::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor weatherground(weathergroundVendorId, "weatherground");
    ret.append(weatherground);
    return ret;
}

QList<DeviceClass> DevicePluginWeatherground::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassWeatherground(pluginId(), weathergroundVendorId, DeviceClassId("af2e15f0-650e-4452-b379-fa76a2dc46c6"));
    deviceClassWeatherground.setName("Weather");

    QVariantList weatherParams;
    QVariantMap cityParam;
    cityParam.insert("name", "citycode");
    cityParam.insert("type", "string");
    weatherParams.append(cityParam);

    QVariantMap autoDetectParam;
    autoDetectParam.insert("name", "autodetect");
    autoDetectParam.insert("type", "bool");
    weatherParams.append(cityParam);


    QList<ActionType> weatherActions;
    ActionType updateWeather(ActionTypeId("a1dc271a-a993-4d9b-adfc-4fbb58cbecb9"));
    updateWeather.setName("refresh");
    weatherActions.append(updateWeather);


    QList<StateType> weatherStates;
    StateType updateTimeState(StateTypeId("09b091f5-d830-4739-b8f0-df96101cabc6"));
    updateTimeState.setName("last update");
    updateTimeState.setType(QVariant::DateTime);
    updateTimeState.setDefaultValue(QDateTime(QDate(1999,1,1),QTime(0,0,0)));
    weatherStates.append(updateTimeState);

    StateType temperatureState(StateTypeId("97ffaa0b-4302-43a5-9aa5-00a5efe321c0"));
    temperatureState.setName("temperature [°C]");
    temperatureState.setType(QVariant::Double);
    temperatureState.setDefaultValue(-999.9);
    weatherStates.append(temperatureState);

    StateType humidityState(StateTypeId("2e925181-69b7-4201-9160-11ca4afecc41"));
    humidityState.setName("humidity [%]");
    humidityState.setType(QVariant::Int);
    humidityState.setDefaultValue(0);
    weatherStates.append(humidityState);

    StateType sunsetState(StateTypeId("a8e6601e-c9de-43c8-9a0e-9688cb66ae6d"));
    sunsetState.setName("sunset");
    sunsetState.setType(QVariant::Time);
    sunsetState.setDefaultValue(QTime(0,0,0));
    weatherStates.append(sunsetState);

    StateType sunriseState(StateTypeId("8e81d15a-3231-415b-8fba-5f6a02259cc1"));
    sunriseState.setName("sunrise");
    sunriseState.setType(QVariant::Time);
    sunriseState.setDefaultValue(QTime(0,0,0));
    weatherStates.append(sunriseState);

    StateType windSpeedState(StateTypeId("546880b9-c4c8-4dc1-b589-ac9c76240009"));
    windSpeedState.setName("wind speed [km/h]");
    windSpeedState.setType(QVariant::Double);
    windSpeedState.setDefaultValue(0);
    weatherStates.append(windSpeedState);

    StateType windDirectionState(StateTypeId("e05e6015-4ed8-4fb1-a18e-1a09be272556"));
    windDirectionState.setName("wind direction");
    windDirectionState.setType(QVariant::String);
    windDirectionState.setDefaultValue("-");
    weatherStates.append(windDirectionState);

    StateType currentlyState(StateTypeId("6032cb3b-fe52-4006-aa8b-d5c1e6d500e3"));
    currentlyState.setName("current weather");
    currentlyState.setType(QVariant::String);
    currentlyState.setDefaultValue("-");
    weatherStates.append(currentlyState);

    StateType ageOfMoonState(StateTypeId("e49fe057-98ac-4a38-9aa3-8e8ff260c162"));
    ageOfMoonState.setName("age of moon [days]");
    ageOfMoonState.setType(QVariant::Int);
    ageOfMoonState.setDefaultValue(-1);
    weatherStates.append(ageOfMoonState);

    StateType moonIlluminatedState(StateTypeId("d8688eb5-5d4f-4c85-9338-d86c7c2069a8"));
    moonIlluminatedState.setName("moon illuminated [%]");
    moonIlluminatedState.setType(QVariant::Int);
    moonIlluminatedState.setDefaultValue(-1);
    weatherStates.append(moonIlluminatedState);

    deviceClassWeatherground.setActions(weatherActions);
    deviceClassWeatherground.setStates(weatherStates);

    ret.append(deviceClassWeatherground);
    return ret;
}

DeviceManager::HardwareResources DevicePluginWeatherground::requiredHardware() const
{
    return DeviceManager::HardwareResourceTimer;
}

QString DevicePluginWeatherground::pluginName() const
{
    return "Weatherground";
}

QUuid DevicePluginWeatherground::pluginId() const
{
    return QUuid("16ed6d4d-2ab4-4fed-bf54-c95107b9982a");
}

void DevicePluginWeatherground::guhTimer()
{
    qDebug() << "update Weatherground states...";
}

void DevicePluginWeatherground::setState(const QUuid &stateTypeId, const QVariant &value)
{

}

void DevicePluginWeatherground::executeAction(Device *device, const Action &action)
{

}

