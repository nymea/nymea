/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 -2016 Simon Stürz <simon.stuerz@guh.guru>           *
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

/*!
  \class Gpio
  \brief The Gpio class allows to interact with the GPIOs.

  \ingroup hardware
  \inmodule libguh

  A "General Purpose Input/Output" (GPIO) is a flexible software-controlled
  digital signal. They are provided from many kinds of chip, and are familiar
  to Linux developers working with embedded and custom hardware. Each GPIO
  represents a bit connected to a particular pin, or "ball" on Ball Grid Array
  (BGA) packages. Board schematics show which external hardware connects to
  which GPIOs. Drivers can be written generically, so that board setup code
  passes such pin configuration data to drivers
  (\l{https://www.kernel.org/doc/Documentation/gpio/gpio.txt}{source}).

  General Purpose Input/Output (a.k.a. GPIO) is a generic pin on a chip whose
  behavior (including whether it is an input or output pin) can be controlled
  through this class. An object of of the Gpio class represents a pin.


  \code
    Gpio *gpioOut = new Gpio(23, this);

    // Export Gpio
    if (!gpioOut->exportGpio()) {
        qWarning() << "Could not export Gpio" << gpioOut->gpioNumber();
        gpioOut->deleteLater();
        return;
    }

    // Configure Gpio direction
    if (!gpioOut->setDirection(PiGpio::DirectionOutput)) {
        qWarning() << "Could not set direction of Gpio" << gpioOut->gpioNumber();
        gpioOut->deleteLater();
        return;
    }

    gpioOut->setValue(Gpio::ValueHigh)

  \endcode

  \code
    Gpio *gpioIn = new Gpio(24, this);

    // Export Gpio
    if (!gpioIn->exportGpio()) {
        qWarning() << "Could not export Gpio" << gpioIn->gpioNumber();
        gpioIn->deleteLater();
        return;
    }

    // Configure Gpio direction
    if (!gpioIn->setDirection(PiGpio::DirectionInput)) {
        qWarning() << "Could not set direction of Gpio" << gpioIn->gpioNumber();
        gpioIn->deleteLater();
        return;
    }

    qDebug() << "Current value" << (bool)gpioIn->value();

  \endcode


  \sa GpioMonitor
*/

/*! \enum Gpio::Direction

    This enum type specifies the dirction a \l{Gpio}.

    \value DirectionInput
        The \l{Gpio} is configured as \b input.
    \value DirectionOutput
        The \l{Gpio} is configured as \b output.
    \value DirectionInvalid
        The direction is not valid.
*/

/*! \enum Gpio::Value

    This enum type specifies the value a \l{Gpio}.

    \value ValueLow
        The \l{Gpio} is low.
    \value ValueHigh
        The \l{Gpio} is high.
    \value ValueInvalid
        The value is not valid.
*/

/*! \enum Gpio::Edge

    This enum type specifies the edge interrupt type of a \l{Gpio}.

    \value EdgeFalling
        The \l{Gpio} reacts on falling edge interrupt.
    \value EdgeRising
        The \l{Gpio} reacts on rising edge interrupt.
    \value EdgeBoth
        The \l{Gpio} reacts on both, rising and falling edge interrupt.
    \value EdgeNone
        The \l{Gpio} does not react on interrupts.

*/

#include "gpio.h"
#include "loggingcategories.h"

#include <QDebug>

/*! Constructs a \l{Gpio} object to represent a GPIO with the given \a gpio number and \a parent. */
Gpio::Gpio(const int &gpio, QObject *parent) :
    QObject(parent),
    m_gpio(gpio),
    m_direction(Gpio::DirectionInvalid),
    m_gpioDirectory(QDir(QString("/sys/class/gpio/gpio%1/").arg(QString::number(gpio))))
{
    m_direction = direction();
}

/*! Destroys and unexports the \l{Gpio}. */
Gpio::~Gpio()
{
    unexportGpio();
}

/*! Returns the directory \tt {/sys/class/gpio/gpio<number>} of this Gpio. */
QString Gpio::gpioDirectory() const
{
    return m_gpioDirectory.canonicalPath();
}

/*! Returns the number of this \l{Gpio}. \note The Gpio number is mostly not equivalent with the pin number. */
int Gpio::gpioNumber() const
{
    return m_gpio;
}


/*! Returns true if the directories \tt {/sys/class/gpio} and \tt {/sys/class/gpio/export} do exist. */
bool Gpio::isAvailable()
{
    return QFile("/sys/class/gpio/export").exists();
}

