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

  \tt{guhbutton.h}
  \code
    #include <QObject>
    #include <QDebug>
    #include <QTimer>

    #include "hardware/gpiomonitor.h"

    class GuhButton : public QObject
    {
        Q_OBJECT
    public:
        explicit GuhButton(QObject *parent = 0, int gpio = 4);
        bool enable();
        void disable();

    private:
        GpioMonitor *m_monitor;
        int m_gpioPin;
        Gpio *m_gpio;

        bool m_buttonPressed;
        QTimer *m_longpressedTimer;
        QTimer *m_debounceTimer;

    signals:
        void buttonPressed();
        void buttonReleased();
        void buttonLongPressed();

    private slots:
        void gpioChanged(const int &gpioPin, const int &value);
    };
  \endcode

  \tt{guhbutton.cpp}
  \code
    #include "guhbutton.h"

    GuhButton::GuhButton(QObject *parent, int gpio) :
        QObject(parent), m_gpioPin(gpio)
    {
        m_buttonPressed = false;
    }

    bool GuhButton::enable()
    {
        qDebug() << "setup button on GPIO " << m_gpioPin;
        m_monitor = new GpioMonitor(this);

        m_gpio = new Gpio(this, m_gpioPin);

        if(!m_monitor->addGpio(m_gpio, false)){
            return false;
        }
        connect(m_monitor, &GpioMonitor::changed, this, &GuhButton::gpioChanged);

        // Timer to generate the long pressed signal
        m_longpressedTimer = new QTimer(this);
        m_longpressedTimer->setInterval(500);
        m_longpressedTimer->setSingleShot(true);
        connect(m_longpressedTimer, &QTimer::timeout, this, &GuhButton::buttonLongPressed);

        // Timer to debounce the button
        m_debounceTimer = new QTimer(this);
        m_debounceTimer->setSingleShot(true);
        m_debounceTimer->setInterval(50);

        m_monitor->enable();
        return true;
    }

    void GuhButton::disable()
    {
        m_monitor->disable();
    }

    void GuhButton::gpioChanged(const int &gpioPin, const int &value)
    {
        if (gpioPin == m_gpioPin){
            if(m_debounceTimer->isActive()){
                return;
            }
            // check button state
            bool buttonState = !QVariant(value).toBool();
            if (m_buttonPressed != buttonState) {
                if (buttonState) {
                    emit buttonPressed();
                    m_longpressedTimer->start();
                    m_debounceTimer->start();
                } else {
                    emit buttonReleased();
                    m_longpressedTimer->stop();
                }
                m_buttonPressed = buttonState;
            }
        }
    }
  \endcode
*/

/*! \fn void GpioMonitor::changed(const int &gpioPin, const int &value);
 *  This signal will be emited, if one of the monitored \l{Gpio}{Gpios} changed his \a value. The \a gpioPin
 *  paramter describes which \l{Gpio} changed his \a value.
 *  \sa Gpio::gpioNumber()
 */

#include "gpiomonitor.h"
#include "loggingcategories.h"

/*! Constructs a \l{GpioMonitor} object with the given \a parent. */
GpioMonitor::GpioMonitor(QObject *parent) :
    QThread(parent)
{
}

/*! Destructs the object of this \l{GpioMonitor}.*/
GpioMonitor::~GpioMonitor()
{
    foreach (Gpio* gpio, m_gpioList) {
        gpio->unexportGpio();
    }
    quit();
    wait();
    deleteLater();
}

/*! Starts the \l{GpioMonitor}. While the \l{GpioMonitor} is running there can not be added new \l{Gpio}{Gpios}.
 *  \sa disable(), */
void GpioMonitor::enable()
{
    m_enabledMutex.lock();
    m_enabled = true;
    m_enabledMutex.unlock();
    start();
}

/*! Stops the \l{GpioMonitor}. No changes on the \l{Gpio} will be recognized.*/
void GpioMonitor::disable()
{
    m_enabledMutex.lock();
    m_enabled = false;
    m_enabledMutex.unlock();
}

/*! Adds the given \l{Gpio} \a gpio to the monitor. This function can only be called if the monitor is not running.
 *  The given \a gpio will be configured as \a activeLow.
 *  Return true if the gpio could be added and set up correctly.
 *  \sa Gpio::setActiveLow(), */
bool GpioMonitor::addGpio(Gpio *gpio, bool activeLow)
{
    if (!gpio->exportGpio() || !gpio->setDirection(INPUT) || !gpio->setEdgeInterrupt(EDGE_BOTH) || !gpio->setActiveLow(activeLow)) {
        return false;
    }
    m_gpioListMutex.lock();
    m_gpioList.append(gpio);
    m_gpioListMutex.unlock();
    return true;
}

/*! Returns the list of \l{Gpio}{Gpios} which currently are monitored.
 *  \sa addGpio(),  */
QList<Gpio *> GpioMonitor::gpioList()
{
    m_gpioListMutex.lock();
    QList<Gpio*> gpioList = m_gpioList;
    m_gpioListMutex.unlock();
    return gpioList;
}

/*! This method represents the reimplementation of the virtual void QThread::run() method. This method will be called from
 *  QThread by calling the method void QThread::start(). Never call this method directly.
 *  \sa enable(), */
void GpioMonitor::run()
{
    struct pollfd *fds;
    char val;
    int ret;
    int retVal;

    fds = (pollfd*) malloc(sizeof(pollfd) * m_gpioList.size());
    m_gpioListMutex.lock();
    for (int i = 0; i < m_gpioList.size(); i++) {
        fds[i].fd = m_gpioList[i]->openGpio();
        fds[i].events = POLLPRI | POLLERR;
    }
    m_gpioListMutex.unlock();

    bool enabled = true;

    m_enabledMutex.lock();
    m_enabled = true;
    m_enabledMutex.unlock();

    while (enabled) {
        m_gpioListMutex.lock();
        ret = poll(fds, m_gpioList.size(), 2000);

        if (ret > 0) {
            for (int i=0; i < m_gpioList.size(); i++) {
                if ((fds[i].revents & POLLPRI) || (fds[i].revents & POLLERR)) {
                    lseek(fds[i].fd, 0, SEEK_SET);
                    retVal = read(fds[i].fd, &val, 1);

                    emit changed(m_gpioList[i]->gpioNumber(), m_gpioList[i]->getValue());

                    if (retVal < 0) {
                        qCWarning(dcHardware) << "GpioMonitor poll failed";
                    }
                }
            }
        } else if (ret < 0) {
            qCWarning(dcHardware) << "poll failed: " << errno;
        }
        m_gpioListMutex.unlock();

        m_enabledMutex.lock();
        enabled = m_enabled;
        m_enabledMutex.unlock();
    }
    free(fds);
}
