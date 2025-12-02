// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class Pwm
    \brief The class represent a pulse wide modulation file system interface.

    \ingroup hardware
    \inmodule libnymea

    This class provides a convenient access to the linux kernel PWM (pulse wide modulation) file system.

    By default, this class will check in the system path \tt{/sys/class/pwm/} for any PWM chips.
    You can find the kernel documentation \l{https://www.kernel.org/doc/Documentation/pwm.txt}{here}.

    This class assumes a file system format where each hardware PWM chip has his own filepath.

    \code
        /sys/class/pwm/pwmchip0
        /sys/class/pwm/pwmchip1
    \endcode

    The number \tt{pwmchip0} represents the chipNumber.

*/

/*! \enum Pwm::Polarity

    \value PolarityNormal
        Normal polarity. This is the default value.
    \value PolarityInversed
        Inversed polarity.
    \value PolarityInvalid
        The current polarity value is invalid.

*/

#include "pwm.h"
#include "loggingcategories.h"

/*! Constructs a new Pwm interface for the given \a chipNumber (\tt{/sys/class/pwm/pwmchip<chipNumber>}) with the given \a parent. */
Pwm::Pwm(int chipNumber, QObject *parent) :
    QObject(parent),
    m_chipNumber(chipNumber),
    m_period(0),
    m_dutyCycle(0)
{
    m_pwmDirectory = QDir("/sys/class/pwm/pwmchip" + QString::number(chipNumber) + "/");
}

/*! Destructor for this Pwm interface. */
Pwm::~Pwm()
{
    unexportPwm();
}

/*! Returns true, if the path \tt{/sys/class/pwm} exists and is not empty. */
bool Pwm::isAvailable()
{
    QDir pwmDirectory("/sys/class/pwm");
    return pwmDirectory.exists() && !pwmDirectory.entryList().isEmpty();
}

/*! Returns true, if this Pwm interface has been exported successfully. */
bool Pwm::exportPwm()
{
    QFile exportFile(m_pwmDirectory.path() + "/export");
    if (!exportFile.open(QIODevice::WriteOnly)) {
        qCWarning(dcHardware()) << "ERROR: could not export PWM" << m_chipNumber;
        return false;
    }

    QTextStream out(&exportFile);
    out << 0;
    exportFile.close();
    return true;
}

/*! Returns true, if this Pwm interface has been enabled successfully. */
bool Pwm::enable()
{
    QFile enableFile(m_pwmDirectory.path() + "/pwm0/enable");
    if (!enableFile.open(QIODevice::WriteOnly)) {
        qCWarning(dcHardware()) << "ERROR: could not enable PWM" << m_chipNumber;
        return false;
    }

    QTextStream out(&enableFile);
    out << 1;
    enableFile.close();
    return true;
}

/*! Returns true, if this Pwm interface has been disabled successfully. */
bool Pwm::disable()
{
    QFile enableFile(m_pwmDirectory.path() + "/pwm0/enable");
    if (!enableFile.open(QIODevice::WriteOnly)) {
        qCWarning(dcHardware()) << "ERROR: could not disable PWM" << m_chipNumber;
        return false;
    }

    QTextStream out(&enableFile);
    out << 0;
    enableFile.close();
    return true;
}

/*! Returns true, if this Pwm interface is enabled. */
bool Pwm::isEnabled()
{
    QFile enableFile(m_pwmDirectory.path() + "/pwm0/enable");
    if (!enableFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcHardware()) << "ERROR: could not read" << enableFile.fileName();
        return false;
    }
    QString value;
    QTextStream in(&enableFile);
    in >> value;
    enableFile.close();

    if (value == "1")
        return true;

    return false;
}

/*! Returns the chip number of this \l{Pwm}.

  The chip number indicates which file path will be used: \tt{/sys/class/pwm/pwmchip<chipNumber>}.
*/
int Pwm::chipNumber()
{
    return m_chipNumber;
}

/*! Returns the period of this PWM. */
long Pwm::period()
{
    // period = active + inactive time
    QFile periodFile(m_pwmDirectory.path() + "/pwm0/period");
    if (!periodFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcHardware()) << "ERROR: could not open" << periodFile.fileName();
        return false;
    }

    QString value;
    QTextStream in(&periodFile);
    in >> value;
    periodFile.close();
    m_period = value.toLong();
    return m_period;
}

/*! Returns true, if the period of this Pwm has been set to \a nanoSeconds successfully. */
bool Pwm::setPeriod(long nanoSeconds)
{
    // the current duty cycle can not be greater than the period
    if (dutyCycle() > nanoSeconds && !setDutyCycle(nanoSeconds))
        return false;

    // period = active + inactive time
    QFile periodFile(m_pwmDirectory.path() + "/pwm0/period");
    if (!periodFile.open(QIODevice::WriteOnly)) {
        qCWarning(dcHardware()) << "ERROR: could not open" << periodFile.fileName();
        return false;
    }
    QTextStream out(&periodFile);
    out << QString::number(nanoSeconds);
    periodFile.close();
    m_period = nanoSeconds;
    return true;
}

/*! Returns the frequency [kHz] of the Pwm. */
double Pwm::frequency()
{
    return (100000000.0 / (period() * 1000));
}