/*! Returns true if this \l{Gpio} could be exported in the system file \tt {/sys/class/gpio/export}. If this Gpio is already exported, this function will return true. */
bool Gpio::exportGpio()
{
    // Check if already exported
    if (m_gpioDirectory.exists())
        return true;

    QFile exportFile("/sys/class/gpio/export");
    if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(dcHardware()) << "Gpio: Could not open GPIO export file:" << exportFile.errorString();
        return false;
    }

    QTextStream out(&exportFile);
    out << m_gpio;
    exportFile.close();
    return true;
}

/*! Returns true if this \l{Gpio} could be unexported in the system file \tt {/sys/class/gpio/unexport}. */
bool Gpio::unexportGpio()
{
    QFile unexportFile("/sys/class/gpio/unexport");
    if (!unexportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(dcHardware()) << "Gpio: Could not open GPIO unexport file:" << unexportFile.errorString();
        return false;
    }

    QTextStream out(&unexportFile);
    out << m_gpio;
    unexportFile.close();
    return true;
}

/*! Returns true if the \a direction of this GPIO could be set. \sa Gpio::Direction, */
bool Gpio::setDirection(Gpio::Direction direction)
{
    if (direction == Gpio::DirectionInvalid) {
        qCWarning(dcHardware()) << "Gpio: Setting an invalid direction is forbidden.";
        return false;
    }

    QFile directionFile(m_gpioDirectory.path() + "/direction");
    if (!directionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(dcHardware()) << "Gpio: Could not open GPIO" << m_gpio << "direction file:" << directionFile.errorString();
        return false;
    }

    m_direction = direction;

    QTextStream out(&directionFile);
    switch (m_direction) {
    case DirectionInput:
        out << "in";
        break;
    case DirectionOutput:
        out << "out";
        break;
    default:
        break;
    }

    directionFile.close();
    return true;
}

/*! Returns the direction of this \l{Gpio}. */
Gpio::Direction Gpio::direction()
{
    QFile directionFile(m_gpioDirectory.path() + "/direction");
    if (!directionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(dcHardware()) << "Gpio: Could not open GPIO" << m_gpio << "direction file:" << directionFile.errorString();
        return Gpio::DirectionInvalid;
    }

    QString direction;
    QTextStream in(&directionFile);
    in >> direction;
    directionFile.close();

    if (direction == "in") {
        m_direction = DirectionInput;
        return Gpio::DirectionInput;
    } else if (direction == "out") {
        m_direction = DirectionOutput;
        return Gpio::DirectionOutput;
    }

    return Gpio::DirectionInvalid;
}

/*! Returns true if the digital \a value of this \l{Gpio} could be set correctly. */
bool Gpio::setValue(Gpio::Value value)
{
    // Check given value
    if (value == Gpio::ValueInvalid) {
        qCWarning(dcHardware()) << "Gpio: Setting an invalid value is forbidden.";
        return false;
    }

    // Check current direction
    if (m_direction == Gpio::DirectionInput) {
        qCWarning(dcHardware()) << "Gpio: Setting the value of an input GPIO is forbidden.";
        return false;
    }

    if (m_direction == Gpio::DirectionInvalid) {
        qCWarning(dcHardware()) << "Gpio: The direction of GPIO" << m_gpio << "is invalid.";
        return false;
    }

    QFile valueFile(m_gpioDirectory.path() + "/value");
    if (!valueFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(dcHardware()) << "Gpio: Could not open GPIO" << m_gpio << "value file:" << valueFile.errorString();
        return false;
    }

    QTextStream out(&valueFile);
    switch (value) {
    case ValueLow:
        out << "0";
        break;
    case ValueHigh:
        out << "1";
        break;
    default:
        valueFile.close();
        return false;
    }

    valueFile.close();
    return true;
}

/*! Returns the current digital value of this \l{Gpio}. */
Gpio::Value Gpio::value()
{
    QFile valueFile(m_gpioDirectory.path() + "/value");
    if (!valueFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(dcHardware()) << "Gpio: Could not open GPIO" << m_gpio << "value file:" << valueFile.errorString();
        return Gpio::ValueInvalid;
    }

    QString value;
    QTextStream in(&valueFile);
    in >> value;
    valueFile.close();

    if (value == "0") {
        return Gpio::ValueLow;
    } else if (value == "1") {
        return Gpio::ValueHigh;
    }

    return Gpio::ValueInvalid;
}

