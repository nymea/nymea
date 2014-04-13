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


#include "openweathermap.h"

OpenWeatherMap::OpenWeatherMap(QObject *parent) :
    QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);

    connect(m_manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
}

void OpenWeatherMap::updateLocationData()
{
    QString urlString = "http://ip-api.com/json";
    QNetworkRequest locationRequest;
    locationRequest.setUrl(QUrl(urlString));

    m_locationReply = m_manager->get(locationRequest);
}

void OpenWeatherMap::updateSearchData()
{
    QString urlString = "http://api.openweathermap.org/data/2.5/find?q=" + m_cityName;
    QNetworkRequest searchRequest;
    searchRequest.setUrl(QUrl(urlString));

    m_searchReply = m_manager->get(searchRequest);
}

void OpenWeatherMap::updateWeatherData()
{
    QString urlString = "http://api.openweathermap.org/data/2.5/weather?id="+ m_cityId + "&mode=json&units=metric";
    QNetworkRequest weatherRequest;
    weatherRequest.setUrl(QUrl(urlString));

    m_weatherReply = m_manager->get(weatherRequest);
}

void OpenWeatherMap::processLocationResponse(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qWarning() << "failed to parse data" << data << ":" << error.errorString();
    }
    //qDebug() << jsonDoc.toJson();

    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    if(dataMap.contains("city")){
        QString cityName = dataMap.value("city").toString();
        // skip search for cityID...
        if(m_cityName != cityName){
            m_cityName = cityName;
            updateSearchData();
            return;
        }else{
            updateWeatherData();
            return;
        }
    }
}

void OpenWeatherMap::processSearchResponse(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qWarning() << "failed to parse data" << data << ":" << error.errorString();
    }
    //qDebug() << jsonDoc.toJson();

    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    if(dataMap.contains("list")){
        QVariantList list = dataMap.value("list").toList();
        foreach (QVariant key, list) {
            QVariantMap elemant = key.toMap();
            if(elemant.contains("id")){
                m_cityId = elemant.value("id").toString();
                updateWeatherData();
                return;
            }
        }
    }
}

void OpenWeatherMap::processWeatherResponse(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qWarning() << "failed to parse data" << data << ":" << error.errorString();
    }
    //qDebug() << jsonDoc.toJson();

    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    if(dataMap.contains("clouds")){
        int cloudiness = dataMap.value("clouds").toMap().value("all").toInt();
        if(m_cloudiness != cloudiness){
            m_cloudiness = cloudiness;
            //emit cloudiness changed
        }
    }
    if(dataMap.contains("dt")){
        uint lastUpdate = dataMap.value("dt").toUInt();
        if(m_lastUpdate != lastUpdate){
            m_lastUpdate = lastUpdate;
            // emit
        }
    }

    if(dataMap.contains("main")){
        double temperatur = dataMap.value("main").toMap().value("temp").toDouble();
        if(m_temperatur != temperatur){
            m_temperatur = temperatur;
            // emit
        }

        double temperaturMax = dataMap.value("main").toMap().value("temp_max").toDouble();
        if(m_temperaturMax != temperaturMax){
            m_temperaturMax = temperaturMax;
            // emit
        }

        double temperaturMin = dataMap.value("main").toMap().value("temp_min").toDouble();
        if(m_temperaturMin != temperaturMin){
            m_temperaturMin = temperaturMin;
            // emit
        }

        double pressure = dataMap.value("main").toMap().value("pressure").toDouble();
        if(m_pressure != pressure){
            m_pressure = pressure;
            //emit
        }

        int humidity = dataMap.value("main").toMap().value("humidity").toInt();
        if(m_humidity != humidity){
            m_humidity = humidity;
            //emit
        }
    }

    if(dataMap.contains("sys")){
        uint sunrise = dataMap.value("sys").toMap().value("sunrise").toUInt();
        if(m_sunrise != sunrise){
            m_sunrise = sunrise;
            // emit
        }

        uint sunset = dataMap.value("sys").toMap().value("sunset").toUInt();
        if(m_sunset != sunset){
            m_sunset = sunset;
            // emit
        }
    }

    if(dataMap.contains("weather")){
        QString description = dataMap.value("weather").toMap().value("description").toString();
        if(m_weatherDescription != description){
            m_weatherDescription = description;
            // emit
        }
    }

    if(dataMap.contains("wind")){
        int windDirection = dataMap.value("wind").toMap().value("deg").toInt();
        if(m_windDirection != windDirection){
            m_windDirection = windDirection;
            //emit
        }

        double windSpeed = dataMap.value("wind").toMap().value("speed").toDouble();
        if(m_windSpeed != windSpeed){
            m_windSpeed = windSpeed;
            // emit
        }
    }
    //    qDebug() << "#########################################################";
    //    qDebug() << m_cityName << m_cityId;
    //    qDebug() << "#########################################################";
    //    qDebug() << "temp" << m_temperatur;
    //    qDebug() << "temp min" << m_temperaturMin;
    //    qDebug() << "temp max" << m_temperaturMax;
    //    qDebug() << "cloudiness" << m_cloudiness;
    //    qDebug() << "humidity" << m_humidity;
    //    qDebug() << "pressure" << m_pressure;
    //    qDebug() << "wind dir" << m_windDirection;
    //    qDebug() << "wind speed" << m_windSpeed;
    //    qDebug() << "sunrise" << QDateTime::fromTime_t(m_sunrise);
    //    qDebug() << "sunset" << QDateTime::fromTime_t(m_sunset);
    //    qDebug() << "last update" << QDateTime::fromTime_t(m_lastUpdate);
}

void OpenWeatherMap::update()
{
    updateLocationData();
}

void OpenWeatherMap::replyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data;

    if(reply == m_locationReply && status == 200){
        data = reply->readAll();
        processLocationResponse(data);
        return;
    }

    if(reply == m_searchReply && status == 200){
        data = reply->readAll();
        processSearchResponse(data);
        return;
    }

    if(reply == m_weatherReply && status == 200){
        data = reply->readAll();
        processWeatherResponse(data);
        return;
    }
}