/*! Returns true, if the frequency [kHz] of this Pwm has been set successfully to the given \a kHz. */
bool Pwm::setFrequency(double kHz)
{
    // p = 1 / f
    long nanoSeconds = (long)(100000000 / (kHz * 1000));
    return setPeriod(nanoSeconds);
}

/*! Returns the duty cycle [ns] of the Pwm. The duty cycle is the active time of one period. */
long Pwm::dutyCycle()
{
    QFile dutyCycleFile(m_pwmDirectory.path() + "/pwm0/duty_cycle");
    if (!dutyCycleFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcHardware()) << "ERROR: could not open" << dutyCycleFile.fileName();
        return false;
    }

    QString value;
    QTextStream in(&dutyCycleFile);
    in >> value;
    dutyCycleFile.close();
    m_dutyCycle = value.toLong();
    return m_dutyCycle;
}

/*! Returns true, if the duty cycle [ns] of the Pwm has been set successfully to the given \a nanoSeconds. The duty cycle is the active time of one period. */
bool Pwm::setDutyCycle(long nanoSeconds)
{
    // can not be greater than period or negative
    if (nanoSeconds > m_period || nanoSeconds < 0) {
        qCWarning(dcHardware()) << "ERROR: duty cycle has to be positive and smaller than the period";
        return false;
    }

    QFile dutyCycleFile(m_pwmDirectory.path() + "/pwm0/duty_cycle");
    if (!dutyCycleFile.open(QIODevice::WriteOnly)) {
        qCWarning(dcHardware()) << "ERROR: could not open" << dutyCycleFile.fileName();
        return false;
    }
    QTextStream out(&dutyCycleFile);
    out << QString::number(nanoSeconds);
    dutyCycleFile.close();
    m_dutyCycle = nanoSeconds;
    return true;
}

/*! Returns the Polarity of this Pwm. */
Pwm::Polarity Pwm::polarity()
{
    QFile polarityFile(m_pwmDirectory.path() + "/pwm0/polarity");
    if (!polarityFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcHardware()) << "ERROR: could not open" << polarityFile.fileName();
        return PolarityInvalid;
    }

    QString value;
    QTextStream in(&polarityFile);
    in >> value;
    polarityFile.close();

    if (value == "normal") {
        return PolarityNormal;
    } else if(value == "inversed") {
        return PolarityInversed;
    }

    return PolarityInvalid;
}

/*! Returns true, if the polarity of this Pwm has been set to \a polarity successfully. */
bool Pwm::setPolarity(Pwm::Polarity polarity)
{
    if (polarity == Pwm::PolarityInvalid)
        return false;

    // Note: the polarity can only be changed if the pwm is disabled.
    bool wasEnabled = isEnabled();
    if (wasEnabled && !disable())
        return false;

    QFile polarityFile(m_pwmDirectory.path() + "/pwm0/polarity");
    if (!polarityFile.open(QIODevice::WriteOnly)) {
        qCWarning(dcHardware()) << "ERROR: could not open" << polarityFile.fileName();
        return false;
    }

    QTextStream out(&polarityFile);
    switch (polarity) {
    case PolarityNormal:
        out << "normal";
        break;
    case PolarityInversed:
        out << "inversed";
        break;
    default:
        break;
    }
    polarityFile.close();

    if (wasEnabled)
        enable();

    return true;
}

/*! Returns the current percentage of this Pwm. */
int Pwm::percentage()
{
    return (int)(dutyCycle() * (100.0 / period()) + 0.5);
}

/*! Returns true, if the percentage of this Pwm has been set to \a percentage successfully. */
bool Pwm::setPercentage(int percentage)
{
    long nanoSeconds = period() * (percentage / 100.0);
    return setDutyCycle(nanoSeconds);
}

/*! Returns true, if this Pwm interface has been unexported successfully. */
bool Pwm::unexportPwm()
{
    QFile unexportFile(m_pwmDirectory.path() + "/unexport");
    if (!unexportFile.open(QIODevice::WriteOnly)) {
        qCWarning(dcHardware()) << "ERROR: could not unexport PWM" << m_chipNumber;
        return false;
    }

    QTextStream out(&unexportFile);
    out << 0;
    unexportFile.close();
    return true;
}

QDebug operator<<(QDebug d, Pwm *pwm)
{
    QDebugStateSaver saver(d);
    d << "-----------------------------------";
    d << "\n--> pwm0 on pwmChip" << pwm->chipNumber()  << ":";
    d << "\n------------------";
    d << "\n          enabled:" << pwm->isEnabled();
    d << "\n           period:" << pwm->period() << "[ns]";
    d << "\n       duty cycle:" << pwm->dutyCycle() << "[ns]";
    d << "\n        frequency:" << pwm->frequency() << "[kHz]";
    d << "\n       percentage:" << pwm->percentage() << "[%]";

    switch (pwm->polarity()) {
    case Pwm::PolarityNormal:
        d << "\n         polarity:" << "normal";
        break;
    case Pwm::PolarityInversed:
        d << "\n         polarity:" << "inversed";
        break;
    default:
        d << "\n         polarity:" << "invalid";
        break;
    }
    d << "\n-----------------------------------\n";
    return d;
}
