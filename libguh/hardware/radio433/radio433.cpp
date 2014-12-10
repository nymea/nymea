/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
  \brief The Radio433 class helps to interact with the 433 MHz Receiver and Transmitter.

  \inmodule libguh

*/

#include "radio433.h"

Radio433::Radio433(QObject *parent) :
    QObject(parent)
{
    m_receiver = new Radio433Receiver(this,27);
    m_transmitter = new Radio433Trasmitter(this,22);
    m_brennenstuhlTransmitter = new Radio433BrennenstuhlGateway(this);

    connect(m_receiver, &Radio433Receiver::readingChanged, this, &Radio433::readingChanged);
    connect(m_receiver, &Radio433Receiver::dataReceived, this, &Radio433::dataReceived);
    connect(m_brennenstuhlTransmitter, &Radio433BrennenstuhlGateway::availableChanged, this, &Radio433::brennenstuhlAvailableChanged);
}

Radio433::~Radio433()
{
    m_receiver->quit();
    m_transmitter->quit();
}

bool Radio433::enable()
{
    m_brennenstuhlTransmitter->enable();

    bool receiverAvailable = m_receiver->startReceiver();
    if (!receiverAvailable) {
        //qWarning() << "ERROR: radio 433 MHz receiver not available on GPIO's";
    }

    bool transmitterAvailable = m_transmitter->startTransmitter();
    if (!transmitterAvailable) {
        //qWarning() << "ERROR: radio 433 MHz transmitter not available on GPIO's";
    }

    if (!receiverAvailable && !transmitterAvailable) {
        qWarning() << "--> Radio 433 MHz GPIO's not available.";
        return false;
    }
    qDebug() << "--> Radio 433 MHz GPIO's enabled.";
    return true;
}

bool Radio433::disabel()
{
    m_brennenstuhlTransmitter->disable();
    if (m_receiver->stopReceiver()) {
        return true;
    }
    return false;
}

void Radio433::readingChanged(bool reading)
{
    if (reading) {
        m_transmitter->allowSending(false);
    } else {
        m_transmitter->allowSending(true);
    }
}

void Radio433::brennenstuhlAvailableChanged(const bool &available)
{
    if (available) {
        qDebug() << "--> Radio 433 MHz Brennenstuhl LAN Gateway available.";
    } else {
        qDebug() << "--> Radio 433 MHz Brennenstuhl LAN Gateway NOT available.";
    }
}

bool Radio433::sendData(int delay, QList<int> rawData)
{
    bool sendGpio = false;
    bool sendBrennenstuhl = false;

    if (m_brennenstuhlTransmitter->available()) {
        sendBrennenstuhl = m_brennenstuhlTransmitter->sendData(delay, rawData);
    }

    if (m_transmitter->available()) {
        m_transmitter->sendData(delay, rawData);
        sendGpio = true;
    }

    return (sendGpio || sendBrennenstuhl);
}
