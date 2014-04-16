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
        emit weatherDataReady(data);
        m_weatherReply->deleteLater();
        return;
    }
}
