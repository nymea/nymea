#ifndef RFTHERMOMETER_H
#define RFTHERMOMETER_H

#include <QObject>
#include <radio/plugins/radioplugin.h>

#define RFThermometerDelay 250

class RFThermometer : public RadioPlugin
{
public:
    explicit RFThermometer(QObject *parent = 0);

    QByteArray getBinCode();
    bool isValid(QList<int> rawData);
    float getTemperature();

private:
    float m_lastTemperature;
    int m_delay;
    QByteArray m_binCode;

signals:
    void temperatureSignalReceived(const QByteArray &id, const float &temperature, const bool &batteryStatus);

public slots:


};

#endif // RFTHERMOMETER_H
