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

/*!
    \class GpioMonitor
    \brief The GpioMonitor class allows to monitor GPIOs.

    \ingroup hardware
    \inmodule libguh

    An instance of this class creates a thread, which monitors each of the added GPIOs. The object
    emits a signal if one of the added GPIOs changes its value. The GpioMonitor configures a GPIO as an
    input, with the edge interrupt EDGE_BOTH (\l{Gpio::setEdgeInterrupt()}{setEdgeInterrupt}).

    \chapter Example
    Following example shows how to use the GpioMonitor class for a button on the Raspberry Pi. There are two possibilitys
    to connect a button. Following picture shows the schematics:

    \image Raspberry_Pi_Button_Example.png "Raspberry Pi button example"

    Button A represents a clean solutin with a 10 k\unicode{0x2126} resistor (set up as activeLow = false).
    Button B represents a "dirty" solution, were the 3.3V will be directly connected to the GPIO if the button is pressed (set activeLow = true).

    Here is the code example for a button class:
    \code
          Button::Button(QObject *parent) :
              QObject(parent)
          {
              m_button  = new GpioMonitor(110, this);
              connect(m_button, &GpioMonitor::valueChanged, this, &Button::stateChanged);
          }

          bool Button::init()
          {
              return m_button->enable();
          }

          void Button::stateChanged(const bool &value)
          {
              if (m_pressed != value) {
                  m_pressed = value;
                  if (value) {
                      emit buttonPressed();
                  } else {
                      emit buttonReleased();
                  }
              }
          }
    \endcode
*/

/*! \fn void GpioMonitor::valueChanged(const bool &value);
 *  This signal will be emited, if the monitored \l{Gpio}{Gpios} changed his \a value. */

#include "gpiomonitor.h"
#include "loggingcategories.h"

/*! Constructs a \l{GpioMonitor} object with the given \a gpio number and \a parent. */
GpioMonitor::GpioMonitor(int gpio, QObject *parent) :
    QObject(parent),
    m_gpioNumber(gpio)
{
    m_valueFile.setFileName("/sys/class/gpio/gpio" + QString::number(m_gpioNumber) + "/value");
}

/*! Returns true if this \l{GpioMonitor} could be enabled successfully. With the \a activeLow parameter the values can be inverted.
    With the \a edgeInterrupt parameter the interrupt type can be specified. */
bool GpioMonitor::enable(bool activeLow, Gpio::Edge edgeInterrupt)
{
    if (!Gpio::isAvailable())
        return false;

    m_gpio = new Gpio(m_gpioNumber, this);
    if (!m_gpio->exportGpio() ||
            !m_gpio->setDirection(Gpio::DirectionInput) ||
            !m_gpio->setActiveLow(activeLow) ||
            !m_gpio->setEdgeInterrupt(edgeInterrupt)) {
        qCWarning(dcHardware()) << "GpioMonitor: Error while initializing GPIO" << m_gpio->gpioNumber();
        return false;
    }

    if (!m_valueFile.open(QFile::ReadOnly)) {
        qWarning(dcHardware()) << "GpioMonitor: Could not open value file for gpio monitor" << m_gpio->gpioNumber();
        return false;
    }

    m_notifier = new QSocketNotifier(m_valueFile.handle(), QSocketNotifier::Exception);
    connect(m_notifier, &QSocketNotifier::activated, this, &GpioMonitor::readyReady);

    m_notifier->setEnabled(true);
    return true;
}

/*! Disables this \l{GpioMonitor}. */
void GpioMonitor::disable()
{
    delete m_notifier;
    delete m_gpio;

    m_notifier = 0;
    m_gpio = 0;

    m_valueFile.close();
}

/*! Returns true if this \l{GpioMonitor} is running. */
bool GpioMonitor::isRunning() const
{
    if (!m_notifier)
        return false;

    return m_notifier->isEnabled();
}

/*! Returns the current value of this \l{GpioMonitor}. */
bool GpioMonitor::value() const
{
    return m_currentValue;
}

/*! Returns the \l{Gpio} of this \l{GpioMonitor}. */
Gpio *GpioMonitor::gpio()
{
    return m_gpio;
}

void GpioMonitor::readyReady(const int &ready)
{
    Q_UNUSED(ready)

    m_valueFile.seek(0);
    QByteArray data = m_valueFile.readAll();

    bool value = false;
    if (data[0] == '1') {
        value = true;
    } else if (data[0] == '0') {
        value = false;
    } else {
        return;
    }

    m_currentValue = value;
    emit valueChanged(value);
}
