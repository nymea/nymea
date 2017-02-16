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

/*!
  \class Radio433
  \brief The Radio433 class helps to interact with the 433 MHz receiver and transmitter.

  \ingroup hardware
  \inmodule libguh

  This class handles all supported radio 433 MHz transmitter. Receiving data on the 433.92 MHz frequency
  is only supported, if there are \l{Gpio}{GPIO's} available and a suitable receiver is connected to GPIO 27. Examples for receiver
  can be found \l{https://www.futurlec.com/Radio-433MHZ.shtml}{here}. The antenna has a very large impact on the quality
  of the signal and how well it is recognized. In many forums and blogs it is described that a 17, 3 mm piece of wire is enough.
  Experiments have shown, it's not. A 50 Ohm coaxial cabel (thickness = 1mm), mounted on the antenna pin of the receiver
  with a minimum distance of 5 cm away from the circuit and unisolated 17.3 mm at the end has shown the best results.

  In order to send data to a 433 MHz device, there currently are two possibilitis. If there are \l{Gpio}{GPIO's}
  available, the data will be sent over the transmitter connected to GPIO 22. Also in this case the antenna is a verry
  important part.

  The second possibility to sent data to a 433 MHz device is the \l{http://www.brennenstuhl.de/de-DE/steckdosenleisten-schaltgeraete-und-adapter/brematic-hausautomation/brematic-home-automation-gateway-gwy-433-1.html}
  {Brennenstuhl 433 MHz LAN Gateway}. If there is a Gateway in the local network, this class will automaticaly detect
  it and will be used. If both transmitter are available (Gateway + GPIO), each signal will be transmitted over both sender.

  \note: Radio 433 on GPIO's is by default disabled. If you want to enable it, you need to compile the source with the qmake config \tt{CONFIG+=radio433gpio}

*/


#include "radio433.h"
#include "loggingcategories.h"
#include "guhsettings.h"
#include "hardware/gpio.h"

#include <QFileInfo>

/*! Construct the hardware resource Radio433 with the given \a parent. Each possible 433 MHz hardware will be initialized here. */
Radio433::Radio433(QObject *parent) :
    QObject(parent)
{
    #ifdef GPIO433
    GuhSettings settings(GuhSettings::SettingsRoleGlobal);
    qCDebug(dcHardware) << "Loading GPIO settings from:" << settings.fileName();
    settings.beginGroup("GPIO");
    int transmitterGpioNumber = settings.value("rf433tx",22).toInt();
    settings.endGroup();

    m_transmitter = new Radio433Trasmitter(this, transmitterGpioNumber);
    #endif

    m_brennenstuhlTransmitter = new Radio433BrennenstuhlGateway(this);
    connect(m_brennenstuhlTransmitter, &Radio433BrennenstuhlGateway::availableChanged, this, &Radio433::brennenstuhlAvailableChanged);
}

/*! Destroys the hardware resource Radio433 object. */
Radio433::~Radio433()
{
    #ifdef GPIO433
    m_transmitter->quit();
    #endif
}

/*! Enables GPIO transmitter and receiver and the Brennenstuhl Lan Gateway.
 *  Returns true, if the GPIO's are available and set up correctly. The status of the gateway will be emited asynchronous. */
bool Radio433::enable()
{
    m_brennenstuhlTransmitter->enable();

    #ifdef GPIO433
    // check if GPIOs are available
    if (Gpio::isAvailable()) {

        bool transmitterAvailable = m_transmitter->startTransmitter();
        if (!transmitterAvailable) {
            //qCWarning(dcHardware) << "ERROR: radio 433 MHz transmitter not available on GPIO's";
        }

        if (!transmitterAvailable) {
            qCWarning(dcHardware) << "--> Radio 433 MHz GPIO's not available.";
            return false;
        }
    }
    qCDebug(dcDeviceManager()) << "--> Radio 433 MHz GPIO's enabled.";
    #endif

    return true;
}

/*! Returns true, if the Radio433 hardware resources disabled correctly. */
bool Radio433::disabel()
{
    m_brennenstuhlTransmitter->disable();
    return true;
}

void Radio433::brennenstuhlAvailableChanged(const bool &available)
{
    if (available) {
        qCDebug(dcHardware()) << "--> Radio 433 MHz Brennenstuhl LAN Gateway available.";
    } else {
        qCWarning(dcHardware()) << "--> Radio 433 MHz Brennenstuhl LAN Gateway NOT available.";
    }
}

/*! Returns true, if the \a rawData with a certain \a delay (pulse length) could be sent \a repetitions times. */
bool Radio433::sendData(int delay, QList<int> rawData, int repetitions)
{
    bool sendGpio = false;
    bool sendBrennenstuhl = false;

    if (m_brennenstuhlTransmitter->available()) {
        sendBrennenstuhl = m_brennenstuhlTransmitter->sendData(delay, rawData, repetitions);
    }

    #ifdef GPIO433
    if (m_transmitter->available()) {
        m_transmitter->sendData(delay, rawData, repetitions);
        sendGpio = true;
    }
    #endif

    return (sendGpio || sendBrennenstuhl);
}
