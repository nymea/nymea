#ifndef RADIOTHERMOMETER_H
#define RADIOTHERMOMETER_H

#include <QObject>
#include <radioplugin/radioplugin.h>

#define RadioThermometerDelay 250

class RadioThermometer : public RadioPlugin
{
    Q_OBJECT
public:
    explicit RadioThermometer(QObject *parent = 0);

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

#endif // RADIOTHERMOMETER_H
