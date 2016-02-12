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
    \page openweathermap.html
    \title Open Weather Map

    \ingroup plugins
    \ingroup services

    This plugin allows to get the current weather data from \l{http://www.openweathermap.org}{OpenWeatherMap}.
    The weather data will be refreshed every 15 minutes automatically, but can also refreshed manually.
    The plugin offers two different search methods for the location: if the user searches for a empty string,
    the plugin makes an autodetction with the WAN ip and offers the user the found weather stations.
    The autodetection function uses the geolocation of your WAN ip and searches all available weather
    stations in a radius of 1.5 km. Otherwise the plugin returns the list of the found search results
    from the search string.

    \underline{NOTE}: If you are using a VPN connection, the autodetection will show the results around your VPN location.

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

    \quotefile plugins/deviceplugins/openweathermap/devicepluginopenweathermap.json
*/

#include "devicepluginopenweathermap.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QJsonDocument>
#include <QVariantMap>
#include <QUrl>
#include <QUrlQuery>
#include <QDateTime>

DevicePluginOpenweathermap::DevicePluginOpenweathermap()
{
    // max 60 calls/minute
    // max 50000 calls/day
    m_apiKey = "c1b9d5584bb740804871583f6c62744f";

    // update every 15 minutes
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->setInterval(900000);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

DeviceManager::DeviceError DevicePluginOpenweathermap::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    if (deviceClassId != openweathermapDeviceClassId) {
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
        searchAutodetect();
    } else {
        search(location);
    }
    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::DeviceSetupStatus DevicePluginOpenweathermap::setupDevice(Device *device)
{
    if (device->deviceClassId() != openweathermapDeviceClassId)
        return DeviceManager::DeviceSetupStatusFailure;

    if (!m_timer->isActive())
        m_timer->start();

    update(device);

    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::HardwareResources DevicePluginOpenweathermap::requiredHardware() const
{
    return DeviceManager::HardwareResourceNetworkManager;
}

DeviceManager::DeviceError DevicePluginOpenweathermap::executeAction(Device *device, const Action &action)
{
    if(action.actionTypeId() == refreshWeatherActionTypeId){
        update(device);
        return DeviceManager::DeviceErrorNoError;
    }
    return DeviceManager::DeviceErrorActionTypeNotFound;
}

void DevicePluginOpenweathermap::deviceRemoved(Device *device)
{
    // check if a device gets removed while we still have a reply!
    foreach (Device *d, m_weatherReplies.values()) {
        if (d->id() == device->id()) {
            QNetworkReply *reply = m_weatherReplies.key(device);
            m_weatherReplies.take(reply);
            reply->deleteLater();
        }
    }

    if (myDevices().isEmpty())
        m_timer->stop();
}

void DevicePluginOpenweathermap::networkManagerReplyReady(QNetworkReply *reply)
{
    if (reply->error()) {
        qCWarning(dcOpenWeatherMap) << "OpenWeatherMap reply error: " << reply->errorString();
        reply->deleteLater();
        return;
    }

    if (m_autodetectionReplies.contains(reply)) {
        QByteArray data = reply->readAll();
        m_autodetectionReplies.removeOne(reply);
        processAutodetectResponse(data);
    } else if (m_searchReplies.contains(reply)) {
        QByteArray data = reply->readAll();
        m_searchReplies.removeOne(reply);
        processSearchResponse(data);
    } else if (m_searchGeoReplies.contains(reply)) {
        QByteArray data = reply->readAll();
        m_searchGeoReplies.removeOne(reply);
        processGeoSearchResponse(data);
    } else if (m_weatherReplies.contains(reply)) {
        QByteArray data = reply->readAll();
        Device* device = m_weatherReplies.take(reply);
        processWeatherData(data, device);
    }
    reply->deleteLater();
}

void DevicePluginOpenweathermap::update(Device *device)
{
    QUrl url("http://api.openweathermap.org/data/2.5/weather");
    QUrlQuery query;
    query.addQueryItem("id", device->paramValue("id").toString());
    query.addQueryItem("mode", "json");
    query.addQueryItem("units", "metric");
    query.addQueryItem("appid", m_apiKey);
    url.setQuery(query);

    QNetworkReply *reply = networkManagerGet(QNetworkRequest(url));
    m_weatherReplies.insert(reply, device);
}

void DevicePluginOpenweathermap::searchAutodetect()
{
    QNetworkReply *reply = networkManagerGet(QNetworkRequest(QUrl("http://ip-api.com/json")));
    m_autodetectionReplies.append(reply);
}

void DevicePluginOpenweathermap::search(QString searchString)
{
    QUrl url("http://api.openweathermap.org/data/2.5/find");
    QUrlQuery query;
    query.addQueryItem("q", searchString);
    query.addQueryItem("type", "like");
    query.addQueryItem("mode", "json");
    query.addQueryItem("units", "metric");
    query.addQueryItem("appid", m_apiKey);
    url.setQuery(query);

    QNetworkReply *reply = networkManagerGet(QNetworkRequest(url));
    m_searchReplies.append(reply);
}

void DevicePluginOpenweathermap::searchGeoLocation(double lat, double lon)
{
    QUrl url("http://api.openweathermap.org/data/2.5/find");
    QUrlQuery query;
    query.addQueryItem("lat", QString::number(lat));
    query.addQueryItem("lon", QString::number(lon));
    query.addQueryItem("cnt", QString::number(3)); // 3 km radius
    query.addQueryItem("type", "like");
    query.addQueryItem("mode", "json");
    query.addQueryItem("units", "metric");
    query.addQueryItem("appid", m_apiKey);
    url.setQuery(query);

    QNetworkReply *reply = networkManagerGet(QNetworkRequest(url));
    m_searchGeoReplies.append(reply);
}

void DevicePluginOpenweathermap::processAutodetectResponse(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcOpenWeatherMap) << "failed to parse data" << data << ":" << error.errorString();
        emit devicesDiscovered(openweathermapDeviceClassId, QList<DeviceDescriptor>());
        return;
    }

    // search by geographic coordinates
    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    if (dataMap.contains("countryCode")) {
        m_country = dataMap.value("countryCode").toString();
    }
    if (dataMap.contains("city")) {
        m_cityName = dataMap.value("city").toString();
    }
    if (dataMap.contains("query")) {
        m_wanIp = QHostAddress(dataMap.value("query").toString());
    }
    if (dataMap.contains("lon") && dataMap.contains("lat")) {
        m_longitude = dataMap.value("lon").toDouble();
        m_latitude = dataMap.value("lat").toDouble();
        qCDebug(dcOpenWeatherMap) << "----------------------------------------";
        qCDebug(dcOpenWeatherMap) << "Autodetection of location: ";
        qCDebug(dcOpenWeatherMap) << "----------------------------------------";
        qCDebug(dcOpenWeatherMap) << "       name:" << m_cityName;
        qCDebug(dcOpenWeatherMap) << "    country:" << m_country;
        qCDebug(dcOpenWeatherMap) << "     WAN IP:" << m_wanIp.toString();
        qCDebug(dcOpenWeatherMap) << "   latitude:" << m_latitude;
        qCDebug(dcOpenWeatherMap) << "  longitude:" << m_longitude;
        qCDebug(dcOpenWeatherMap) << "----------------------------------------";
        searchGeoLocation(m_latitude, m_longitude);
    }
}

void DevicePluginOpenweathermap::processSearchResponse(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcOpenWeatherMap) << "failed to parse data" << data << ":" << error.errorString();
        emit devicesDiscovered(openweathermapDeviceClassId, QList<DeviceDescriptor>());
        return;
    }

    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    QList<QVariantMap> cityList;
    if (dataMap.contains("list")) {
        QVariantList list = dataMap.value("list").toList();
        foreach (QVariant key, list) {
            QVariantMap elemant = key.toMap();
            QVariantMap city;
            city.insert("name",elemant.value("name").toString());
            city.insert("country", elemant.value("sys").toMap().value("country").toString());
            city.insert("id",elemant.value("id").toString());
            cityList.append(city);
        }
    }
    processSearchResults(cityList);
}