/*! This method allows to invert the logic of this \l{Gpio}. Returns true, if the GPIO could be set \a activeLow. */
bool Gpio::setActiveLow(bool activeLow)
{
    QFile activeLowFile(m_gpioDirectory.path() + "/active_low");
    if (!activeLowFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(dcHardware()) << "Gpio: Could not open GPIO" << m_gpio << "active_low file:" << activeLowFile.errorString();
        return false;
    }

    QTextStream out(&activeLowFile);
    if (activeLow) {
        out << "0";
    } else {
        out << "1";
    }

    activeLowFile.close();
    return true;
}

/*! Returns true if the logic of this \l{Gpio} is inverted (1 = low, 0 = high). */
bool Gpio::activeLow()
{
    QFile activeLowFile(m_gpioDirectory.path() + "/active_low");
    if (!activeLowFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(dcHardware()) << "Gpio: Could not open GPIO" << m_gpio << "active_low file:" << activeLowFile.errorString();
        return false;
    }

    QString value;
    QTextStream in(&activeLowFile);
    in >> value;
    activeLowFile.close();

    if (value == "0")
        return true;

    return false;
}

/*! Returns true if the \a edge of this GPIO could be set correctly. The \a edge parameter specifies,
 *  when an interrupt occurs. */
bool Gpio::setEdgeInterrupt(Gpio::Edge edge)
{
    if (m_direction == Gpio::DirectionOutput) {
        qCWarning(dcHardware()) << "Gpio: Could not set edge interrupt, GPIO is configured as an output.";
        return false;
    }

    QFile edgeFile(m_gpioDirectory.path() + "/edge");
    if (!edgeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(dcHardware()) << "Gpio: Could not open GPIO" << m_gpio << "edge file:" << edgeFile.errorString();
        return false;
    }

    QTextStream out(&edgeFile);
    switch (edge) {
    case EdgeFalling:
        out << "falling";
        break;
    case EdgeRising:
        out << "rising";
        break;
    case EdgeBoth:
        out << "both";
        break;
    case EdgeNone:
        out << "none";
        break;
    default:
        return false;
    }

    edgeFile.close();
    return true;
}

/*! Returns the edge interrupt of this \l{Gpio}. */
Gpio::Edge Gpio::edgeInterrupt()
{
    QFile edgeFile(m_gpioDirectory.path() + "/edge");
    if (!edgeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(dcHardware()) << "Gpio: Could not open GPIO" << m_gpio << "edge file:" << edgeFile.errorString();
        return Gpio::EdgeNone;
    }

    QString edge;
    QTextStream in(&edgeFile);
    in >> edge;
    edgeFile.close();

    if (edge.contains("falling")) {
        return Gpio::EdgeFalling;
    } else if (edge.contains("rising")) {
        return Gpio::EdgeRising;
    } else if (edge.contains("both")) {
        return Gpio::EdgeBoth;
    } else if (edge.contains("none")) {
        return Gpio::EdgeNone;
    }

    return Gpio::EdgeNone;
}



QDebug operator<<(QDebug debug, Gpio *gpio)
{
    debug.nospace() << "Gpio(" << gpio->gpioNumber() << ", ";
    if (gpio->direction() == Gpio::DirectionInput) {
        debug.nospace() << "Input, ";
    } else if (gpio->direction() == Gpio::DirectionOutput) {
        debug.nospace() << "Output, ";
    } else {
        debug.nospace() << "Invalid, ";
    }

    switch (gpio->edgeInterrupt()) {
    case Gpio::EdgeFalling:
        debug.nospace() << "Ir: Falling, ";
        break;
    case Gpio::EdgeRising:
        debug.nospace() << "Ir: Rising, ";
        break;
    case Gpio::EdgeBoth:
        debug.nospace() << "Ir: Both, ";
        break;
    case Gpio::EdgeNone:
        debug.nospace() << "Ir: None, ";
        break;
    default:
        break;
    }

    if (gpio->activeLow()) {
        debug.nospace() << "Active Low: 1, ";
    } else {
        debug.nospace() << "Active Low: 0, ";
    }

    if (gpio->value() == Gpio::ValueHigh) {
        debug.nospace() << "Value: 1)";
    } else if (gpio->value() == Gpio::ValueLow) {
        debug.nospace() << "Value: 0)";
    } else {
        debug.nospace() << "Value: Invalid)";
    }

    return debug;
}
