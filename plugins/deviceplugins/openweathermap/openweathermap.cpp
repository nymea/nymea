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

void OpenWeatherMap::update(QString id)
{
    m_cityId = id;
    QString urlString = "http://api.openweathermap.org/data/2.5/weather?id="+ m_cityId + "&mode=json&units=metric";
    QNetworkRequest weatherRequest;
    weatherRequest.setUrl(QUrl(urlString));

    m_weatherReply = m_manager->get(weatherRequest);
}

void OpenWeatherMap::searchAutodetect()
{
    QString urlString = "http://ip-api.com/json";
    QNetworkRequest locationRequest;
    locationRequest.setUrl(QUrl(urlString));

    m_locationReply = m_manager->get(locationRequest);
}

void OpenWeatherMap::search(QString searchString)
{
    QString urlString = "http://api.openweathermap.org/data/2.5/find?q=" + searchString + "&type=like&units=metric&mode=json";
    QNetworkRequest searchRequest;
    searchRequest.setUrl(QUrl(urlString));

    m_searchReply = m_manager->get(searchRequest);
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
        search(dataMap.value("city").toString());
    }
}

void OpenWeatherMap::processSearchResponse(QByteArray data)
{
    emit weatherDataReady(data);

    // TODO: return here...remove the rest from here...

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qWarning() << "failed to parse data" << data << ":" << error.errorString();
    }
    //qDebug() << jsonDoc.toJson();

    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    qDebug() << "----------------------------------------";
    qDebug() << "openweathermap search results";
    qDebug() << "----------------------------------------";
    QList<QVariantMap> cityList;
    if(dataMap.contains("list")){
        QVariantList list = dataMap.value("list").toList();
        foreach (QVariant key, list) {
            QVariantMap elemant = key.toMap();
            qDebug() << elemant.value("name").toString();
            qDebug() << elemant.value("sys").toMap().value("country").toString();
            qDebug() << elemant.value("id").toString();
            qDebug() << "--------------------------------------";

            QVariantMap city;
            city.insert("name",elemant.value("name").toString());
            city.insert("country", elemant.value("sys").toMap().value("country").toString());
            city.insert("id",elemant.value("id").toString());
            cityList.append(city);
        }
    }
    qDebug() << "----------------------------------------";
    emit searchResultReady(cityList);
}

void OpenWeatherMap::processSearchLocationResponse(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qWarning() << "failed to parse data" << data << ":" << error.errorString();
    }
    //qDebug() << jsonDoc.toJson();

    QList<QVariantMap> cityList;
    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    if(dataMap.contains("list")){
        QVariantList list = dataMap.value("list").toList();
        foreach (QVariant key, list) {
            QVariantMap elemant = key.toMap();

            QVariantMap city;
            city.insert("name",elemant.value("name").toString());
            city.insert("country", elemant.value("sys").toMap().value("country").toString());
            city.insert("id",elemant.value("id").toString());
            cityList.append(city);

            m_cityId = elemant.value("id").toString();
            search(m_cityName);
            return;
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
        m_cloudiness = cloudiness;
    }
    if(dataMap.contains("dt")){
        uint lastUpdate = dataMap.value("dt").toUInt();
        m_lastUpdate = lastUpdate;
    }
    if(dataMap.contains("name")){
        QString description = dataMap.value("name").toString();
        m_cityName = description;
    }

    if(dataMap.contains("sys")){
        QString description = dataMap.value("sys").toMap().value("country").toString();
        m_country = description;
    }

    if(dataMap.contains("main")){
        double temperatur = dataMap.value("main").toMap().value("temp").toDouble();
        m_temperatur = temperatur;

        double temperaturMax = dataMap.value("main").toMap().value("temp_max").toDouble();
        m_temperaturMax = temperaturMax;
    }
    double temperaturMin = dataMap.value("main").toMap().value("temp_min").toDouble();
    m_temperaturMin = temperaturMin;

    double pressure = dataMap.value("main").toMap().value("pressure").toDouble();
    m_pressure = pressure;

    int humidity = dataMap.value("main").toMap().value("humidity").toInt();
    m_humidity = humidity;

    if(dataMap.contains("sys")){
        uint sunrise = dataMap.value("sys").toMap().value("sunrise").toUInt();
        m_sunrise = sunrise;

        uint sunset = dataMap.value("sys").toMap().value("sunset").toUInt();
        m_sunset = sunset;
    }

    if(dataMap.contains("weather")){
        QString description = dataMap.value("weather").toMap().value("description").toString();
        m_weatherDescription = description;
    }

    if(dataMap.contains("wind")){
        int windDirection = dataMap.value("wind").toMap().value("deg").toInt();
        m_windDirection = windDirection;

        double windSpeed = dataMap.value("wind").toMap().value("speed").toDouble();
        m_windSpeed = windSpeed;
    }

    qDebug() << "#########################################################";
    qDebug() << m_cityName << m_country << m_cityId;
    qDebug() << "#########################################################";
    qDebug() << "temp" << m_temperatur;
    qDebug() << "temp min" << m_temperaturMin;
    qDebug() << "temp max" << m_temperaturMax;
    qDebug() << "cloudiness" << m_cloudiness;
    qDebug() << "humidity" << m_humidity;
    qDebug() << "pressure" << m_pressure;
    qDebug() << "wind dir" << m_windDirection;
    qDebug() << "wind speed" << m_windSpeed;
    qDebug() << "sunrise" << QDateTime::fromTime_t(m_sunrise);
    qDebug() << "sunset" << QDateTime::fromTime_t(m_sunset);
    qDebug() << "last update" << QDateTime::fromTime_t(m_lastUpdate);
}

void OpenWeatherMap::replyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data;

    if(reply == m_locationReply && status == 200){
        data = reply->readAll();
        processLocationResponse(data);
        m_locationReply->deleteLater();
        return;
    }

    if(reply == m_searchReply && status == 200){
        data = reply->readAll();
        processSearchResponse(data);
        m_searchReply->deleteLater();
        return;
    }

    if(reply == m_searchLocationReply && status == 200){
        data = reply->readAll();
        processSearchLocationResponse(data);
        m_searchLocationReply->deleteLater();
        return;
    }

    if(reply == m_weatherReply && status == 200){
        data = reply->readAll();
        processWeatherResponse(data);
        m_weatherReply->deleteLater();
        return;
    }
}
