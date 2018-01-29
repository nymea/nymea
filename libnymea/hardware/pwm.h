/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 -2016 Simon Stürz <simon.stuerz@guh.io>             *
 *                                                                         *
 *  This file is part of guh.                                              *
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

#ifndef PWM_H
#define PWM_H

#include <QObject>
#include <QDebug>
#include <QFile>
#include <QDir>

#include "libguh.h"

/* i.MX6 PWMs
 *
 * /sys/class/pwm/pwmchip0 -> pwm1 (npwm = 1)
 * /sys/class/pwm/pwmchip1 -> pwm2 (npwm = 1)
 * /sys/class/pwm/pwmchip2 -> pwm3 (npwm = 1)
 * /sys/class/pwm/pwmchip3 -> pwm4 (npwm = 1)
 *
 * each chip has 1 pwm ( -> pwm0)
 *
 * example:
 * echo 0 > /sys/class/pwm/pwmchip1/export
 *
 * ls -l /sys/class/pwm/pwmchip1/pwm0/
 * duty_cycle   => nano seconds active time
 * enable       => 0 = disable, 1 = enable
 * period       => nano seconds active + inactive
 * polarity     => "normal" or "inversed"
 *
 *      |<--------- period --------->|
 *
 *      |<--duty cycle -->|
 * 1_    _________________             _________________             _____
 *      |                 |           |                 |           |
 * 0_  _|                 |___________|                 |___________|       .......
 *
 */

class LIBGUH_EXPORT Pwm : public QObject
{
    Q_OBJECT
public:
    enum Polarity {
        PolarityNormal,
        PolarityInversed,
        PolarityInvalid
    };

    explicit Pwm(int chipNumber, QObject *parent = nullptr);
    ~Pwm();

    static bool isAvailable();

    bool exportPwm();

    bool enable();
    bool disable();

    bool isEnabled();

    int chipNumber();

    long period();
    bool setPeriod(long nanoSeconds);

    double frequency();
    bool setFrequency(double kHz);

    long dutyCycle();
    bool setDutyCycle(long nanoSeconds);

    Pwm::Polarity polarity();
    bool setPolarity(Pwm::Polarity polarity);

    int percentage();
    bool setPercentage(int percentage);

private:
    int m_chipNumber;
    long m_period;
    long m_dutyCycle;
    QDir m_pwmDirectory;

    bool unexportPwm();
};

QDebug operator<< (QDebug d, Pwm *pwm);

#endif // PWM_H
