/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

    void enable();
    void disable();
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