void DevicePluginOpenweathermap::processGeoSearchResponse(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcOpenWeatherMap) << "failed to parse data" << data << ":" << error.errorString();
        emit devicesDiscovered(openweathermapDeviceClassId, QList<DeviceDescriptor>());
        return;
    }

    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    QList<QVariantMap> cityList;
    if (dataMap.contains("list")) {
        QVariantList list = dataMap.value("list").toList();
        foreach (QVariant key, list) {
            QVariantMap elemant = key.toMap();
            QVariantMap city;
            city.insert("name",elemant.value("name").toString());
            if(elemant.value("sys").toMap().value("country").toString().isEmpty()){
                city.insert("country",m_country);
            }else{
                city.insert("country", elemant.value("sys").toMap().value("country").toString());
            }
            city.insert("id",elemant.value("id").toString());
            cityList.append(city);
        }
    }
    processSearchResults(cityList);
}

void DevicePluginOpenweathermap::processSearchResults(const QList<QVariantMap> &cityList)
{
    QList<DeviceDescriptor> retList;
    foreach (QVariantMap elemant, cityList) {
        DeviceDescriptor descriptor(openweathermapDeviceClassId, elemant.value("name").toString(), elemant.value("country").toString());
        ParamList params;
        Param nameParam("name", elemant.value("name"));
        params.append(nameParam);
        Param countryParam("country", elemant.value("country"));
        params.append(countryParam);
        Param idParam("id", elemant.value("id"));
        params.append(idParam);
        descriptor.setParams(params);
        retList.append(descriptor);
    }
    emit devicesDiscovered(openweathermapDeviceClassId, retList);
}

