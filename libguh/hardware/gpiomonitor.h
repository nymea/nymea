/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 -2016 Simon Stürz <simon.stuerz@guh.io>             *
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

#include <QObject>
#include <QDebug>
#include <QSocketNotifier>
#include <QFile>

#include "libguh.h"
#include "gpio.h"

class LIBGUH_EXPORT GpioMonitor : public QObject
{
    Q_OBJECT

public:
    explicit GpioMonitor(int gpio, QObject *parent = 0);

    bool enable(bool activeLow = false, Gpio::Edge edgeInterrupt = Gpio::EdgeBoth);
    void disable();

    bool isRunning() const;
    bool value() const;

    Gpio* gpio();

private:
    int m_gpioNumber;
    Gpio *m_gpio;
    QSocketNotifier *m_notifier;
    QFile m_valueFile;
    bool m_currentValue;

signals:
    void valueChanged(const bool &value);

private slots:
    void readyReady(const int &ready);

};

#endif // GPIOMONITOR_H
