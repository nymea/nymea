#ifndef RADIORECIVER_H
#define RADIORECIVER_H

#define RC_MAX_CHANGES 49

#include <QObject>

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
    void detectProtocol(QList<int> rawData);
    float parseTemperature(QByteArray codeBin);

    bool m_enable;
    int m_pin;
    Frequency m_frequency;
    unsigned int m_timings[RC_MAX_CHANGES];
    unsigned int m_duration;
    unsigned int m_changeCount;
    unsigned long m_lastTime;
    unsigned int m_repeatCount;

signals:
    
public slots:
    void enableReceiver();
    void disableReceiver();
};

#endif // RADIORECIVER_H
