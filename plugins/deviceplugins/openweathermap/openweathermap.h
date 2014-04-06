#ifndef OPENWEATHERMAP_H
#define OPENWEATHERMAP_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QUrl>

class OpenWeatherMap : public QObject
{
    Q_OBJECT
public:
    explicit OpenWeatherMap(QObject *parent = 0);

private:
    QNetworkAccessManager *m_manager;

    QString m_cityName;
    QString m_cityId;

    QNetworkReply *m_locationReplay;
    QNetworkReply *m_searchReplay;
    QNetworkReply *m_weatherReplay;

    QString m_country;
    QString m_weatherDescription;
    uint m_lastUpdate;
    uint m_sunrise;
    uint m_sunset;
    double m_temperatur;
    double m_temperaturMin;
    double m_temperaturMax;
    double m_pressure;
    double m_windSpeed;
    int m_windDirection;
    int m_humidity;
    int m_cloudiness;

    void updateLocationData();
    void updateSearchData();
    void updateWeatherData();

    void processLocationResponse(QByteArray data);
    void processSearchResponse(QByteArray data);
    void processWeatherResponse(QByteArray data);

signals:


public slots:
    void update();

private slots:
    void replyFinished(QNetworkReply *reply);

};

#endif // OPENWEATHERMAP_H
