#include "weathergroundparser.h"

WeathergroundParser::WeathergroundParser(QObject *parent) :
    QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);

    m_cityCode = "/q/zmw:00000.2.11034";
    m_language = "DL";

    connect(m_manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
    connect(this,SIGNAL(dataReady(QByteArray)),this,SLOT(processResponse(QByteArray)));
    connect(this,SIGNAL(locationDetected()),this,SLOT(getDataFromLocation()));
}

void WeathergroundParser::replyFinished(QNetworkReply *reply)
{
    emit dataReady(reply->readAll());
}

void WeathergroundParser::processResponse(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qDebug() << "failed to parse data" << data << ":" << error.errorString();
    }
    //qDebug() << "-------------------------\n" << jsonDoc.toJson();

    QVariantMap dataMap = jsonDoc.toVariant().toMap();


    //=====================================================================================
    // Pars answere

    //----------------------------------------------------
    if(dataMap.contains("location")){
        QVariantMap locationMap = dataMap.value("location").toMap();
        m_cityName = locationMap.value("city").toString();
        m_cityCode = locationMap.value("l").toString();
        m_country = locationMap.value("country_iso3166").toString();

        qDebug() << m_cityName << m_country << m_cityCode;
        qDebug() << "-------------------";
        emit locationDetected();
    }


    //----------------------------------------------------
    if(dataMap.contains("RESULTS")){
        QVariantList list = dataMap.value("RESULTS").toList();

        QStringList *cityList;

        foreach (QVariant key, list) {
            qDebug() << "----------------------------------------";
            QVariantMap elemant = key.toMap();
            if(elemant.contains("name")){
                qDebug() << elemant.value("name").toString();
                qDebug() << elemant.value("l").toString();
                cityList->append(elemant.value("name").toString());
            }
        }
        emit querryListReady(cityList);
    }
    //----------------------------------------------------
    if(dataMap.contains("sun_phase")){
        qDebug() << jsonDoc.toJson();
        int sunRiseH = dataMap.value("sun_phase").toMap().value("sunrise").toMap().value("hour").toInt();
        int sunRiseM = dataMap.value("sun_phase").toMap().value("sunrise").toMap().value("minute").toInt();

        int sunSetH = dataMap.value("sun_phase").toMap().value("sunset").toMap().value("hour").toInt();
        int sunSetM = dataMap.value("sun_phase").toMap().value("sunset").toMap().value("minute").toInt();

        m_sunrise = QTime(sunRiseH,sunRiseM);
        m_sunset = QTime(sunSetH,sunSetM);

        qDebug() << "sunrise =" << m_sunrise.toString();
        qDebug() << "sunset =" << m_sunset.toString();

        emit sunDataReady(m_sunset, m_sunrise);
    }
    //----------------------------------------------------
    if(dataMap.contains("current_observation")){
        //qDebug() << jsonDoc.toJson();
        m_weather = dataMap.value("current_observation").toMap().value("weather").toString();
        qDebug() << "Currently = " << m_weather;
        m_temperature = dataMap.value("current_observation").toMap().value("temp_c").toDouble();
        qDebug() << "Temperature =" << m_temperature;
        m_temperatureFeeling = dataMap.value("current_observation").toMap().value("feelslike_c").toDouble();
        qDebug() << "Temperature feels like =" << m_temperatureFeeling;
        m_humidity = dataMap.value("current_observation").toMap().value("relative_humidity").toString();
        qDebug() << "Humidity =" << m_humidity ;
        m_windSpeed = dataMap.value("current_observation").toMap().value("wind_kph").toDouble();
        qDebug() << "Wind speed =" << m_windSpeed ;
        m_windDirection = dataMap.value("current_observation").toMap().value("wind_dir").toString();
        qDebug() << "Wind direction =" << m_windDirection;
    }
}

void WeathergroundParser::getDataFromLocation()
{
    QUrl url = "http://api.wunderground.com/api/bc9fbd0a246f151c/conditions/lang:" + m_language + m_cityCode + ".json";
    m_manager->get(QNetworkRequest(url));

    url = "http://api.wunderground.com/api/bc9fbd0a246f151c/astronomy/lang:" + m_language + m_cityCode + ".json";
    m_manager->get(QNetworkRequest(url));
}

void WeathergroundParser::updateData()
{
    qDebug() << "=============================================";
    qDebug() << QTime::currentTime().toString();

    QUrl url = QUrl("http://api.wunderground.com/api/bc9fbd0a246f151c/geolookup/lang:" + m_language + "/q/autoip.json");
    m_manager->get(QNetworkRequest(url));
}

void WeathergroundParser::updateData(QString cityCode, QString language)
{
    QUrl url = "http://api.wunderground.com/api/bc9fbd0a246f151c/conditions/lang:" + m_language + m_cityCode + ".json";
    m_manager->get(QNetworkRequest(url));

    url = "http://api.wunderground.com/api/bc9fbd0a246f151c/astronomy/lang:" + m_language + m_cityCode + ".json";
    m_manager->get(QNetworkRequest(url));
}
