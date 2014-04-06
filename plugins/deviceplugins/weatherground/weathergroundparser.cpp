#include "weathergroundparser.h"

WeathergroundParser::WeathergroundParser(QObject *parent) :
    QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);

    m_cityCode = "/q/zmw:00000.2.11034";
    m_language = "EN";
    m_apikey = "779a480dea5163c6";

    connect(m_manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
    connect(this,SIGNAL(dataReady(QByteArray)),this,SLOT(processResponse(QByteArray)));
    connect(this,SIGNAL(locationDetected(QString,QString)),this,SLOT(updateData(QString,QString)));
}

void WeathergroundParser::replyFinished(QNetworkReply *reply)
{
    emit dataReady(reply->readAll());
}

void WeathergroundParser::processResponse(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    qDebug() << jsonDoc.toJson();

    if(error.error != QJsonParseError::NoError) {
        qWarning() << "failed to parse data" << data << ":" << error.errorString();
    }

    QVariantMap dataMap = jsonDoc.toVariant().toMap();

    //=====================================================================================
    // Pars answere

    //----------------------------------------------------
    if(dataMap.contains("location")){
        QVariantMap locationMap = dataMap.value("location").toMap();
        m_cityName = locationMap.value("city").toString();
        m_cityCode = locationMap.value("l").toString();
        m_country = locationMap.value("country_iso3166").toString();

        emit locationDetected(m_cityCode,m_language);
    }


    //----------------------------------------------------
    if(dataMap.contains("RESULTS")){
        QVariantList list = dataMap.value("RESULTS").toList();
//        foreach (QVariant key, list) {
//            qDebug() << "----------------------------------------";
//            QVariantMap elemant = key.toMap();
//            if(elemant.contains("name")){
//                qDebug() << elemant.value("name").toString();
//                qDebug() << elemant.value("l").toString();
//            }
//        }
        emit querryListReady(list);
    }
    //----------------------------------------------------
    if(dataMap.contains("sun_phase")){
        int sunRiseH = dataMap.value("sun_phase").toMap().value("sunrise").toMap().value("hour").toInt();
        int sunRiseM = dataMap.value("sun_phase").toMap().value("sunrise").toMap().value("minute").toInt();

        int sunSetH = dataMap.value("sun_phase").toMap().value("sunset").toMap().value("hour").toInt();
        int sunSetM = dataMap.value("sun_phase").toMap().value("sunset").toMap().value("minute").toInt();

        m_sunrise = QTime(sunRiseH,sunRiseM);
        m_sunset = QTime(sunSetH,sunSetM);

        emit sunDataReady(m_sunset, m_sunrise);
    }
    //----------------------------------------------------
    if(dataMap.contains("current_observation")){
        m_weather = dataMap.value("current_observation").toMap().value("weather").toString();
        m_temperature = dataMap.value("current_observation").toMap().value("temp_c").toDouble();
        m_temperatureFeeling = dataMap.value("current_observation").toMap().value("feelslike_c").toDouble();
        m_humidity = dataMap.value("current_observation").toMap().value("relative_humidity").toString();
        m_windSpeed = dataMap.value("current_observation").toMap().value("wind_kph").toDouble();
        m_windDirection = dataMap.value("current_observation").toMap().value("wind_dir").toString();
    }
}

void WeathergroundParser::updateData()
{
//    qDebug() << "=============================================";
//    qDebug() << QTime::currentTime().toString();

    QUrl url = QUrl("http://api.wunderground.com/api/" + m_apikey +"/geolookup/lang:" + m_language + "/q/autoip.json");
    m_manager->get(QNetworkRequest(url));
}

void WeathergroundParser::updateData(const QString &cityCode, const QString &language)
{
    QUrl url = "http://api.wunderground.com/api/" + m_apikey +"/conditions/lang:" + language + cityCode + ".json";
    m_manager->get(QNetworkRequest(url));

    url = "http://api.wunderground.com/api/" + m_apikey +"/astronomy/lang:" + language + cityCode + ".json";
    m_manager->get(QNetworkRequest(url));
}

void WeathergroundParser::searchCity(const QString &searchString)
{
    QUrl url = QUrl("http://autocomplete.wunderground.com/aq?query=" + searchString);
    m_manager->get(QNetworkRequest(url));
}
