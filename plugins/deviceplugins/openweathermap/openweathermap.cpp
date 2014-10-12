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

#include "openweathermap.h"
#include <QDebug>
#include <QDateTime>

OpenWeatherMap::OpenWeatherMap(QObject *parent) :
    QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &OpenWeatherMap::replyFinished);
}

void OpenWeatherMap::update(QString id, DeviceId deviceId)
{
    m_cityId = id;
    QString urlString = "http://api.openweathermap.org/data/2.5/weather?id="+ m_cityId + "&mode=json&units=metric";
    QNetworkRequest weatherRequest;
    weatherRequest.setUrl(QUrl(urlString));

    QNetworkReply *weatherReply = m_manager->get(weatherRequest);
    m_weatherReplys.insert(weatherReply, deviceId);
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

void OpenWeatherMap::searchGeoLocation(double lat, double lon)
{
    QString urlString = "http://api.openweathermap.org/data/2.5/find?lat=" + QString::number(lat) + "&lon=" + QString::number(lon) + "cnt=5&type=like&units=metric&mode=json";
    QNetworkRequest searchRequest;
    searchRequest.setUrl(QUrl(urlString));
    qDebug() << "search URL " << urlString;

    m_searchGeoReply = m_manager->get(searchRequest);
}

void OpenWeatherMap::processLocationResponse(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qWarning() << "failed to parse data" << data << ":" << error.errorString();
    }
    // qDebug() << jsonDoc.toJson();

    // search by geographic coordinates
    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    if(dataMap.contains("countryCode")){
        m_country = dataMap.value("countryCode").toString();
    }
    if(dataMap.contains("city")){
        m_cityName = dataMap.value("city").toString();
    }
    if(dataMap.contains("query")){
        m_wanIp = QHostAddress(dataMap.value("query").toString());
    }
    if(dataMap.contains("lon") && dataMap.contains("lat")){
        qDebug() << "Autodetection of location: " << m_cityName << "(" << m_country << ")" << m_wanIp;
        searchGeoLocation(dataMap.value("lat").toDouble(),dataMap.value("lon").toDouble());
    }
}

void OpenWeatherMap::processSearchResponse(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qWarning() << "failed to parse data" << data << ":" << error.errorString();
    }

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

void OpenWeatherMap::processSearchGeoResponse(QByteArray data)
{
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
            if(elemant.value("sys").toMap().value("country").toString().isEmpty()){
                qDebug() << m_country;
            }else{
                qDebug() << elemant.value("sys").toMap().value("country").toString();
            }
            qDebug() << elemant.value("id").toString();
            qDebug() << "--------------------------------------";

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
    qDebug() << "----------------------------------------";
    emit searchResultReady(cityList);
}

void OpenWeatherMap::replyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data;

    if(reply->error() == QNetworkReply::NoError){
        if(reply == m_locationReply){
            data = reply->readAll();
            processLocationResponse(data);
            delete m_locationReply;
            return;
        }

        if(reply == m_searchReply){
            data = reply->readAll();
            processSearchResponse(data);
            delete m_searchReply;
            return;
        }

        if(reply == m_searchGeoReply){
            data = reply->readAll();
            processSearchGeoResponse(data);
            delete m_searchGeoReply;
            return;
        }

        if(m_weatherReplys.contains(reply)){
            data = reply->readAll();
            DeviceId deviceId = m_weatherReplys.value(reply);
            emit weatherDataReady(data, deviceId);
            m_weatherReplys.take(reply);
            delete reply;
        }
    }else{
        qWarning() << "ERROR: OpenWeatherMap reply error code: " << status << reply->errorString();
        delete reply;
    }
}