void DevicePluginOpenweathermap::processWeatherData(const QByteArray &data, Device *device)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    //qCDebug(dcOpenWeatherMap) << jsonDoc.toJson();

    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcOpenWeatherMap) << "failed to parse weather data for device " << device->name() << ": " << data << ":" << error.errorString();
        return;
    }

    // http://openweathermap.org/current
    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    if (dataMap.contains("clouds")) {
        int cloudiness = dataMap.value("clouds").toMap().value("all").toInt();
        device->setStateValue(cloudinessStateTypeId, cloudiness);
    }
    if (dataMap.contains("dt")) {
        uint lastUpdate = dataMap.value("dt").toUInt();
        device->setStateValue(updateTimeStateTypeId, lastUpdate);
    }

    if (dataMap.contains("main")) {
        double temperatur = dataMap.value("main").toMap().value("temp").toDouble();
        double temperaturMax = dataMap.value("main").toMap().value("temp_max").toDouble();
        double temperaturMin = dataMap.value("main").toMap().value("temp_min").toDouble();
        double pressure = dataMap.value("main").toMap().value("pressure").toDouble();
        int humidity = dataMap.value("main").toMap().value("humidity").toInt();

        device->setStateValue(temperatureStateTypeId, temperatur);
        device->setStateValue(temperatureMinStateTypeId, temperaturMin);
        device->setStateValue(temperatureMaxStateTypeId, temperaturMax);
        device->setStateValue(pressureStateTypeId, pressure);
        device->setStateValue(humidityStateTypeId, humidity);
    }

    if (dataMap.contains("sys")) {
        uint sunrise = dataMap.value("sys").toMap().value("sunrise").toUInt();
        uint sunset = dataMap.value("sys").toMap().value("sunset").toUInt();

        device->setStateValue(sunriseStateTypeId, sunrise);
        device->setStateValue(sunsetStateTypeId, sunset);
    }

    if (dataMap.contains("visibility")) {
        int visibility = dataMap.value("visibility").toInt();
        device->setStateValue(visibilityStateTypeId, visibility);
    }

    // http://openweathermap.org/weather-conditions
    if (dataMap.contains("weather")) {
        QString description = dataMap.value("weather").toList().first().toMap().value("description").toString();
        device->setStateValue(weatherDescriptionStateTypeId, description);
    }

    if (dataMap.contains("wind")) {
        int windDirection = dataMap.value("wind").toMap().value("deg").toInt();
        double windSpeed = dataMap.value("wind").toMap().value("speed").toDouble();

        device->setStateValue(windDirectionStateTypeId, windDirection);
        device->setStateValue(windSpeedStateTypeId, windSpeed);
    }
}

void DevicePluginOpenweathermap::onTimeout()
{
    foreach (Device *device, myDevices()) {
        update(device);
    }
}

