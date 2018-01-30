/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 -2016 Simon Stürz <simon.stuerz@guh.io>             *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef GPIOMONITOR_H
#define GPIOMONITOR_H

#include <QObject>
#include <QDebug>
#include <QSocketNotifier>
#include <QFile>

#include "libnymea.h"
#include "gpio.h"

class LIBNYMEA_EXPORT GpioMonitor : public QObject
{
    Q_OBJECT

public:
    explicit GpioMonitor(int gpio, QObject *parent = nullptr);

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
