#ifndef RADIORECIVER_H
#define RADIORECIVER_H

#define RC_MAX_CHANGES 49

#include <QObject>
#include "radioplugin/radioswitch.h"
#include "radioplugin/radiothermometer.h"

class ISRHandler;

class RadioReciver : public QObject
{
    Q_OBJECT
public:
    explicit RadioReciver(QObject *parent = 0);

    friend class ISRHandler;

    enum Frequency{
        RF433MHz = 0x0,
        RF868MHz = 0x1
    };

    void setFrequency(Frequency frequency);
    Frequency getFrequency() const;
    void setPin(int pin);
    int getPin() const;

private:
    void handleInterrupt();
    bool detectProtocol(QList<int> rawData);

    bool m_enable;
    int m_pin;
    Frequency m_frequency;
    unsigned int m_timings[RC_MAX_CHANGES];
    unsigned int m_duration;
    unsigned int m_changeCount;
    unsigned long m_lastTime;
    unsigned int m_repeatCount;

    RadioThermometer *m_thermometer;
    RadioSwitch *m_switch;

signals:
    void temperatureSignalReceived(const QByteArray &id, const float &temperature, const bool &batteryStatus);
    void switchSignalReceived(const QByteArray &channel, const char &button, const bool &buttonStatus);

public slots:
    void enableReceiver();
    void disableReceiver();
};

#endif // RADIORECIVER_H
