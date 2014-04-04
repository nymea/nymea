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
    void locationDetected();
    void querryListReady(const QStringList *citys);
    void temperatureReady(const double &temperature);
    void humidityReady(const double &humidity);
    void sunDataReady(const QTime &sunSet, const QTime &sunRise);

private slots:
    void replyFinished(QNetworkReply *reply);
    void processResponse(const QByteArray &data);
    void getDataFromLocation();

public slots:
    void updateData();
    void updateData(QString cityCode, QString language);


};

#endif // WEATHERGROUNDPARSER_H
