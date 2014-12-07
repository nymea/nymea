#ifndef GPIOMONITOR_H
#define GPIOMONITOR_H

#include <QThread>
#include <QDebug>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <errno.h>

#include "hardware/gpio.h"

class GpioMonitor : public QThread
{
    Q_OBJECT
public:
    explicit GpioMonitor(QObject *parent = 0);
    ~GpioMonitor();

    void stop();
    bool addGpio(Gpio *gpio, bool activeLow);
    QList<Gpio*> gpioList();


private:
    QMutex m_enabledMutex;
    bool m_enabled;

    QMutex m_gpioListMutex;
    QList<Gpio*> m_gpioList;

protected:
    void run();

signals:
    void changed(const int &gpioPin, const int &value);

};

#endif // GPIOMONITOR_H
