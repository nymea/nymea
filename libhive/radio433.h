#ifndef RADIO433_H
#define RADIO433_h

#include <QObject>
#include <QThread>
#include <gpio.h>

#define RC_MAX_CHANGES 49

class Radio433: public QObject
{
    Q_OBJECT

public:
    Radio433(QObject *parent = 0);
    ~Radio433();

public:
    void sendData(QList<int> rawData);

private:
    Gpio *m_receiver;
    Gpio *m_transmitter;

    unsigned int m_timings[RC_MAX_CHANGES];
    unsigned int m_duration;
    unsigned int m_changeCount;
    unsigned long m_lastTime;
    unsigned int m_repeatCount;
    unsigned int m_epochMicro;

    int micros();
    void delayMicros(int microSeconds);

private slots:
    void handleInterrupt();


signals:
    /*! This signal is emitted whenever a valid signal of 48 bits was recognized over the
     * 433 MHz receiver
     */
    void dataReceived(QList<int> rawData);
};

#endif
