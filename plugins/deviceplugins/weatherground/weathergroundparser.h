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

#ifndef WEATHERGROUNDPARSER_H
#define WEATHERGROUNDPARSER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QStringList>
#include <QJsonDocument>

class WeathergroundParser : public QObject
{
    Q_OBJECT
public:
    explicit WeathergroundParser(QObject *parent = 0);

private:
    QNetworkAccessManager *m_manager;

    QString m_cityCode;
    QString m_cityName;
    QString m_country;
    QString m_language;
    QString m_apikey;

    QTime m_currentTime;

    //current weather
    QString m_weather;
    QString m_windDirection;
    QString m_humidity;
    double m_temperature;
    double m_temperatureFeeling;
    double m_windSpeed;
    double m_pressure;

    // astronomy
    QTime m_sunset;
    QTime m_sunrise;

signals:
    void dataReady(const QByteArray &data);
    void locationDetected(const QString &cityCode, const QString &language);
    void querryListReady(const QVariantList &querryList);

    void temperatureReady(const double &temperature);
    void humidityReady(const double &humidity);
    void windSpeedReady(const double &windSpeed);
    void windDirectionReady(const QString &windDirection);
    void currentWeatherReady(const QString &currentWeather);
    void moonAgeReady(const int &age);
    void moonIlluminationReady(const int &age);
    void sunDataReady(const QTime &sunSet, const QTime &sunRise);

private slots:
    void replyFinished(QNetworkReply *reply);
    void processResponse(const QByteArray &data);

public slots:
    void updateData();
    void updateData(const QString &cityCode, const QString &language);
    void searchCity(const QString &searchString);


};

#endif // WEATHERGROUNDPARSER_H
